#ifndef VKR_COURSE_OPTIMIZER
#define VKR_COURSE_OPTIMIZER

#include "storage.hpp"
#include <unordered_set>

template <typename KeyType>
class StorageOptimizer {
private:
    const Storage<KeyType>& storage1;
    const Storage<KeyType>& storage2;
    
public:
    StorageOptimizer(const Storage<KeyType>& s1, const Storage<KeyType>& s2)
        : storage1(s1), storage2(s2) {}
    
    // Метод для расчета метрики g_v
    int calculate_gv() const {
        int internal_edges_weight = 0;
        int external_edges_weight = 0;

        // Получаем граничные вершины для обоих хранилищ
        std::vector<Node<KeyType>> boundary_nodes1 = storage1.get_nodes_with_neighbors_in_storage_copy(storage2.get_id());
        std::vector<Node<KeyType>> boundary_nodes2 = storage2.get_nodes_with_neighbors_in_storage_copy(storage1.get_id());

        
        
        internal_edges_weight += storage1.get_internal_edges_weight_sum();
        internal_edges_weight += storage2.get_internal_edges_weight_sum();
        
        // Рассчитываем сумму весов внешних ребер между хранилищами
        external_edges_weight += storage1.get_external_edges_weight_sum_to_storage(storage2.get_id());
        //external_edges_weight += calculate_external_edges_weight(storage2, storage1.get_id());
        
        return internal_edges_weight - external_edges_weight;
    }
};

#endif // VKR_COURSE_OPTIMIZER