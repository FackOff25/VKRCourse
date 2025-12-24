#ifndef VKR_COURSE_GRAPH
#define VKR_COURSE_GRAPH

#include <stdlib.h>
#include <map>
#include <string>
#include <vector>

class INodeKey{
    virtual bool operator<(const INodeKey& other);
};

template <typename data_type> 
class Parameter{
    private:
        data_type data;
    public:
        void set_data(const data_type& _data) {
            data = _data;
        }
    virtual data_type  get_data();
};

template <typename NodeKey>
class INode {
public:
    INode(NodeKey _key): key(_key);
    INode(NodeKey _key, std::vector<NodeKey> _this_storage_neighbours): key(_key), this_storage_neighbours(_this_storage_neighbours);
    INode(NodeKey _key, std::vector<NodeKey> _this_storage_neighbours, std::map<int,std::vector<NodeKey>> _other_storages_neighbours): key(_key), this_storage_neighbours(_this_storage_neighbours), other_storages_neighbours(_other_storages_neighbours);

    static_assert(std::is_base_of<INodeKey, NodeKey>::value, "NodeKey must be a type derived from INodeKey");
    
    const NodeKey key;

    std::map<std::string, Parameter<char[]>> parameters;

    std::vector<NodeKey> this_storage_neighbours;

    std::map<int,std::vector<NodeKey>> other_storages_neighbours;
};

#endif