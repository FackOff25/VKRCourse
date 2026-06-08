#ifndef VKR_PATHFINDERS
#define VKR_PATHFINDERS

#include "graph.hpp"
#include "pathfinder.hpp"
#include <vector>

template <typename KeyType> 
class IWeightAdjuster {
public:
    virtual void put_weight(Path<KeyType>) = 0;
};

template <typename KeyType> 
class SchismAdjuster : public IWeightAdjuster<KeyType> {
public:
    SchismAdjuster() {};

    void put_weight(Path<KeyType>) override {
        return;
    };
};

template <typename KeyType> 
class WAWAdjuster : public IWeightAdjuster<KeyType> {
public:
    WAWAdjuster() {};

    void put_weight(Path<KeyType>) override {
        return;
    };
};

#endif // VKR\_PATHFINDERS