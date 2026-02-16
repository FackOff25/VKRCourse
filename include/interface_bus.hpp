#ifndef VKR_IBUS
#define VKR_IBUS

#include "graph.hpp"
#include <set>

template <typename KeyType> 
class IBus {
public:
    virtual int send_node(const Node<KeyType>& node) = 0;
    virtual bool send_node(const Node<KeyType>& node, int storage_id) = 0;
    virtual int ask_who_has(int asker_id, NodeKey<KeyType> key) = 0;
    virtual void announce_add(NodeKey<KeyType> key, int storage_id, std::set<Edge<KeyType>> edges) = 0;
    virtual void announce_remove(NodeKey<KeyType> key, int storage_id) = 0;
    // запрашивает у source вершины, соседствующие с target
    virtual std::set<Node<KeyType>> ask_neigbours_to_storage(int source, int target) = 0;
    // запрашивает у source рёбра, идущие в target
    virtual std::map<NodeKey<KeyType>, Edge<KeyType>> ask_edges_to_storage(int source, int target) = 0;
};

#endif // VKR\_IBUS