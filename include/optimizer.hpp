#ifndef VKR_COURSE_OPTIMIZER
#define VKR_COURSE_OPTIMIZER

#include "storage.hpp"
#include <unordered_set>

template <typename KeyType>
class StorageOptimizer {
private:
    const Storage<KeyType>& storage1;
    const Storage<KeyType>& storage2;

    int calculate_gv(const Node<KeyType>& node, int other_storage_id) const {
        int internal_edges_weight = node.get_internal_edges_weight_sum();
        int external_edges_weight = node.get_external_edges_weight_sum_to_storage(other_storage_id);
        
        return internal_edges_weight - external_edges_weight;
    }
    
public:
    StorageOptimizer(const Storage<KeyType>& s1, const Storage<KeyType>& s2)
        : storage1(s1), storage2(s2) {}

    void calculate_gvs() const {
        // Получаем граничные вершины для обоих хранилищ
        std::unordered_map<KeyType, Node<KeyType>> boundary_nodes1 = storage1.get_nodes_with_neighbors_in_storage_map_copy(storage2.get_id());
        std::unordered_map<KeyType, Node<KeyType>> boundary_nodes2 = storage2.get_nodes_with_neighbors_in_storage_map_copy(storage1.get_id());

        typename std::unordered_map<KeyType, Node<KeyType>>::const_iterator it;
        for (it = boundary_nodes1.begin(); it != boundary_nodes1.end(); ++it) {
            const Node<KeyType>& node = it->second;
            
            std::cout << "Metric for node " << node.key.key_value << " is " << calculate_gv(node, storage2.get_id()) << std::endl;
        }

        for (it = boundary_nodes2.begin(); it != boundary_nodes2.end(); ++it) {
            const Node<KeyType>& node = it->second;
            
            std::cout << "Metric for node " << node.key.key_value << " is " << calculate_gv(node, storage1.get_id()) << std::endl;
        }
    }
};

#endif // VKR_COURSE_OPTIMIZER