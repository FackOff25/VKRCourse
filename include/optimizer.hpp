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

std::map<int, std::set<Node<KeyType>>> get_negative_gvs(std::map<int, std::map<Node<KeyType>, float>> full_map) {
    std::map<int, std::set<Node<KeyType>>> result;
    typename std::map<int, std::map<Node<KeyType>, float>>::const_iterator it;
    for (it = full_map.begin(); it != full_map.end(); ++it) {
        result[it->first] = std::set<Node<KeyType>>();
        const std::map<Node<KeyType>, float>& nodes = it->second;
        typename std::map<Node<KeyType>, float>::const_iterator it2;
        for (it2 = nodes.begin(); it2 != nodes.end(); ++it2) {
            if (it2->second < 0) {
                result[it->first].insert(it2->first);
            }
        }
        if (result[it->first].empty()) {
            result.erase(it->first);
        }
    }
    return result;
};
    
public:
ExternalStorageOptimizer(IBus<KeyType>* _bus)
    : bus(_bus) {}

std::map<int, std::map<Node<KeyType>, float>> calculate_gvs(int storage1, int storage2) const {
    std::map<int, std::map<Node<KeyType>, float>> result;
    // Получаем граничные вершины для обоих хранилищ
    std::set<Node<KeyType>> boundary_nodes1 = bus->ask_neigbours_to_storage(storage1, storage2);
    std::set<Node<KeyType>> boundary_nodes2 = bus->ask_neigbours_to_storage(storage2, storage1);

    std::set<Edge<KeyType>> boundary_edges1 = bus->ask_edges_to_storage(storage1, storage2);
    std::set<Edge<KeyType>> boundary_edges2 = bus->ask_edges_to_storage(storage2, storage1);

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
};

void optimize(int storage1, int storage2, int iterations_limit = 5) {
    if (iterations_limit == 0) return;
    
    int iteration = 0;
    std::map<int, std::set<Node<KeyType>>> negative_gvs = get_negative_gvs(calculate_gvs(storage1, storage2));
    do {
        typename std::map<int, std::set<Node<KeyType>>>::iterator map_it;
        for (map_it = negative_gvs.begin(); map_it != negative_gvs.end(); ++map_it) {
            int this_storage = map_it->first;
            int other_storage = this_storage == storage1 ? storage2 : storage1;
            std::set<Node<KeyType>>& nodes = map_it->second;
            typename std::set<Node<KeyType>>::const_iterator set_it;
            for (set_it = nodes.begin(); set_it != nodes.end(); ++set_it) {
                const Node<KeyType>& node = *set_it;
                Node<int> removed = bus->request_node(node.get_key());
                bus->send_remove_node(removed.get_key(), this_storage);
                bus->send_add_node(removed, other_storage);
            }
        }
        ++iteration;
        negative_gvs = get_negative_gvs(calculate_gvs(storage1, storage2));
    } while (iteration < iterations_limit && !negative_gvs.empty());
}
};

#endif // VKR\_OPTIMIZER