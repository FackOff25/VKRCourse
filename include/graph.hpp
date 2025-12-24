#ifndef VKR_COURSE_GRAPH
#define VKR_COURSE_GRAPH

#include <stdlib.h>
#include <map>
#include <string>
#include <vector>

template <typename KeyType> 
class NodeKey {
    public:
        const KeyType key_value;
        NodeKey(const KeyType& key): key_value(key) {};

        bool operator<(const KeyType& other){
            return key_value < other;
        };
};


template <typename data_type> 
class Parameter{
    private:
        data_type data;
    public:
        void set_data(const data_type& _data) {
            data = _data;
        }
        data_type get_data() {
            return data;
        };
};

template <typename KeyType>
class Node {
    public:
        Node(KeyType _key): key(NodeKey<KeyType>(_key)) {};
        Node(NodeKey<KeyType> _key): key(_key) {};
        Node(NodeKey<KeyType> _key, std::vector<NodeKey<KeyType>> _this_storage_neighbours): key(_key), this_storage_neighbours(_this_storage_neighbours) {};
        Node(NodeKey<KeyType> _key, std::vector<NodeKey<KeyType>> _this_storage_neighbours, std::map<int,std::vector<NodeKey<KeyType>>> _other_storages_neighbours): key(_key), this_storage_neighbours(_this_storage_neighbours), other_storages_neighbours(_other_storages_neighbours) {};

        const NodeKey<KeyType> key;

        std::map<std::string, Parameter<char*>> parameters;

        std::vector<NodeKey<KeyType>> this_storage_neighbours;

        std::map<int,std::vector<NodeKey<KeyType>>> other_storages_neighbours;
};

#endif