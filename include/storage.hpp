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
// external_edges[storage edge go to][external node][local node] = edge
std::map<int, std::map<Key, std::map<Key, Edge<KeyType>>>> external_edges;
IBus<KeyType> *bus = nullptr;

public:
Storage(int id) 
    :storage_id(id) {}
Storage(int id, std::map<Key, StorageNode> _nodes) 
    :storage_id(id), nodes(_nodes) {}

int get_id() const { return storage_id; };
void connect_to_bus(IBus<KeyType>* _bus) { bus = _bus; };
bool has(Key key) {std::cout << storage_id << " was asked for " << key.key_value << " answer: " << (nodes.find(key) != nodes.end()) << std::endl; return nodes.find(key) != nodes.end(); };

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

void get_remove_announcement(Key key, int storage_id) {
    typename std::map<int, std::map<Key, std::map<Key, Edge<KeyType>>>>::const_iterator storage_it = external_edges.find(storage_id);
    if (storage_it == external_edges.end()) {
        return;
    }

    const std::map<Key, std::map<Key, Edge<KeyType>>>& external_map = storage_it->second;
    typename std::map<Key, std::map<Key, Edge<KeyType>>>::const_iterator external_node_it = external_map.find(key);
    if (external_node_it == external_map.end()) {
        return;
    }

    const std::map<Key, Edge<KeyType>>& local_to_ext_map = external_node_it->second;
    for (typename std::map<Key, Edge<KeyType>>::const_iterator local_nodes_it = local_to_ext_map.begin(); local_nodes_it != local_to_ext_map.end(); ++local_nodes_it) {
        Key local_key = local_nodes_it->first;
        typename std::map<Key, StorageNode>::iterator node = nodes.find(local_key);
        if (node != nodes.end()) {
            node->second.remove_edge_to(key);
        }
    }
    
    // TODO: finish function
};

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

/*
// Получить подграф узлов, имеющих соседей в указанном хранилище (копии)
std::map<KeyType, StorageNode> get_nodes_with_neighbors_in_storage_map_copy(int target_storage_id) const {
    std::map<KeyType, StorageNode> result;
    
    typename std::map<KeyType, StorageNode>::const_iterator it;
    for (it = nodes.begin(); it != nodes.end(); ++it) {
        const StorageNode& node = it->second;
        
        typename std::map<int, std::vector<typename StorageNode::Neighbour>>::const_iterator map_it;
        map_it = node.other_storages_neighbours.find(target_storage_id);
        
        if (map_it != node.other_storages_neighbours.end()) {
            if (!map_it->second.empty()) {
                result.insert(*it);
            }
        }
    }
    
    return result;
}

int get_total_edges_to_storage(int target_storage_id) const {
    int total = 0;
    
    typename std::map<KeyType, StorageNode>::const_iterator it;
    for (it = nodes.begin(); it != nodes.end(); ++it) {
        const StorageNode& node = it->second;
        
        typename std::map<int, std::vector<typename StorageNode::Neighbour>>::const_iterator map_it;
        map_it = node.other_storages_neighbours.find(target_storage_id);
        
        if (map_it != node.other_storages_neighbours.end()) {
            total += map_it->second.size();
        }
    }
    
    return total;
}

/*
//
// Подсчёты сумм весов
//

// Подсчет суммы весов всех внутренних ребер в хранилище
int get_internal_edges_weight_sum() const {
    // Потенциально будет использоваться с использованием только "граничных" вершин, поэтому нужно отдельно записывать дупликаты, так есть недублирующиеся, которые идут "вглубь" хранилища
    int total_weight = 0;
    int duplicate_weight = 0;
    
    typename std::map<KeyType, StorageNode>::const_iterator node_it;
    for (node_it = nodes.begin(); node_it != nodes.end(); ++node_it) {
        const StorageNode& node = node_it->second;
        
        typename std::vector<typename StorageNode::Neighbour>::const_iterator neighbour_it;
        for (neighbour_it = node.this_storage_neighbours.begin();
            neighbour_it != node.this_storage_neighbours.end();
            ++neighbour_it) {
            
            // пропускаем петли
            if (node.key.key_value == neighbour_it->first.key_value) {
                continue;
            }

            std::cout << "Edge from " << node.key.key_value << " to " << neighbour_it->first.key_value << std::endl;

            total_weight += neighbour_it->second.weight;
            if (has_node(neighbour_it->first.key_value)) {
                duplicate_weight += neighbour_it->second.weight;
            }
        }
    }
    
    return total_weight - duplicate_weight / 2;
}

// Подсчет суммы весов ребер из текущего хранилища в целевое хранилище
int get_external_edges_weight_sum_to_storage(int target_storage_id) const {
    int total_weight = 0;
    
    typename std::map<KeyType, StorageNode>::const_iterator node_it;
    for (node_it = nodes.begin(); node_it != nodes.end(); ++node_it) {
        const StorageNode& node = node_it->second;
        
        typename std::map<int, std::vector<typename StorageNode::Neighbour>>::const_iterator map_it;
        map_it = node.other_storages_neighbours.find(target_storage_id);
        
        if (map_it != node.other_storages_neighbours.end()) {
            const std::vector<typename StorageNode::Neighbour>& edges = map_it->second;
            
            typename std::vector<typename StorageNode::Neighbour>::const_iterator edge_it;
            for (edge_it = edges.begin(); edge_it != edges.end(); ++edge_it) {
                total_weight += edge_it->second.weight;
            }
        }
    }
    
    return total_weight;
}

//
// операции над вершинами
//

bool add_internal_edge(const KeyType& from_key, 
                        const KeyType& to_key, 
                        const Edge& edge) {
    StorageNode* from_node = get_node(from_key);
    StorageNode* to_node = get_node(to_key);
    
    if (from_node == nullptr || to_node == nullptr) {
        return false;
    }
    
    typename StorageNode::Neighbour neighbor(NodeKey<KeyType>(to_key), edge);
    from_node->this_storage_neighbours.push_back(neighbor);
    return true;
}

bool add_external_edge(const KeyType& from_key,
                        int target_storage_id,
                        const KeyType& to_key,
                        const Edge& edge) {
    StorageNode* from_node = get_node(from_key);
    
    if (from_node == nullptr) {
        return false;
    }
    
    typename StorageNode::Neighbour neighbor(NodeKey<KeyType>(to_key), edge);
    from_node->other_storages_neighbours[target_storage_id].push_back(neighbor);
    return true;
}

// Удалить внутреннее ребро
bool remove_internal_edge(const KeyType& from_key, const KeyType& to_key) {
    StorageNode* from_node = get_node(from_key);
    if (from_node == nullptr) {
        return false;
    }
    
    // Ищем ребро в this\_storage\_neighbours
    typename std::vector<typename StorageNode::Neighbour>::iterator it;
    for (it = from_node->this_storage_neighbours.begin(); 
            it != from_node->this_storage_neighbours.end(); 
            ++it) {
        if (it->first.key_value == to_key) {
            from_node->this_storage_neighbours.erase(it);
            return true;
        }
    }
    
    return false;
}

// Удалить внешнее ребро (конкретное ребро в конкретное хранилище)
bool remove_external_edge(const KeyType& from_key, 
                            int target_storage_id, 
                            const KeyType& to_key) {
    StorageNode* from_node = get_node(from_key);
    if (from_node == nullptr) {
        return false;
    }
    
    typename std::map<int, std::vector<typename StorageNode::Neighbour>>::iterator storage_it;
    storage_it = from_node->other_storages_neighbours.find(target_storage_id);
    if (storage_it == from_node->other_storages_neighbours.end()) {
        return false;
    }
    
    std::vector<typename StorageNode::Neighbour>& neighbors = storage_it->second;
    typename std::vector<typename StorageNode::Neighbour>::iterator it;
    for (it = neighbors.begin(); it != neighbors.end(); ++it) {
        if (it->first.key_value == to_key) {
            neighbors.erase(it);
            
            // Если после удаления вектор стал пустым, удаляем запись из map
            if (neighbors.empty()) {
                from_node->other_storages_neighbours.erase(storage_it);
            }
            
            return true;
        }
    }
    
    return false;
}
    
// Проверить, есть ли узел с рёбрами в указанное хранилище
bool has_node_with_edges_to_storage(const KeyType& node_key, int target_storage_id) const {
    const StorageNode* node = get_node(node_key);
    if (node == nullptr) {
        return false;
    }
    
    typename std::map<int, std::vector<typename StorageNode::Neighbour>>::const_iterator it;
    it = node->other_storages_neighbours.find(target_storage_id);
    
    if (it != node->other_storages_neighbours.end()) {
        return !it->second.empty();
    }
    
    return false;
}

// Получить вес всех рёбер в указанное хранилище
int get_total_weight_to_storage(int target_storage_id) const {
    int total_weight = 0;
    
    typename std::map<KeyType, StorageNode>::const_iterator it;
    for (it = nodes.begin(); it != nodes.end(); ++it) {
        const StorageNode& node = it->second;
        
        typename std::map<int, std::vector<typename StorageNode::Neighbour>>::const_iterator map_it;
        map_it = node.other_storages_neighbours.find(target_storage_id);
        
        if (map_it != node.other_storages_neighbours.end()) {
            const std::vector<typename StorageNode::Neighbour>& edges = map_it->second;
            for (size_t i = 0; i < edges.size(); ++i) {
                total_weight += edges[i].second.weight;
            }
        }
    }
    
    return total_weight;
}
    */

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
    
    os << ")";
    return os;
}
};

#endif // VKR\_COURSE\_STORAGE