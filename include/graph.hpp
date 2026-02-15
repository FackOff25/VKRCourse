#ifndef VKR_GRAPH
#define VKR_GRAPH

#include <stdlib.h>
#include <map>
#include <string>
#include <vector>
#include <numeric>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <sstream>

template <typename KeyType> 
class NodeKey {
public:
KeyType key_value;

NodeKey(): key_value(KeyType()) {};
NodeKey(const KeyType& key): key_value(key) {};

// Оператор присваивания
NodeKey<KeyType>& operator=(const NodeKey<KeyType>& other) {
    if (this != &other) {
        key_value = other.key_value;
    }
    return *this;
};

// Дружественные операторы сравнения
friend bool operator<(const NodeKey<KeyType>& lhs, const NodeKey<KeyType>& rhs) {
    return lhs.key_value < rhs.key_value;
}

friend bool operator>(const NodeKey<KeyType>& lhs, const NodeKey<KeyType>& rhs) {
    return rhs < lhs;
}

friend bool operator==(const NodeKey<KeyType>& lhs, const NodeKey<KeyType>& rhs) {
    return lhs.key_value == rhs.key_value;
}

friend bool operator!=(const NodeKey<KeyType>& lhs, const NodeKey<KeyType>& rhs) {
    return !(lhs == rhs);
}
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

    Edge& get_reverted() {
        return Edge<KeyType>(from, to, weight, directional, parameters);
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

    bool operator<(const Edge& other) const {
        if (from != other.from) return from < other.from;
        if (to != other.to) return to < other.to;
        if (weight != other.weight) return weight < other.weight;
        return directional < other.directional;
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
    
    NodeKey<KeyType> get_other(const NodeKey<KeyType>& node) const {
        if (node == to) {
            return from;
        } else if (node == from) {
            return to;
        }

        return node;
    }

    NodeKey<KeyType> get_other(NodeKey<KeyType>* node) const {
        if (*node == to) {
            return from;
        } else if (*node == from) {
            return to;
        } else {
            return *node;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const Edge<KeyType>& edge) {
        os << "Edge(from: " << edge.get_from().key_value 
        << ", to: " << edge.get_to().key_value 
        << ", weight: " << edge.get_weight();
        
        if (edge.is_directional()) {
            os << ", directional";
        }
        
        // Добавляем информацию о параметрах, если они есть
        const std::map<std::string, Parameter>& params = edge.get_parameters();
        if (!params.empty()) {
            os << ", parameters: {";
            typename std::map<std::string, Parameter>::const_iterator it = params.begin();
            if (it != params.end()) {
                os << it->first;
                ++it;
            }
            for (; it != params.end(); ++it) {
                os << ", " << it->first;
            }
            os << "}";
        }
        
        os << ")";
        return os;
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

    NodeKey<KeyType> get_key() const {
        return key;
    }

    void add_edge(Edge<KeyType> new_edge) {
        NodeKey<KeyType> current_key = this->get_key();
        edges[new_edge.get_other(current_key)] = new_edge;
    }

    void add_edges(std::map<NodeKey<KeyType>, Edge<KeyType>> new_edges) {
        edges.merge(new_edges);
    }

    Node& operator=(const Node& other) {
        if (this != &other) {
            key = other.key;
            parameters = other.parameters;
            edges = other.edges;
        }
        return *this;
    }

    bool operator<(const Node<KeyType>& other) const {
        return key < other.key;
    }

    friend std::ostream& operator<<(std::ostream& os, const Node<KeyType>& node) {
        os << "Node(key: " << node.get_key().key_value;
        
        // Выводим параметры узла
        if (!node.parameters.empty()) {
            os << ", parameters: {";
            typename std::map<std::string, Parameter>::const_iterator param_it = node.parameters.begin();
            if (param_it != node.parameters.end()) {
                os << param_it->first;
                ++param_it;
            }
            for (; param_it != node.parameters.end(); ++param_it) {
                os << ", " << param_it->first;
            }
            os << "}";
        }
        
        // Выводим ребра узла
        if (!node.edges.empty()) {
            os << ", edges: [";
            typename std::map<NodeKey<KeyType>, Edge<KeyType>>::const_iterator edge_it = node.edges.begin();
            if (edge_it != node.edges.end()) {
                os << edge_it->second;
                ++edge_it;
            }
            for (; edge_it != node.edges.end(); ++edge_it) {
                os << ", " << edge_it->second;
            }
            os << "]";
        }
        
        os << ")";
        return os;
    }
};

#endif // VKR\_GRAPH