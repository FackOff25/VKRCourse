#ifndef VKR_OPTIMIZER
#define VKR_OPTIMIZER

#include "interface_bus.hpp"

#include <stdlib.h>
#include <iostream>

/*template <typename KeyType>
class StorageOptimizer0 {
private:
    const Storage<KeyType>& storage1;
    const Storage<KeyType>& storage2;

    int calculate_gv(const Node<KeyType>& node, int other_storage_id) const {
        int internal_edges_weight = node.get_internal_edges_weight_sum();
        int external_edges_weight = node.get_external_edges_weight_sum_to_storage(other_storage_id);
        
        return internal_edges_weight - external_edges_weight;
    }
    
public:
    StorageOptimizer0(const Storage<KeyType>& s1, const Storage<KeyType>& s2)
        : storage1(s1), storage2(s2) {}

    void calculate_gvs() const {
        // Получаем граничные вершины для обоих хранилищ
        std::unordered_map<KeyType, Node<KeyType>> boundary_nodes1 = storage1.get_nodes_with_neighbors_in_storage_map_copy(storage2.get_id());
        std::unordered_map<KeyType, Node<KeyType>> boundary_nodes2 = storage2.get_nodes_with_neighbors_in_storage_map_copy(storage1.get_id());

        typename std::unordered_map<KeyType, Node<KeyType>>::const_iterator it;
        for (it = boundary_nodes1.begin(); it != boundary_nodes1.end(); ++it) {
            const Node<KeyType>& node = it->second;
            
            std::cout << "Metric for node " << node.key.key_value << " is " << calculate_gv(node, storage2.get_id()) << std::endl;
        }

        for (it = boundary_nodes2.begin(); it != boundary_nodes2.end(); ++it) {
            const Node<KeyType>& node = it->second;
            
            std::cout << "Metric for node " << node.key.key_value << " is " << calculate_gv(node, storage1.get_id()) << std::endl;
        }
    }
};*/

template <typename KeyType>
class ExternalStorageOptimizer {
private:
IBus<KeyType>* bus;

float calculate_gv(const Node<KeyType>& node, std::map<NodeKey<KeyType>, Edge<KeyType>> boundary_edges) const {
    NodeKey<KeyType> this_key = node.get_key();
    float internal_edges_weight = 0;
    float external_edges_weight = 0;

    for (typename std::map<NodeKey<KeyType>, Edge<KeyType>>::const_iterator edge_it = node.edges.begin(); edge_it != node.edges.end(); ++edge_it) {
        const NodeKey<KeyType>& neighbor_key = edge_it->first;
        const Edge<KeyType>& edge = edge_it->second;
        if (boundary_edges.find(neighbor_key) != boundary_edges.end()) {
            external_edges_weight += edge.get_weight();
        } else {
            internal_edges_weight += edge.get_weight();
        }
    }
    
    return internal_edges_weight - external_edges_weight;
}
    
public:
ExternalStorageOptimizer(IBus<KeyType>* _bus)
    : bus(_bus) {}

void calculate_gvs(int storage1, int storage2) const {
    // Получаем граничные вершины для обоих хранилищ
    std::set<Node<KeyType>> boundary_nodes1 = bus->ask_neigbours_to_storage(storage1, storage2);
    std::set<Node<KeyType>> boundary_nodes2 = bus->ask_neigbours_to_storage(storage2, storage1);

    std::map<NodeKey<KeyType>, Edge<KeyType>> boundary_edges1 = bus->ask_edges_to_storage(1, 2);
    std::map<NodeKey<KeyType>, Edge<KeyType>> boundary_edges2 = bus->ask_edges_to_storage(2, 1);

    typename std::set<Node<KeyType>>::const_iterator it;
    for (it = boundary_nodes1.begin(); it != boundary_nodes1.end(); ++it) {
        const Node<KeyType>& node = *it;
        
        std::cout << "Metric for node " << node.get_key().key_value << " is " << calculate_gv(node, boundary_edges1) << std::endl;
    }

    for (it = boundary_nodes2.begin(); it != boundary_nodes2.end(); ++it) {
        const Node<KeyType>& node = *it;
        
        std::cout << "Metric for node " << node.get_key().key_value << " is " << calculate_gv(node, boundary_edges2) << std::endl;
    }
}
};

#endif // VKR\_OPTIMIZER