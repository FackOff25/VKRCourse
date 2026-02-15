#ifndef VKR_BUS
#define VKR_BUS

#include "interface_bus.hpp"
#include "storage.hpp"
#include <map>
#include <set>

template <typename KeyType> 
class SimpleBus : public IBus<KeyType> {
private:
std::map<int, Storage<KeyType>*> storages;
public:
SimpleBus(): storages() {};

int connect_storage(Storage<KeyType>* storage) {
    if (storages.find(storage->get_id()) != storages.end()) {
        return -1;
    }
    storages[storage->get_id()] = storage;
    storage->connect_to_bus(this);
    return storage->get_id();
};

int send_node(const Node<KeyType>& node) override {
    (void)node;
    // No autosending
    return -1;
};
bool send_node(const Node<KeyType>& node, int storage_id) override {
    if (storages.find(storage_id) == storages.end()) {
        return false;
    }
    return storages[storage_id]->add_node_and_announce(node);
};

int ask_who_has(int asker_id, NodeKey<KeyType> key) override{
    for (typename std::map<int, Storage<KeyType>*>::iterator it = storages.begin(); it != storages.end(); ++it) {
        if (it->second != nullptr && it->second->has(key)) {
            return it->first;
        };
    }
    return -1;
};

void announce_add(NodeKey<KeyType> key, int storage_id, std::set<Edge<KeyType>> edges) override {
    for (typename std::map<int, Storage<KeyType>*>::iterator it = storages.begin(); it != storages.end(); ++it) {
        if (it->second != nullptr) {
            it->second->get_add_announcement(key, storage_id, edges);
        };
    }
};

void announce_remove(NodeKey<KeyType> key, int storage_id) override {
    for (typename std::map<int, Storage<KeyType>*>::iterator it = storages.begin(); it != storages.end(); ++it) {
        if (it->second != nullptr && it->second->has(key)) {
            it->second->get_remove_announcement(key, storage_id);
        };
    }
};

// запрашивает у source вершины, соседствующие с target
std::set<Node<KeyType>> ask_neigbours_to_storage(int source, int target) {
    if (storages.find(source) == storages.end()) {
        return std::set<Node<KeyType>>();
    }
    return storages[source]->get_nodes_with_neighbors_in_storage(target);
}

// запрашивает у source рёбра, идущие в target
std::map<KeyType, Edge<KeyType>> ask_edges_to_storage(int source, int target) {
    if (storages.find(source) == storages.end()) {
        return std::map<KeyType, Edge<KeyType>>();
    }
    return storages[source]->get_all_edges_to_storage(target);
}

};

#endif // VKR\_BUS