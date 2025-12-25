#ifndef VKR_COURSE_STORAGE
#define VKR_COURSE_STORAGE

#include "graph.hpp"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>

template <typename KeyType>
class Storage {
private:
    typedef Node<KeyType> StorageNode;
    int storage_id;
    std::unordered_map<KeyType, StorageNode> nodes;

public:
    Storage(int id) : storage_id(id) {}
    Storage(int id, std::unordered_map<KeyType, StorageNode> _nodes) : storage_id(id), nodes(_nodes) {}
    
    int get_id() const {
        return storage_id;
    }

    // добавляет узел и создает двусторонние ребра с указанными соседями
    bool add_node(const StorageNode& node) {
        KeyType key = node.key.key_value;
        typename std::unordered_map<KeyType, StorageNode>::iterator it = nodes.find(key);
        if (it != nodes.end()) {
            return false;
        }
        nodes[key] = node;
        
        
        // Проходим по всем указанным соседям
        for (size_t i = 0; i < node.this_storage_neighbours.size(); ++i) {
            KeyType neighbor_key = node.this_storage_neighbours[i].first.key_value;
            Edge edge = node.this_storage_neighbours[i].second;
            
            // Пропускаем петли (ребра к самому себе)
            if (neighbor_key == key) {
                continue;
            }
            
            typename StorageNode::Neighbour neighbor_to(NodeKey<KeyType>(neighbor_key), edge);
            
            if (has_node(neighbor_key)) {
                StorageNode* neighbor_node = get_node(neighbor_key);
                
                for (size_t j = 0; j < neighbor_node->this_storage_neighbours.size(); ++j) {
                    if (neighbor_node->this_storage_neighbours[j].first.key_value == key) {
                        neighbor_node->this_storage_neighbours.push_back(neighbor_to);
                        break;
                    }
                }                
            }
        }
        return true;
    }

    // Самый простой remove_node: удаляет узел и все связанные с ним ребра
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
    
    size_t count_nodes_with_neighbors_in_storage(int target_storage_id) const {
        size_t count = 0;
        
        typename std::unordered_map<KeyType, StorageNode>::const_iterator it;
        for (it = nodes.begin(); it != nodes.end(); ++it) {
            const StorageNode& node = it->second;
            
            typename std::map<int, std::vector<typename StorageNode::Neighbour>>::const_iterator map_it;
            map_it = node.other_storages_neighbours.find(target_storage_id);
            
            if (map_it != node.other_storages_neighbours.end()) {
                if (!map_it->second.empty()) {
                    ++count;
                }
            }
        }
        
        return count;
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
    

    int get_edges_to_storage(int target_storage_id) const {
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
        
        // Ищем ребро в this_storage_neighbours
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

};

#endif // VKR_COURSE_STORAGE