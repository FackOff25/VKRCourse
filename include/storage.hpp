#ifndef VKR_COURSE_STORAGE
#define VKR_COURSE_STORAGE

#include "graph.hpp"
#include "i_bus.hpp"
#include "base_storage.hpp"
#include "weight_adjuster.hpp"
#include <set>
#include <optional>
#include <vector>
#include <algorithm>
#include <iostream>
#include <map>

template <typename KeyType>
class SimpleStorage : virtual public BaseStorage<KeyType> {
typedef Node<KeyType> StorageNode;
typedef NodeKey<KeyType> Key;
public:
SimpleStorage(int id, std::unique_ptr<IWeightAdjuster<KeyType>> _weight_adjuster, std::map<Key, StorageNode> _nodes = std::map<Key, StorageNode>()) 
    : BaseStorage<KeyType>(id, std::move(_weight_adjuster), _nodes) {}

float get_streaming_euristics_change(const Node<KeyType>& node, long total_edges) override {
    return 0;
}
};

template <typename KeyType> 
class SimpleAStarStorage : public SimpleStorage<KeyType>, public AStarStorage<KeyType> {
public:
    SimpleAStarStorage(int id, 
                       std::unique_ptr<IWeightAdjuster<KeyType>> _weight_adjuster, 
                       std::map<NodeKey<KeyType>, Node<KeyType>> _nodes = {})
        : BaseStorage<KeyType>(id, std::move(_weight_adjuster), _nodes)
        , SimpleStorage<KeyType>(id, nullptr, _nodes) 
        , AStarStorage<KeyType>(id, nullptr, _nodes)
    {}
};
#endif // VKR\_COURSE\_STORAGE