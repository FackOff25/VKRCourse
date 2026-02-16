#ifndef VKR_COURSE_STORAGE
#define VKR_COURSE_STORAGE

#include "graph.hpp"
#include "interface_bus.hpp"
#include <set>
#include <optional>
#include <vector>
#include <algorithm>
#include <iostream>
#include <map>

template <typename KeyType>
class Storage {
private:
typedef Node<KeyType> StorageNode;
typedef NodeKey<KeyType> Key;
int storage_id;
std::map<Key, StorageNode> nodes;
// external\_edges[storage edge go to][external node][local node] = edge
std::map<int, std::map<Key, std::map<Key, Edge<KeyType>>>> external_edges;
IBus<KeyType> *bus = nullptr;

public:
Storage(int id) 
    :storage_id(id) {}
Storage(int id, std::map<Key, StorageNode> _nodes) 
    :storage_id(id), nodes(_nodes) {}

int get_id() const { return storage_id; };
void connect_to_bus(IBus<KeyType>* _bus) { bus = _bus; };

void get_add_announcement(Key key, int announcer_id, std::set<Edge<KeyType>> edges) {
    if (announcer_id == storage_id) return;

    for (typename std::set<Edge<KeyType>>::iterator edge_it = edges.begin(); edge_it != edges.end(); ++edge_it) {
        Key other_key = edge_it->get_other(key);
        typename std::map<Key, StorageNode>::iterator it = nodes.find(other_key);
        if (it != nodes.end()) {
            it->second.add_edge(*edge_it);
            typename std::map<Key, std::map<Key, Edge<KeyType>>>::iterator it2 = external_edges[announcer_id].find(key);
            if (it2 == external_edges[announcer_id].end()){
                external_edges[announcer_id][key] = std::map<Key, Edge<KeyType>>();
            }
            external_edges[announcer_id][key][other_key] = *edge_it;
        }
    }
};

void get_remove_announcement(Key key, int announcer_id) {
    if (announcer_id == storage_id) return;
    
    typename std::map<int, std::map<Key, std::map<Key, Edge<KeyType>>>>::iterator storage_it = 
        external_edges.find(announcer_id);
    if (storage_it == external_edges.end()) {
        return;
    }

    std::map<Key, std::map<Key, Edge<KeyType>>>& external_map = storage_it->second;
    typename std::map<Key, std::map<Key, Edge<KeyType>>>::iterator external_node_it = 
        external_map.find(key);
    if (external_node_it == external_map.end()) {
        return;
    }

    std::map<Key, Edge<KeyType>>& local_to_ext_map = external_node_it->second;
    
    // Удаляем рёбра из локальных узлов
    for (typename std::map<Key, Edge<KeyType>>::iterator local_nodes_it = local_to_ext_map.begin(); 
         local_nodes_it != local_to_ext_map.end(); ++local_nodes_it) {
        Key local_key = local_nodes_it->first;
        typename std::map<Key, StorageNode>::iterator node = nodes.find(local_key);
        if (node != nodes.end()) {
            node->second.remove_edge_to(key);
        }
    }
    
    // Удаляем запись из external\_edges
    external_map.erase(external_node_it);
    
    // Если для этого хранилища не осталось внешних узлов, удаляем запись
    if (external_map.empty()) {
        external_edges.erase(storage_it);
    }
}

std::optional<std::set<Edge<KeyType>>> add_node(const StorageNode& node){ 
    Key key = node.get_key();
    std::cout << storage_id << " adding " << key.key_value  << std::endl;
    typename std::map<Key, StorageNode>::iterator it = nodes.find(key);
    if (it != nodes.end()) {
        return std::nullopt;
    }
    nodes[key] = node;

    std::set<Edge<KeyType>> external_edges_to_announce;
    for (typename std::map<Key, Edge<KeyType>>::const_iterator edge_it = node.edges.begin(); edge_it != node.edges.end(); ++edge_it) {
        const Key& neighbor_key = edge_it->first;
        const Edge<KeyType>& edge = edge_it->second;
        std::cout << storage_id << " looks for " << neighbor_key.key_value  << std::endl;
        // Ищем соседа в текущем хранилище
        typename std::map<Key, StorageNode>::iterator it2 = nodes.find(neighbor_key);
        if (it2 != nodes.end()) {
            std::cout << storage_id << " node " << neighbor_key.key_value << " is inside, no asking"  << std::endl;
            // Сосед найден в этом же хранилище - добавляем обратное ребро
            it2->second.add_edge(edge);
        } else {
            std::cout << storage_id << " node " << neighbor_key.key_value << " is outside, asking"  << std::endl;
            // Сосед не найден - спрашиваем у шины, где он находится
            int neighbours_storage_id = bus->ask_who_has(storage_id, neighbor_key);
            std::cout << storage_id << " asked for " << neighbor_key.key_value << " answer: " << neighbours_storage_id << std::endl;
            if (neighbours_storage_id != -1) {
                // Сохраняем внешнее ребро
                external_edges[neighbours_storage_id][neighbor_key][key] = edge;
                external_edges_to_announce.insert(edge);
            }
            // Если сосед нигде не найден - игнорируем (возможно, будет добавлен позже)
        }
    }
    
    return external_edges_to_announce;
};

bool add_node_and_announce(const StorageNode& node) {
    if (bus == nullptr) {
        return false;
    };
    std::optional<std::set<Edge<KeyType>>> external_edges = add_node(node);
    if (!external_edges.has_value()) {
        return false;
    }
    bus->announce_add(node.get_key(), storage_id, external_edges.value());
    return true;
};


bool remove_node(const Key& key) {
    if (!has_node(key)) {
        return false;
    };
    StorageNode node = nodes.find(key)->second;
    nodes.erase(key);

    for (typename std::map<NodeKey<KeyType>, Edge<KeyType>>::iterator edge_it = node.edges.begin(); edge_it != node.edges.end(); ++edge_it) {
        Key other_key = edge_it->second.get_other(key);
        typename std::map<Key, StorageNode>::iterator it = nodes.find(other_key);
        if (it != nodes.end()) {
            it->second.remove_edge_to(key);
        }
    }

    // Удаляем все внешние рёбра, связанные с этим узлом
    typename std::map<int, std::map<Key, std::map<Key, Edge<KeyType>>>>::iterator ext_storage_it;
    for (ext_storage_it = external_edges.begin(); ext_storage_it != external_edges.end(); ) {
        std::map<Key, std::map<Key, Edge<KeyType>>>& ext_nodes = ext_storage_it->second;
        
        // Удаляем записи, где наш узел является локальным
        typename std::map<Key, std::map<Key, Edge<KeyType>>>::iterator ext_node_it;
        for (ext_node_it = ext_nodes.begin(); ext_node_it != ext_nodes.end(); ) {
            std::map<Key, Edge<KeyType>>& local_edges = ext_node_it->second;
            
            // Удаляем все рёбра, где локальный ключ - наш удаляемый узел
            typename std::map<Key, Edge<KeyType>>::iterator edge_it = local_edges.find(key);
            if (edge_it != local_edges.end()) {
                local_edges.erase(edge_it);
            }
            
            // Если после удаления для этого внешнего узла не осталось рёбер, удаляем запись
            if (local_edges.empty()) {
                ext_nodes.erase(ext_node_it++);
            } else {
                ++ext_node_it;
            }
        }
        
        // Если для этого хранилища не осталось внешних узлов, удаляем запись
        if (ext_nodes.empty()) {
            external_edges.erase(ext_storage_it++);
        } else {
            ++ext_storage_it;
        }
    }
    
    // Удаляем сам узел
    nodes.erase(node.get_key());

    return true;
};

bool remove_node_and_announce(const Key& key) {
    if (remove_node(key)) {
        bus->announce_remove(key, storage_id);
        return true;
    }
    return false;
};
   
StorageNode* get_node(const Key& key) {
    typename std::map<Key, StorageNode>::iterator it = nodes.find(key);
    if (it != nodes.end()) {
        return &(it->second);
    }
    return nullptr;
};


bool has_node(const Key& key) const {
    return nodes.find(key) != nodes.end();
}

const std::map<Key, StorageNode>& get_all_nodes() const {
    return nodes;
}

size_t size() const {
    return nodes.size();
}

void clear() {
    nodes.clear();
}

//
// Получение наборов связанных с другим хранилищем
//

// Получить набор узлов, имеющих соседей в указанном хранилище (копии)
std::set<StorageNode> get_nodes_with_neighbors_in_storage(int target_storage_id) const {
    std::set<StorageNode> result;

    typename std::map<int, std::map<Key,  std::map<Key, Edge<KeyType>>>>::const_iterator storage_it = external_edges.find(target_storage_id);
    if (storage_it == external_edges.end()) {
        return result;
    }

    const std::map<Key, std::map<Key, Edge<KeyType>>>& node_edges = storage_it->second;
    for (typename std::map<Key, std::map<Key, Edge<KeyType>>>::const_iterator node_edges_it = node_edges.begin(); node_edges_it != node_edges.end(); ++node_edges_it) {
        const std::map<Key, Edge<KeyType>>& local_node_edges = node_edges_it->second;
        for (typename std::map<Key, Edge<KeyType>>::const_iterator local_node_edges_it = local_node_edges.begin(); local_node_edges_it != local_node_edges.end(); ++local_node_edges_it) {
            Key local_key = local_node_edges_it->first;
            typename std::map<Key, StorageNode>::const_iterator node_it = nodes.find(local_key);
            if (node_it != nodes.end()) {
                result.insert(node_it->second);
            }
        }
    }
    
    return result;
}

std::set<Edge<KeyType>> get_all_edges_to_storage(int target_storage_id) const {
    std::set<Edge<KeyType>> result;
    typename std::map<int, std::map<Key, std::map<Key, Edge<KeyType>>>>::const_iterator storage_it = external_edges.find(target_storage_id);
    if (storage_it == external_edges.end()) {
        return std::set<Edge<KeyType>>();
    } else {
        const std::map<Key, std::map<Key, Edge<KeyType>>>& external_nodes_map = storage_it->second;
        for (typename std::map<Key, std::map<Key, Edge<KeyType>>>::const_iterator node_edges_it = external_nodes_map.begin(); node_edges_it != external_nodes_map.end(); ++node_edges_it) {
            const std::map<Key, Edge<KeyType>>& node_edge_map = node_edges_it->second;
            for (typename std::map<Key, Edge<KeyType>>::const_iterator edges_it = node_edge_map.begin(); edges_it != node_edge_map.end(); ++edges_it) {
                result.insert(edges_it->second);
            }
        }
    }
    return result;
}

typename std::map<Key, StorageNode>::iterator begin() {
    return nodes.begin();
}

typename std::map<Key, StorageNode>::iterator end() {
    return nodes.end();
}

typename std::map<Key, StorageNode>::const_iterator begin() const {
    return nodes.begin();
}

typename std::map<Key, StorageNode>::const_iterator end() const {
    return nodes.end();
}

typename std::map<Key, StorageNode>::const_iterator cbegin() const {
    return nodes.cbegin();
}

typename std::map<Key, StorageNode>::const_iterator cend() const {
    return nodes.cend();
}

bool empty() const {
    return nodes.empty();
}

friend std::ostream& operator<<(std::ostream& os, const Storage<KeyType>& storage) {
    os << "Storage(id: " << storage.get_id();
    
    if (storage.empty()) {
        os << ", empty";
    } else {
        os << ", nodes: " << storage.size() << " {";
        
        typename std::map<Key, StorageNode>::const_iterator it = storage.begin();
        if (it != storage.end()) {
            os << std::endl << "  " << it->second;
            ++it;
        }
        for (; it != storage.end(); ++it) {
            os << "," << std::endl << "  " << it->second;
        }
        os << std::endl << "}";
    }
    
    // Вывод внешних рёбер
    if (!storage.external_edges.empty()) {
        os << ", external_edges: {";
        
        typename std::map<int, std::map<Key, std::map<Key, Edge<KeyType>>>>::const_iterator storage_it = 
            storage.external_edges.begin();
        
        if (storage_it != storage.external_edges.end()) {
            os << std::endl << "  to storage " << storage_it->first << ": {";
            
            const std::map<Key, std::map<Key, Edge<KeyType>>>& ext_nodes = storage_it->second;
            typename std::map<Key, std::map<Key, Edge<KeyType>>>::const_iterator node_it = ext_nodes.begin();
            
            if (node_it != ext_nodes.end()) {
                os << std::endl << "    external node " << node_it->first.key_value << ": [";
                
                const std::map<Key, Edge<KeyType>>& local_edges = node_it->second;
                typename std::map<Key, Edge<KeyType>>::const_iterator edge_it = local_edges.begin();
                
                if (edge_it != local_edges.end()) {
                    os << std::endl << "      local node " << edge_it->first.key_value 
                       << " -> " << edge_it->second;
                    ++edge_it;
                }
                for (; edge_it != local_edges.end(); ++edge_it) {
                    os << "," << std::endl << "      local node " << edge_it->first.key_value 
                       << " -> " << edge_it->second;
                }
                os << std::endl << "    ]";
                ++node_it;
            }
            for (; node_it != ext_nodes.end(); ++node_it) {
                os << "," << std::endl << "    external node " << node_it->first.key_value << ": [";
                
                const std::map<Key, Edge<KeyType>>& local_edges = node_it->second;
                typename std::map<Key, Edge<KeyType>>::const_iterator edge_it = local_edges.begin();
                
                if (edge_it != local_edges.end()) {
                    os << std::endl << "      local node " << edge_it->first.key_value 
                       << " -> " << edge_it->second;
                    ++edge_it;
                }
                for (; edge_it != local_edges.end(); ++edge_it) {
                    os << "," << std::endl << "      local node " << edge_it->first.key_value 
                       << " -> " << edge_it->second;
                }
                os << std::endl << "    ]";
            }
            os << std::endl << "  }";
            ++storage_it;
        }
        
        for (; storage_it != storage.external_edges.end(); ++storage_it) {
            os << "," << std::endl << "  to storage " << storage_it->first << ": {";
            
            const std::map<Key, std::map<Key, Edge<KeyType>>>& ext_nodes = storage_it->second;
            typename std::map<Key, std::map<Key, Edge<KeyType>>>::const_iterator node_it = ext_nodes.begin();
            
            if (node_it != ext_nodes.end()) {
                os << std::endl << "    external node " << node_it->first.key_value << ": [";
                
                const std::map<Key, Edge<KeyType>>& local_edges = node_it->second;
                typename std::map<Key, Edge<KeyType>>::const_iterator edge_it = local_edges.begin();
                
                if (edge_it != local_edges.end()) {
                    os << std::endl << "      local node " << edge_it->first.key_value 
                       << " -> " << edge_it->second;
                    ++edge_it;
                }
                for (; edge_it != local_edges.end(); ++edge_it) {
                    os << "," << std::endl << "      local node " << edge_it->first.key_value 
                       << " -> " << edge_it->second;
                }
                os << std::endl << "    ]";
                ++node_it;
            }
            for (; node_it != ext_nodes.end(); ++node_it) {
                os << "," << std::endl << "    external node " << node_it->first.key_value << ": [";
                
                const std::map<Key, Edge<KeyType>>& local_edges = node_it->second;
                typename std::map<Key, Edge<KeyType>>::const_iterator edge_it = local_edges.begin();
                
                if (edge_it != local_edges.end()) {
                    os << std::endl << "      local node " << edge_it->first.key_value 
                       << " -> " << edge_it->second;
                    ++edge_it;
                }
                for (; edge_it != local_edges.end(); ++edge_it) {
                    os << "," << std::endl << "      local node " << edge_it->first.key_value 
                       << " -> " << edge_it->second;
                }
                os << std::endl << "    ]";
            }
            os << std::endl << "  }";
        }
        
        os << std::endl << "}";
    }
    
    os << ")";
    return os;
}
};

#endif // VKR\_COURSE\_STORAGE