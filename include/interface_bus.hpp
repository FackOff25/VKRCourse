#ifndef VKR_IBUS
#define VKR_IBUS

#include "graph.hpp"
#include <set>

template <typename KeyType> 
class IBus {
public:
    virtual int send_node(const Node<KeyType>& node) = 0;
    virtual bool send_node(const Node<KeyType>& node, int storage_id) = 0;
    virtual int ask_who_has(NodeKey<KeyType> key) = 0;
    virtual void announce_add(NodeKey<KeyType> key, int storage_id, std::set<Edge<KeyType>> edges) = 0;
    virtual void announce_remove(NodeKey<KeyType> key, int storage_id) = 0;
};

#endif // VKR\_IBUS