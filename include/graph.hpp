#ifndef VKR_COURSE_GRAPH
#define VKR_COURSE_GRAPH

#include <stdlib.h>
#include <map>
#include <string>
#include <vector>
#include <numeric>
#include <utility>
#include <stdexcept>

template <typename KeyType> 
class NodeKey {
    public:
        KeyType key_value;
        NodeKey(): key_value(KeyType()) {};
        NodeKey(const KeyType& key): key_value(key) {};

        bool operator<(const KeyType& other){
            return key_value < other;
        };

        bool operator>(const KeyType& other){
            return key_value > other;
        };

        bool operator==(const KeyType& other){
            return key_value == other;
        };

        NodeKey<KeyType>& operator=(const NodeKey<KeyType>& other) {
            key_value = other.key_value;
            return *this;
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
        int weight;
        std::map<std::string, Parameter> parameters;

        Edge& operator=(const Edge& other){
            weight = other.weight;
            parameters = other.parameters;
            return *this;
        };
};

template <typename KeyType>
class Node {
    public:
        typedef std::pair<NodeKey<KeyType>,Edge> Neighbour;

        Node(): key(NodeKey<KeyType>()) {};
        Node(KeyType _key): key(NodeKey<KeyType>(_key)) {};
        Node(NodeKey<KeyType> _key): key(_key) {};
        Node(NodeKey<KeyType> _key, std::vector<Neighbour> _this_storage_neighbours): key(_key), this_storage_neighbours(_this_storage_neighbours) {};
        Node(NodeKey<KeyType> _key, std::vector<Neighbour> _this_storage_neighbours, std::map<int,std::vector<Neighbour>> _other_storages_neighbours)
            : key(_key), 
            this_storage_neighbours(_this_storage_neighbours), 
            other_storages_neighbours(_other_storages_neighbours) {};
        // Конструктор копирования
        Node(const Node& other) 
            : key(other.key),
            parameters(other.parameters),
            this_storage_neighbours(other.this_storage_neighbours),
            other_storages_neighbours(other.other_storages_neighbours) {}

        NodeKey<KeyType> key;
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

        friend void swap(Node& first, Node& second) noexcept {
            using std::swap;
            
            swap(first.parameters, second.parameters);
            swap(first.this_storage_neighbours, second.this_storage_neighbours);
            swap(first.other_storages_neighbours, second.other_storages_neighbours);
        }

        Node& operator=(const Node& other) {
            if (this != &other) {
                key = other.key;
                parameters = other.parameters;
                this_storage_neighbours = other.this_storage_neighbours;
                other_storages_neighbours = other.other_storages_neighbours;
            }
            return *this;
        }
};

#endif // VKR_COURSE_GRAPH