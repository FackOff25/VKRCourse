#ifndef VKR_PATHFINDERS
#define VKR_PATHFINDERS

#include "graph.hpp"
#include "pathfinder.hpp"
#include <vector>

template <typename KeyType>
class IWeightAdjuster {
public:
    virtual float compute_new_weight(
        const std::vector<NodeKey<KeyType>>& path,
        size_t edge_index,
        float current_weight
    ) const = 0;
};

template <typename KeyType>
class SchismAdjuster : public IWeightAdjuster<KeyType> {
public:
    SchismAdjuster() {};

    float compute_new_weight(
        const std::vector<NodeKey<KeyType>>& path,
        size_t edge_index,
        float current_weight
    ) const override {
        return current_weight + 1.0f;
    }
};

template <typename KeyType> 
class WAWAdjuster : public IWeightAdjuster<KeyType> {
public:
    WAWAdjuster() {};

    float compute_new_weight(
        const std::vector<NodeKey<KeyType>>& path,
        size_t edge_index,
        float current_weight
    ) const override {
        return current_weight;
    };
};

#endif // VKR\_PATHFINDERS