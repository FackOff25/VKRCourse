#ifndef VKR_LPR
#define VKR_LPR

#include "graph.hpp"

template<typename KeyType>
struct LocalPathResult {
    std::vector<NodeKey<KeyType>> path;
    bool success = false;
    NodeKey<KeyType> exit_node;
};

#endif