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

template <typename KeyType>
class Edge {
private:
    int weight;
    bool directional;
    std::map<std::string, Parameter> parameters;
    NodeKey<KeyType> from;
    NodeKey<KeyType> to;
public:
    Edge() : weight(1), directional(false) {}
    Edge(const NodeKey<KeyType>& from_node, const NodeKey<KeyType>& to_node) 
        : weight(1), directional(false), from(from_node), to(to_node) {}
    
    Edge(const NodeKey<KeyType>& from_node, const NodeKey<KeyType>& to_node, int edge_weight) 
        : weight(edge_weight), directional(false), from(from_node), to(to_node) {}
    
    Edge(const NodeKey<KeyType>& from_node, const NodeKey<KeyType>& to_node, 
        int edge_weight, bool is_directional) 
        : weight(edge_weight), directional(is_directional), from(from_node), to(to_node) {}
    
    Edge(const NodeKey<KeyType>& from_node, const NodeKey<KeyType>& to_node,
        int edge_weight, bool is_directional, const std::map<std::string, Parameter>& params)
        : weight(edge_weight), directional(is_directional), parameters(params), 
        from(from_node), to(to_node) {}
    
    Edge(const Edge& other) 
        : weight(other.weight), directional(other.directional), 
        parameters(other.parameters), from(other.from), to(other.to) {}

    int get_weight() const {
        return weight;
    }
    bool is_directional() const {
        return directional;
    }
    const std::map<std::string, Parameter>& get_parameters() const {
        return parameters;
    }
    const NodeKey<KeyType>& get_from() const {
        return from;
    }
    const NodeKey<KeyType>& get_to() const {
        return to;
    }
    
    Edge& operator=(const Edge& other) {
        if (this != &other) {
            weight = other.weight;
            directional = other.directional;
            parameters = other.parameters;
            from = other.from;
            to = other.to;
        }
        return *this;
    }

    // doesn't check extra parameters
    bool operator==(const Edge& other) {
        if (other.directional != directional) return false;

        if (directional) {
            if (other.to != to || other.from != from) return false;
        } else {
            if ((other.to != to && other.from != from) ||
                (other.from != to && other.to != from)) return false;
        }

        if (other.weight != weight) return false;

        return true;
    }
    
    NodeKey<KeyType> get_other(const NodeKey<KeyType>& node) {
        if (node == to) {
            return from;
        } else if (node == from) {
            return to;
        }

        return node;
    }

    NodeKey<KeyType> get_other(NodeKey<KeyType>* node) {
        if (*node == to) {
            return from;
        } else if (*node == from) {
            return to;
        } else {
            return *node;
        }
    }
};

template <typename KeyType>
class Node {
private:
    NodeKey<KeyType> key;
public:
    std::map<std::string, Parameter> parameters;
    std::map<NodeKey<KeyType>, Edge<KeyType>> edges;

    Node(): 
        key(NodeKey<KeyType>()) {};

    Node(KeyType _key): 
        key(NodeKey<KeyType>(_key)) {};

    Node(NodeKey<KeyType> _key): 
        key(_key) {};

    Node(NodeKey<KeyType> _key, std::map<NodeKey<KeyType>, Edge<KeyType>> _edges): 
        key(_key), edges(_edges) {};

    Node(const Node& other) 
        : key(other.key),
        parameters(other.parameters),
        edges(other.edges) {};

    void add_edge(Edge<KeyType> new_edge) {
        edges[new_edge.get_other(*this)] = new_edge;
    }

    void add_edges(std::map<NodeKey<KeyType>, Edge<KeyType>> new_edges) {
        edges.merge(new_edges);
    }

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
            edges = other.edges;
        }
        return *this;
    }
};

#endif // VKR\_COURSE\_GRAPH