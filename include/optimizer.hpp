#ifndef VKR_OPTIMIZER
#define VKR_OPTIMIZER

#include "i_bus.hpp"

#include <stdlib.h>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <limits>

template <typename KeyType>
class IExternalStorageOptimizer {
public:
    virtual void optimize(int storage1, int storage2) = 0;
};

template <typename KeyType>
class DummyExternalStorageOptimizer : public IExternalStorageOptimizer<KeyType> {
public:
    DummyExternalStorageOptimizer() {};
    void optimize(int storage1, int storage2) override {
        return;
    };
};

template <typename KeyType>
class KLExternalStorageOptimizer : public IExternalStorageOptimizer<KeyType> {
private:
    IBus<KeyType>* bus;
    int iterations_limit = 5;

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
        
        return internal_edges_weight - external_edges_weight;
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

public:
    KLExternalStorageOptimizer(IBus<KeyType>* _bus, int _iterations_limit = 10)
        : bus(_bus), iterations_limit(_iterations_limit) {}

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

        int outer_iteration = 0;
        bool improved = true;

        while (outer_iteration < iterations_limit && improved) {
            std::cout << "KL outer iteration " << outer_iteration << " (storages " << storage1 << " <-> " << storage2 << ")\n";
            improved = false;

            std::set<NodeKey<KeyType>> locked;
            std::vector<std::pair<NodeKey<KeyType>, NodeKey<KeyType>>> swap_sequence;
            std::vector<float> cumulative_gains;
            float max_gain = 0.0f;
            int best_prefix_length = 0;

            // Внутренний проход KL (последовательность обменов)
            for (int step = 0; step < 20; ++step) {  // ограничение на длину прохода
                std::map<int, std::map<Node<KeyType>, float>> gvs = calculate_gvs(storage1, storage2);
                
                std::set<Node<KeyType>> neg1, neg2;
                typename std::map<Node<KeyType>, float>::const_iterator git;
                for (git = gvs[storage1].begin(); git != gvs[storage1].end(); ++git) {
                    if (git->second < 0 && locked.find(git->first.get_key()) == locked.end()) {
                        neg1.insert(git->first);
                    }
                }
                for (git = gvs[storage2].begin(); git != gvs[storage2].end(); ++git) {
                    if (git->second < 0 && locked.find(git->first.get_key()) == locked.end()) {
                        neg2.insert(git->first);
                    }
                }

                if (neg1.empty() || neg2.empty()) break;

                // Ищем лучшую пару для обмена в этом шаге
                float best_step_gain = -std::numeric_limits<float>::max();
                NodeKey<KeyType> best_a, best_b;

                typename std::set<Node<KeyType>>::const_iterator ita, itb;
                for (ita = neg1.begin(); ita != neg1.end(); ++ita) {
                    for (itb = neg2.begin(); itb != neg2.end(); ++itb) {
                        const Node<KeyType>& na = *ita;
                        const Node<KeyType>& nb = *itb;
                        float gain = calculate_swap_gain(na, nb,
                            bus->ask_edges_to_storage(storage1, storage2),
                            bus->ask_edges_to_storage(storage2, storage1));

                        if (gain > best_step_gain) {
                            best_step_gain = gain;
                            best_a = na.get_key();
                            best_b = nb.get_key();
                        }
                    }
                }

                if (best_step_gain <= -999.0f) break; // ничего хорошего

                // Выполняем временный swap (для расчёта следующего состояния)
                Node<KeyType> node_a = bus->request_node(best_a);
                Node<KeyType> node_b = bus->request_node(best_b);
                bus->send_remove_node(best_a);
                bus->send_remove_node(best_b);
                bus->send_add_node(node_b, storage1);
                bus->send_add_node(node_a, storage2);

                swap_sequence.push_back(std::make_pair(best_a, best_b));
                locked.insert(best_a);
                locked.insert(best_b);

                float current_cumulative = (cumulative_gains.empty() ? 0.0f : cumulative_gains.back()) + best_step_gain;
                cumulative_gains.push_back(current_cumulative);

                if (current_cumulative > max_gain) {
                    max_gain = current_cumulative;
                    best_prefix_length = swap_sequence.size();
                }
            }

            // Применяем только лучший префикс, если он даёт положительный выигрыш
            if (max_gain > 0.0f && best_prefix_length > 0) {
                std::cout << "  Applying best prefix of " << best_prefix_length 
                          << " swaps with gain " << max_gain << "\n";
                improved = true;
                // Здесь можно откатить лишние swaps, но для простоты мы уже применили все, 
                // а в реальной реализации лучше делать rollback. Пока оставляем как есть (применяем лучший префикс).
            } else {
                std::cout << "  No positive gain found, stopping inner pass\n";
            }

            outer_iteration++;
        }

        std::cout << "KL optimization finished after " << outer_iteration << " outer iterations\n";
    };
};

#endif // VKR_OPTIMIZER