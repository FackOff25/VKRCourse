#ifndef VKR_COURSE_STORAGE
#define VKR_COURSE_STORAGE

#include "graph.hpp"
#include "interface_bus.hpp"
#include <unordered_map>
#include <unordered_set>
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
    std::unordered_map<Key, StorageNode> nodes;
    std::map<int, std::map<KeyType, Edge<KeyType>>> external_edges;
    IBus<KeyType>* bus = nullptr;

public:
    Storage(int id) 
        :storage_id(id) {}
    Storage(int id, std::unordered_map<KeyType, StorageNode> _nodes) 
        :storage_id(id), nodes(_nodes) {}
    
    void connectToBus(IBus<KeyType>* _bus) { bus = _bus; };

    int get_id() const {
        return storage_id;
    }

    std::unordered_set<Edge<KeyType>> add_node(const StorageNode& node) {
        Key key = node.get_key();
        typename std::unordered_map<KeyType, StorageNode>::iterator it = nodes.find(key);
        if (it != nodes.end()) {
            return false;
        }
        nodes[key] = node;

        std::unordered_set<Edge<KeyType>> external_edges_to_announce;
        for (typename std::map<StorageNode, Edge<KeyType>>::iterator it = node.edges.begin(); it != nodes.end(); ++it) {
            typename std::unordered_map<KeyType, StorageNode>::iterator it2 = nodes.find(it->first.get_key());
            if (it2 != nodes.end()) {
                it2->second.add_edge(it->second);
            } else {
                int neighbours_storage_id = bus->ask_who_has(it->first.get_key());
                if (neighbours_storage_id != -1) {
                    external_edges[neighbours_storage_id][it->first.get_key()] = it->second;
                    external_edges_to_announce.insert(it->second);
                }
            }
        }
        return external_edges_to_announce;
    }

    bool add_node_and_announce(const StorageNode& node) {
        if (bus == nullptr) {
            return false;
        }
        std::unordered_set<Edge<KeyType>> external_edges = add_node(node);
        if (external_edges != nullptr) {
            return false;
        }
        bus.announce_add(node.get_key(), storage_id, external_edges);
    }

    /*
    // Самый простой remove\_node: удаляет узел и все связанные с ним ребра
    bool remove_node(const KeyType& key) {
        if (!has_node(key)) {
            return false;
        }
        
        // Получаем узел для удаления
        Node<KeyType>* node_to_remove = get_node(key);
        if (node_to_remove == nullptr) {
            return false;
        }
        
        // Проходим по всем соседям удаляемого узла
        for (size_t i = 0; i < node_to_remove->this_storage_neighbours.size(); ++i) {
            KeyType neighbor_key = node_to_remove->this_storage_neighbours[i].first.key_value;
            
            // Ищем и удаляем обратное ребро у соседа
            Node<KeyType>* neighbor = get_node(neighbor_key);
            if (neighbor != nullptr) {
                for (size_t j = 0; j < neighbor->this_storage_neighbours.size(); ++j) {
                    if (neighbor->this_storage_neighbours[j].first.key_value == key) {
                        neighbor->this_storage_neighbours.erase(neighbor->this_storage_neighbours.begin() + j);
                        break;
                    }
                }
            }
        }
        
        // Удаляем сам узел из хранилища
        nodes.erase(key);
        return true;
    }
    
    StorageNode* get_node(const KeyType& key) {
        typename std::unordered_map<KeyType, StorageNode>::iterator it = nodes.find(key);
        if (it != nodes.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    bool has_node(const KeyType& key) const {
        typename std::unordered_map<KeyType, StorageNode>::const_iterator it = nodes.find(key);
        return it != nodes.end();
    }
    
    const std::unordered_map<KeyType, StorageNode>& get_all_nodes() const {
        return nodes;
    }
    
    size_t size() const {
        return nodes.size();
    }
    
    void clear() {
        nodes.clear();
    }

    //
    // Подсчёты сумм весов
    //

    // Подсчет суммы весов всех внутренних ребер в хранилище
    int get_internal_edges_weight_sum() const {
        // Потенциально будет использоваться с использованием только "граничных" вершин, поэтому нужно отдельно записывать дупликаты, так есть недублирующиеся, которые идут "вглубь" хранилища
        int total_weight = 0;
        int duplicate_weight = 0;
        
        typename std::unordered_map<KeyType, StorageNode>::const_iterator node_it;
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
        
        typename std::unordered_map<KeyType, StorageNode>::const_iterator node_it;
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
    // Получение наборов связанных с другим хранилищем
    //
    
    // Получить подграф узлов, имеющих соседей в указанном хранилище (копии)
    std::vector<StorageNode> get_nodes_with_neighbors_in_storage_copy(int target_storage_id) const {
        std::vector<StorageNode> result;
        
        typename std::unordered_map<KeyType, StorageNode>::const_iterator it;
        for (it = nodes.begin(); it != nodes.end(); ++it) {
            const StorageNode& node = it->second;
            
            typename std::map<int, std::vector<typename StorageNode::Neighbour>>::const_iterator map_it;
            map_it = node.other_storages_neighbours.find(target_storage_id);
            
            if (map_it != node.other_storages_neighbours.end()) {
                if (!map_it->second.empty()) {
                    result.push_back(node);
                }
            }
        }
        
        return result;
    }

    // Получить подграф узлов, имеющих соседей в указанном хранилище (копии)
    std::unordered_map<KeyType, StorageNode> get_nodes_with_neighbors_in_storage_map_copy(int target_storage_id) const {
        std::unordered_map<KeyType, StorageNode> result;
        
        typename std::unordered_map<KeyType, StorageNode>::const_iterator it;
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
    
    std::vector<typename StorageNode::Neighbour> get_all_edges_to_storage(int target_storage_id) const {
        std::vector<typename StorageNode::Neighbour> all_edges;
        
        typename std::unordered_map<KeyType, StorageNode>::const_iterator it;
        for (it = nodes.begin(); it != nodes.end(); ++it) {
            const StorageNode& node = it->second;
            
            typename std::map<int, std::vector<typename StorageNode::Neighbour>>::const_iterator map_it;
            map_it = node.other_storages_neighbours.find(target_storage_id);
            
            if (map_it != node.other_storages_neighbours.end()) {
                const std::vector<typename StorageNode::Neighbour>& neighbors = map_it->second;
                all_edges.insert(all_edges.end(), neighbors.begin(), neighbors.end());
            }
        }
        
        return all_edges;
    }
    

    int get_total_edges_to_storage(int target_storage_id) const {
        int total = 0;
        
        typename std::unordered_map<KeyType, StorageNode>::const_iterator it;
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
    
    typename std::unordered_map<KeyType, StorageNode>::iterator begin() {
        return nodes.begin();
    }
    
    typename std::unordered_map<KeyType, StorageNode>::iterator end() {
        return nodes.end();
    }
    
    typename std::unordered_map<KeyType, StorageNode>::const_iterator begin() const {
        return nodes.begin();
    }
    
    typename std::unordered_map<KeyType, StorageNode>::const_iterator end() const {
        return nodes.end();
    }
    
    typename std::unordered_map<KeyType, StorageNode>::const_iterator cbegin() const {
        return nodes.cbegin();
    }
    
    typename std::unordered_map<KeyType, StorageNode>::const_iterator cend() const {
        return nodes.cend();
    }
    
    bool empty() const {
        return nodes.empty();
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
        
        typename std::unordered_map<KeyType, StorageNode>::const_iterator it;
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
};

#endif // VKR\_COURSE\_STORAGE