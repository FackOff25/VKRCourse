#ifndef VKR_PATHFINDER
#define VKR_PATHFINDER

#include "graph.hpp"
#include <vector>

template <typename KeyType>
using Path = std::vector<Edge<KeyType>>;

template <typename KeyType> 
class IPathfinder {
public:
    virtual Path<KeyType> find_path(const NodeKey<KeyType>& from, const NodeKey<KeyType>& to) = 0;
};

template <typename KeyType> 
class AStarPathfinder : public IPathfinder<KeyType> {
public:
    AStarPathfinder();
    Path<KeyType> find_path(const NodeKey<KeyType>& from, const NodeKey<KeyType>& to) override {
        return Path<KeyType>();
    };
};

#endif // VKR\_IPATHFINDER