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
        if (path.empty() || edge_index >= path.size() - 1) {
            return current_weight;
        }

        const size_t path_length = path.size() - 1;
        if (path_length == 0) {
            return current_weight;
        }

        float position_factor = static_cast<float>(edge_index) / path_length; // 0.0 .. ~1.0
        float adjustment = 1.0f + 0.5f * (1.0f - position_factor);
        return current_weight * adjustment;
    }
};

#endif // VKR\_PATHFINDERS