#ifndef VKR_BUS
#define VKR_BUS

#include "i_bus.hpp"
#include "i_storage.hpp"
#include <map>
#include <vector>
#include <random>
#include <set>

template <typename KeyType> 
class SimpleBus : public IBus<KeyType> {
private:
std::mt19937 gen;
std::map<int, IStorage<KeyType>*> storages;
long total_edges = 0;
public:
SimpleBus(unsigned int seed_value = 42) : gen(seed_value) {};

int connect_storage(IStorage<KeyType>* storage) {
    if (storages.find(storage->get_id()) != storages.end()) {
        return -1;
    }
    storages[storage->get_id()] = storage;
    storage->connect_to_bus(this);
    return storage->get_id();
};

Node<KeyType> request_node(const NodeKey<KeyType>& node) override {
    for (typename std::map<int, IStorage<KeyType>*>::iterator it = storages.begin(); it != storages.end(); ++it) {
        Node<KeyType>* node_pointer = it->second->get_node(node);
        if (node_pointer != nullptr) {
            return Node<KeyType>(*node_pointer);
            break;
        };
    }
    return Node<KeyType>();
};

int send_add_node(const Node<KeyType>& node) override {
    std::vector<IStorage<KeyType>*> best_storages;
    float best_euristics = -10000000;

    std::cout << "Adding node " << node;
    for (typename std::map<int, IStorage<KeyType>*>::iterator it = storages.begin(); it != storages.end(); ++it) {
        float this_euristics = it->second->get_streaming_euristics_change(node, total_edges);
        std::cout << ", storage " << it->second->get_id() << " has euristics " << this_euristics;
        if (this_euristics > best_euristics) {
            best_storages.clear();
            best_storages.push_back(it->second);
            best_euristics = this_euristics;
        } else if (this_euristics == best_euristics) {
            best_storages.push_back(it->second);
        }
    }
    std::cout << std::endl;
    if (best_storages.empty()) {
        return -1;
    }
    std::uniform_int_distribution<> dist{0, static_cast<int>(best_storages.size()) - 1};
    IStorage<KeyType>* chosen = best_storages[dist(gen)];

    std::cout << " → Chosen storage: " << chosen->get_id() << std::endl;

    send_add_node(node, chosen->get_id());
    return chosen->get_id();
};

bool send_add_node(const Node<KeyType>& node, int storage_id) override {
    if (storages.find(storage_id) == storages.end()) {
        return false;
    }
    size_t old_edges_num = storages[storage_id]->internal_edges_size();
    std::optional<std::set<Edge<KeyType>>> result = storages[storage_id]->add_node_and_announce(node);
    if (!result.has_value()) {
        return false;
    }
    total_edges += result.value().size();
    total_edges = total_edges - old_edges_num + storages[storage_id]->internal_edges_size();
    //std::cout << "Total number of edges: "  << total_edges << std::endl;
    return true;
};

bool send_remove_node(const NodeKey<KeyType>& node) override {
    bool success = false;
    // TODO: implement counter change when remove
    for (typename std::map<int, IStorage<KeyType>*>::iterator it = storages.begin(); it != storages.end(); ++it) {
        if (it->second->remove_node_and_announce(node)) {
            success = true;
            break;
        };
    }
    return success;
};

bool send_remove_node(const NodeKey<KeyType>& node, int storage_id) override {
    if (storages.find(storage_id) == storages.end()) {
        return false;
    }
    return storages[storage_id]->remove_node_and_announce(node);
};

int ask_who_has(int asker_id, NodeKey<KeyType> key) override{
    for (typename std::map<int, IStorage<KeyType>*>::iterator it = storages.begin(); it != storages.end(); ++it) {
        if (it->second != nullptr && it->second->has_node(key)) {
            return it->first;
        };
    }
    return -1;
};

void announce_add(NodeKey<KeyType> key, int storage_id, std::set<Edge<KeyType>> edges) override {
    for (typename std::map<int, IStorage<KeyType>*>::iterator it = storages.begin(); it != storages.end(); ++it) {
        if (it->second != nullptr) {
            it->second->get_add_announcement(key, storage_id, edges);
        };
    }
};

void announce_remove(NodeKey<KeyType> key, int storage_id) override {
    for (typename std::map<int, IStorage<KeyType>*>::iterator it = storages.begin(); it != storages.end(); ++it) {
        if (it->second != nullptr) {
            it->second->get_remove_announcement(key, storage_id);
        };
    }
};

float get_streaming_euristics_change(const Node<KeyType>& node, int storage_id) override {
    if (storages.find(storage_id) == storages.end()) {
        return false;
    }
    return storages[storage_id]->get_streaming_euristics_change(node, total_edges);
}

// запрашивает у source вершины, соседствующие с target
std::set<Node<KeyType>> ask_neigbours_to_storage(int source, int target) {
    if (storages.find(source) == storages.end()) {
        return std::set<Node<KeyType>>();
    }
    return storages[source]->get_nodes_with_neighbors_in_storage(target);
}

std::set<Edge<KeyType>> ask_edges_to_storage(int source, int target) {
    if (storages.find(source) == storages.end()) {
        return std::set<Edge<KeyType>>();
    }
    return storages[source]->get_all_edges_to_storage(target);
}

LocalPathResult<KeyType> request_local_path(
    int storage_id,
    const NodeKey<KeyType>& start,
    const NodeKey<KeyType>& goal,
    const std::unordered_set<NodeKey<KeyType>>& global_visited = {}) override 
{
    auto it = storages.find(storage_id);
    if (it == storages.end() || it->second == nullptr) {
        return {};
    }

    return it->second->find_local_path(start, goal, global_visited);
}

void adjust_weights(std::vector<NodeKey<KeyType>> path) {
    for (typename std::map<int, IStorage<KeyType>*>::iterator it = storages.begin(); it != storages.end(); ++it) {
        it->second->adjust_weight(path);
    }
}

double get_inter_storage_cut_percent() override {
    if (storages.size() < 2) return 0.0;

    long total_internal = 0;
    long total_external = 0;

    for (const auto& pair : storages) {
        if (!pair.second) continue;
        const BaseStorage<KeyType>* base = dynamic_cast<const BaseStorage<KeyType>*>(pair.second);
        if (!base) continue;

        total_internal += base->internal_edges_size();
        total_external += base->external_edges_size();
    }

    // external_edges считаются в обоих направлениях → делим на 2
    long inter_edges = total_external / 2;
    long total_edges = total_internal + inter_edges;

    if (total_edges == 0) return 0.0;
    return (static_cast<double>(inter_edges) / total_edges) * 100.0;
}

};

#endif // VKR\_BUS