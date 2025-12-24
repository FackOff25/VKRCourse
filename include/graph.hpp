#ifndef VKR_COURSE_GRAPH
#define VKR_COURSE_GRAPH

#include <stdlib.h>
#include <map>
#include <string>
#include <vector>
#include <numeric>
#include <utility>

template <typename KeyType> 
class NodeKey {
    public:
        const KeyType key_value;
        NodeKey(const KeyType& key): key_value(key) {};

        bool operator<(const KeyType& other){
            return key_value < other;
        };
};


class Parameter{
    private:
        std::vector<std::byte> data;
    public:
        void set_data(const std::vector<std::byte>& _data) {
            data = _data;
        }
        std::vector<std::byte> get_data() {
            return data;
        };
};

class Edge {
    public:
        const int weight;
        std::map<std::string, Parameter> parameters;
};

template <typename KeyType>
class Node {
    public:
        typedef std::pair<NodeKey<KeyType>,Edge> Neighbour;

        Node(KeyType _key): key(NodeKey<KeyType>(_key)) {};
        Node(NodeKey<KeyType> _key): key(_key) {};
        Node(NodeKey<KeyType> _key, std::vector<Neighbour> _this_storage_neighbours): key(_key), this_storage_neighbours(_this_storage_neighbours) {};
        Node(NodeKey<KeyType> _key, std::vector<Neighbour> _this_storage_neighbours, std::map<int,std::vector<Neighbour>> _other_storages_neighbours): key(_key), this_storage_neighbours(_this_storage_neighbours), other_storages_neighbours(_other_storages_neighbours) {};

        const NodeKey<KeyType> key;
        std::map<std::string, Parameter> parameters;

        std::vector<Neighbour> this_storage_neighbours;
        std::map<int,std::vector<Neighbour>> other_storages_neighbours;

        void set_this_storage_neighbours(std::vector<Neighbour> neighbours) const {this_storage_neighbours = neighbours;};
        void set_other_storages_neighbours(std::map<int,std::vector<Neighbour>> neighbours) const {other_storages_neighbours = neighbours;};

        int get_edge_num_to_others() const {
            return std::accumulate(
                other_storages_neighbours.begin(),
                other_storages_neighbours.end(),
                0,
                [](int sum, const auto& pair) {
                    return sum + pair.second.size();
                }
            );
        };
};

#endif