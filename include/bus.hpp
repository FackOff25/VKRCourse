#ifndef VKR_BUS
#define VKR_BUS

#include "interface_bus.hpp"
#include "storage.hpp"
#include <unordered_map>

template <typename KeyType> 
class SimpleBus : public IBus<KeyType> {
private:

public:
    int connect_storage(Storage<KeyType> storage);
    void send_node(NodeKey<KeyType> key);
    void send_node(NodeKey<KeyType> key, int storage_id);
    int ask_who_has(NodeKey<KeyType> key);
    void announce_add(NodeKey<KeyType> key, int storage_id, std::unordered_set<Edge<KeyType>> edges);
    void announce_remove(NodeKey<KeyType> key, int storage_id);
};

#endif // VKR\_BUS