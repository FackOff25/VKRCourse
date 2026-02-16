#ifndef VKR_OPTIMIZER
#define VKR_OPTIMIZER

#include "interface_bus.hpp"

#include <stdlib.h>
#include <iostream>
#include <map>

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

std::map<int, std::map<Node<KeyType>, float>> calculate_gvs(int storage1, int storage2) const {
    std::map<int, std::map<Node<KeyType>, float>> result;
    // Получаем граничные вершины для обоих хранилищ
    std::set<Node<KeyType>> boundary_nodes1 = bus->ask_neigbours_to_storage(storage1, storage2);
    std::set<Node<KeyType>> boundary_nodes2 = bus->ask_neigbours_to_storage(storage2, storage1);

    std::map<NodeKey<KeyType>, Edge<KeyType>> boundary_edges1 = bus->ask_edges_to_storage(1, 2);
    std::map<NodeKey<KeyType>, Edge<KeyType>> boundary_edges2 = bus->ask_edges_to_storage(2, 1);

    result[storage1] = std::map<Node<KeyType>, float>();
    typename std::set<Node<KeyType>>::const_iterator it;
    for (it = boundary_nodes1.begin(); it != boundary_nodes1.end(); ++it) {
        const Node<KeyType>& node = *it;
        result[storage1][node.get_key()] = calculate_gv(node, boundary_edges1);
    }

    result[storage2] = std::map<Node<KeyType>, float>();
    for (it = boundary_nodes2.begin(); it != boundary_nodes2.end(); ++it) {
        const Node<KeyType>& node = *it;
        result[storage2][node.get_key()] = calculate_gv(node, boundary_edges2);
    }
    return result;
}
};

#endif // VKR\_OPTIMIZER