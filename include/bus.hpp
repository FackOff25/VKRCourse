#ifndef VKR_BUS
#define VKR_BUS

#include "interface_bus.hpp"
#include "i_storage.hpp"
#include <map>
#include <vector>
#include <random>
#include <set>

template <typename KeyType> 
class SimpleBus : public IBus<KeyType> {
private:
std::random_device seed;
std::mt19937 gen{seed()};
std::map<int, IStorage<KeyType>*> storages;
long total_edges = 0;
public:
SimpleBus(): storages() {};

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
    } else {
        std::uniform_int_distribution<> dist{0, best_storages.size() - 1};
        IStorage<KeyType>* random_storage = best_storages[dist(gen)];
        send_add_node(node, random_storage->get_id());
        return random_storage->get_id();
    }
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

// запрашивает у source рёбра, идущие в target
std::set<Edge<KeyType>> ask_edges_to_storage(int source, int target) {
    if (storages.find(source) == storages.end()) {
        return std::set<Edge<KeyType>>();
    }
    return storages[source]->get_all_edges_to_storage(target);
}

};

#endif // VKR\_BUS