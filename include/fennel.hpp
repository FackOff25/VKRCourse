#ifndef VKR_FENNEL
#define VKR_FENNEL

#include "storage.hpp"
#include "graph.hpp"
#include <set>
#include <algorithm>
#include <cmath>

struct FennelParameters {
    /*k*/int storage_number;
    /*gamma*/float balancing_number;
};

template <typename KeyType> 
class FennelStorage : public Storage<KeyType> {
protected:
typedef Node<KeyType> StorageNode;
typedef NodeKey<KeyType> Key;
FennelParameters params;
//float number_of_edges;
int number_of_nodes;
float streaming_euristics = 0;

float get_p_parameter(int number_of_nodes, long number_of_edges){
    if (number_of_nodes == 0) {
        return 0;
    } 
    return 2 * number_of_edges * std::pow(params.storage_number , (params.balancing_number - 1)) / std::pow(number_of_nodes, params.balancing_number);
}

float fennel_function(const Node<KeyType>& node, long total_edges) {
    float p = get_p_parameter(number_of_nodes, total_edges);
    float dg = 0;
    for (typename std::map<Key, Edge<KeyType>>::const_iterator edge_it = node.edges.begin(); edge_it != node.edges.end(); ++edge_it) {
        const Key& neighbor_key = edge_it->first;
        const Edge<KeyType>& edge = edge_it->second;
        // Ищем соседа в текущем хранилище
        typename std::map<Key, StorageNode>::iterator it2 = this->nodes.find(neighbor_key);
        if (it2 != this->nodes.end()) {
            ++dg;
        }
    }
    dg -= p * this->size();
    return dg;
}
public:

FennelStorage(int id, FennelParameters _params, std::map<Key, StorageNode> _nodes = std::map<Key, StorageNode>()) 
    : Storage<KeyType>(id, _nodes), params(_params) {}

float get_streaming_euristics_change(const Node<KeyType>& node, long total_edges) override {
    return fennel_function(node, total_edges);
}

void get_add_announcement(Key key, int announcer_id, std::set<Edge<KeyType>> edges) override {
    ++number_of_nodes;
    Storage<KeyType>::get_add_announcement(key, announcer_id, edges);
};

void get_remove_announcement(Key key, int announcer_id) override {
    --number_of_nodes;
    Storage<KeyType>::get_remove_announcement(key, announcer_id);
}
};

#endif // VKR\_FENNEL