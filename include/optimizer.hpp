#ifndef VKR_OPTIMIZER
#define VKR_OPTIMIZER

#include "i_bus.hpp"

#include <stdlib.h>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <limits>
#include <fstream>
#include <iomanip>

template <typename KeyType>
class IExternalStorageOptimizer {
public:
    virtual void optimize(int storage1, int storage2) = 0;
    virtual ~IExternalStorageOptimizer() = default;
};

template <typename KeyType>
class DummyExternalStorageOptimizer : public IExternalStorageOptimizer<KeyType> {
private:
    IBus<KeyType>* bus;
    std::string log_file_path; 
public:
    DummyExternalStorageOptimizer(IBus<KeyType>* _bus, std::string _log_file_path = "kl_experiment.log")
        : bus(_bus), log_file_path(_log_file_path) {}
    void optimize(int storage1, int storage2) override {
        double cut = bus->get_inter_storage_cut_percent();
        std::ofstream log_file(log_file_path, std::ios::app);
        if (log_file.is_open()) {
            log_file << "KL " << storage1 << "-" << storage2 
                    << " iterations=0 initial_cut=" << cut 
                    << " final_cut=" << cut << " delta=0\n";
        }
        std::cout << "Dummy KL [" << storage1 << "<->" << storage2 
                << "] cut = " << cut << "%\n";
    };
};

template <typename KeyType>
class KLExternalStorageOptimizer : public IExternalStorageOptimizer<KeyType> {
private:
    IBus<KeyType>* bus;
    std::string log_file_path; 
    int iterations_limit = 10;

    float calculate_gv(const Node<KeyType>& node, std::set<Edge<KeyType>> boundary_edges) const {
        NodeKey<KeyType> this_key = node.get_key();
        float internal_edges_weight = 0;
        float external_edges_weight = 0;

        std::map<NodeKey<KeyType>, Edge<KeyType>> boundary_edges_map;
        for (typename std::set<Edge<KeyType>>::const_iterator edge_it = boundary_edges.begin(); 
             edge_it != boundary_edges.end(); ++edge_it) {
            NodeKey<KeyType> other = edge_it->get_other(this_key);
            if (!(other == this_key)) {  
                boundary_edges_map[other] = *edge_it;
            }
        }

        for (typename std::map<NodeKey<KeyType>, Edge<KeyType>>::const_iterator edge_it = node.edges.begin(); edge_it != node.edges.end(); ++edge_it) {
            const NodeKey<KeyType>& neighbor_key = edge_it->first;
            const Edge<KeyType>& edge = edge_it->second;
            if (boundary_edges_map.find(neighbor_key) != boundary_edges_map.end()) {
                external_edges_weight += edge.get_weight();
            } else {
                internal_edges_weight += edge.get_weight();
            }
        }
        
        return external_edges_weight - internal_edges_weight;
    }

    // Вспомогательная функция для вычисления gain от обмена двух вершин
    float calculate_swap_gain(const Node<KeyType>& a, const Node<KeyType>& b,
                              std::set<Edge<KeyType>> boundary_edges_a,
                              std::set<Edge<KeyType>> boundary_edges_b) const {
        float gv_a = calculate_gv(a, boundary_edges_a);
        float gv_b = calculate_gv(b, boundary_edges_b);
        
        // Вычитаем 2 * вес ребра между a и b, если оно существует (cut edge)
        float cut_weight = 0.0f;
        typename std::map<NodeKey<KeyType>, Edge<KeyType>>::const_iterator edge_it = a.edges.find(b.get_key());
        if (edge_it != a.edges.end()) {
            cut_weight = edge_it->second.get_weight();
        }
        
        return gv_a + gv_b - 2.0f * cut_weight;
    }

    void swap(const Node<KeyType>& node_1, const Node<KeyType>& node_2, int storage_1, int storage_2) {
        bus->send_remove_node(node_1);
        bus->send_remove_node(node_2);
        bus->send_add_node(node_2, storage_1);
        bus->send_add_node(node_1, storage_2);
    }

    void swap(const NodeKey<KeyType>& key_1, const NodeKey<KeyType>& key_2, int storage_1, int storage_2) {
        Node<KeyType> node_1 = bus->request_node(key_1);
        Node<KeyType> node_2 = bus->request_node(key_2);
        bus->send_remove_node(node_1);
        bus->send_remove_node(node_2);
        bus->send_add_node(node_2, storage_1);
        bus->send_add_node(node_1, storage_2);
    }

public:
    KLExternalStorageOptimizer(IBus<KeyType>* _bus, std::string _log_file_path = "kl_experiment.log", int _iterations_limit = 10)
        : bus(_bus), log_file_path(_log_file_path), iterations_limit(_iterations_limit) {}

    std::map<int, std::map<Node<KeyType>, float>> calculate_gvs(int storage1, int storage2) const {
        std::map<int, std::map<Node<KeyType>, float>> result;
        std::set<Node<KeyType>> boundary_nodes1 = bus->ask_neigbours_to_storage(storage1, storage2);
        std::set<Node<KeyType>> boundary_nodes2 = bus->ask_neigbours_to_storage(storage2, storage1);

        std::set<Edge<KeyType>> boundary_edges1 = bus->ask_edges_to_storage(storage1, storage2);
        std::set<Edge<KeyType>> boundary_edges2 = bus->ask_edges_to_storage(storage2, storage1);

        result[storage1] = std::map<Node<KeyType>, float>();
        typename std::set<Node<KeyType>>::const_iterator it;
        for (it = boundary_nodes1.begin(); it != boundary_nodes1.end(); ++it) {
            const Node<KeyType>& node = *it;
            result[storage1][node] = calculate_gv(node, boundary_edges1);
        }

        result[storage2] = std::map<Node<KeyType>, float>();
        for (it = boundary_nodes2.begin(); it != boundary_nodes2.end(); ++it) {
            const Node<KeyType>& node = *it;
            result[storage2][node] = calculate_gv(node, boundary_edges2);
        }
        return result;
    };

    void optimize(int storage1, int storage2) override {
        if (iterations_limit == 0) return;

        // === ЛОГИРОВАНИЕ НАЧАЛЬНОГО РАЗРЕЗА ===
        double initial_cut = bus->get_inter_storage_cut_percent();
        double clean_initial_cut = initial_cut;
        std::cout << "KL [" << storage1 << "<->" << storage2 
                  << "] Initial cut: " << initial_cut << "%\n";

        int outer_iteration = 0;
        bool improved = true;

        while (outer_iteration < iterations_limit && improved) {
            std::cout << "KL outer iteration " << outer_iteration 
                      << " (storages " << storage1 << " <-> " << storage2 << ")\n";
            improved = false;

            std::set<Node<KeyType>> locked;
            std::vector<std::pair<Node<KeyType>, Node<KeyType>>> swap_sequence;
            std::vector<float> cumulative_gains;
            float max_gain = 0.0f;
            int best_prefix_length = 0;

            // Внутренний проход KL
            std::map<int, std::map<Node<KeyType>, float>> gvs = calculate_gvs(storage1, storage2);

            for (int step = 0; step < 40; ++step) {
                std::vector<std::pair<Node<KeyType>,float>> neg1, neg2;
                typename std::map<Node<KeyType>, float>::const_iterator git;

                for (git = gvs[storage1].begin(); git != gvs[storage1].end(); ++git) {
                    if (locked.find(git->first.get_key()) == locked.end()) {
                        neg1.push_back({git->first, git->second});
                    }
                }
                for (git = gvs[storage2].begin(); git != gvs[storage2].end(); ++git) {
                    if (locked.find(git->first.get_key()) == locked.end()) {
                        neg2.push_back({git->first, git->second});
                    }
                }

                if (neg1.empty() || neg2.empty()) {
                    std::cout << "No swappable neighbours (" << neg1.size() << ", " << neg2.size() << "), stop" << std::endl;
                    break;
                }

                std::sort(neg1.begin(), neg1.end(), [](const auto& a, const auto& b){ return a.second > b.second; });
                std::sort(neg2.begin(), neg2.end(), [](const auto& a, const auto& b){ return a.second > b.second; });

                float best_step_gain = std::numeric_limits<float>::min();
                Node<KeyType> best_a, best_b;

                for (const auto& pa : neg1) {
                    for (const auto& pb : neg2) {
                        float cut_weight = 0.0f;
                        auto edge_it = pa.first.edges.find(pb.first.get_key());
                        if (edge_it != pa.first.edges.end()) {
                            cut_weight = edge_it->second.get_weight();
                        }
                        float gain = pa.second + pb.second - 2.0f * cut_weight;

                        if (gain > best_step_gain) {
                            best_step_gain = gain;
                            best_a = pa.first;
                            best_b = pb.first;
                        }
                    }
                }

                if (best_step_gain == std::numeric_limits<float>::min()) {
                    std::cout << "No good swap" << std::endl;
                    break;
                }

                // Временный своп
                swap(best_a, best_b, storage1, storage2);

                swap_sequence.push_back({best_a, best_b});
                locked.insert(best_a);
                locked.insert(best_b);

                float current_cumulative = (cumulative_gains.empty() ? 0.0f : cumulative_gains.back()) + best_step_gain;
                cumulative_gains.push_back(current_cumulative);

                if (current_cumulative > max_gain) {
                    max_gain = current_cumulative;
                    best_prefix_length = swap_sequence.size();
                }

                // Обновление gvs
                std::set<Edge<KeyType>> boundary_edges1 = bus->ask_edges_to_storage(storage1, storage2);
                std::set<Edge<KeyType>> boundary_edges2 = bus->ask_edges_to_storage(storage2, storage1);

                std::set<Node<KeyType>> affected = {best_a, best_b};
                for (const auto& e : best_a.edges) affected.insert(e.first);
                for (const auto& e : best_b.edges) affected.insert(e.first);

                for (auto& p : neg1) {
                    if (affected.count(p.first.get_key())) {
                        float gv = calculate_gv(p.first, boundary_edges1);
                        p.second = gv;
                        gvs[storage1][p.first] = gv;
                    }
                }
                for (auto& p : neg2) {
                    if (affected.count(p.first.get_key())) {
                        float gv = calculate_gv(p.first, boundary_edges2);
                        p.second = gv;
                        gvs[storage2][p.first] = gv;
                    }
                }
            }

            // Применяем только лучший префикс
            if (max_gain > 0.0f && best_prefix_length > 0) {
                std::cout << "  Applying best prefix of " << best_prefix_length 
                          << " swaps with gain " << max_gain << "\n";
                improved = true;

                // Откат всех свопов
                for (int i = swap_sequence.size() - 1; i >= 0; --i) {
                    auto [a, b] = swap_sequence[i];
                    swap(a,b, storage2, storage1);
                }

                // Применяем только лучший префикс
                for (int i = 0; i < best_prefix_length; ++i) {
                    auto [a, b] = swap_sequence[i];
                    swap(a,b, storage1, storage2);
                }
            }

            outer_iteration++;
        }

        // === ЛОГИРОВАНИЕ ФИНАЛЬНОГО РАЗРЕЗА ===
        double final_cut = bus->get_inter_storage_cut_percent();
        std::cout << "KL [" << storage1 << "<->" << storage2 
                  << "] Final cut: " << final_cut << "% (change: " 
                  << (final_cut - clean_initial_cut) << "%)\n";

        // Логирование в файл
        std::ofstream log_file(log_file_path, std::ios::app);
        if (log_file.is_open()) {
            log_file << "KL " << storage1 << "-" << storage2 
                     << " iterations=" << outer_iteration 
                     << " initial_cut=" << clean_initial_cut 
                     << " final_cut=" << final_cut 
                     << " delta=" << (final_cut - clean_initial_cut)
                     << std::endl;
        }

        std::cout << "KL optimization finished after " << outer_iteration << " outer iterations\n";
    };
    
};

#endif // VKR_OPTIMIZER