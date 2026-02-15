#include "../include/bus.hpp"

template <typename KeyType> 
int SimpleBus<KeyType>::connect_storage(Storage<KeyType>* storage) {
    if (storages.contains(storage->get_id())){
        return -1;
    };
    storages[storage->get_id()] = storage;
    return storage->get_id();
}
/*
void send_node(NodeKey<KeyType> key);
void send_node(NodeKey<KeyType> key, int storage_id);
int ask_who_has(NodeKey<KeyType> key);
void announce_add(NodeKey<KeyType> key, int storage_id, std::unordered_set<Edge<KeyType>> edges);
void announce_remove(NodeKey<KeyType> key, int storage_id);*/

template <typename KeyType> 
void SimpleBus<KeyType>::send_node(NodeKey<KeyType> key, int storage_id){

}
