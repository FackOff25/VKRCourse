#ifndef VKR_IBUS
#define VKR_IBUS

#include "graph.hpp"
#include <unordered_set>

template <typename KeyType> 
class IBus {
public:
    virtual void send_node(NodeKey<KeyType> key);
    virtual void send_node(NodeKey<KeyType> key, int storage_id);
    virtual int ask_who_has(NodeKey<KeyType> key);
    virtual void announce_add(NodeKey<KeyType> key, int storage_id, std::unordered_set<Edge<KeyType>> edges);
    virtual void announce_remove(NodeKey<KeyType> key, int storage_id);
};

#endif // VKR\_IBUS