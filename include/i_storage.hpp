#ifndef VKR_ISTORAGE
#define VKR_ISTORAGE

#include <optional>

template <typename KeyType>
class IStorage {
public:
virtual int get_id() const = 0;
virtual void connect_to_bus(IBus<KeyType>* _bus) = 0;

virtual void get_add_announcement(NodeKey<KeyType> key, int announcer_id, std::set<Edge<KeyType>> edges) = 0 ;
virtual void get_remove_announcement(NodeKey<KeyType> key, int announcer_id) = 0;

virtual std::optional<std::set<Edge<KeyType>>> add_node(const Node<KeyType>& node) = 0;
virtual std::optional<std::set<Edge<KeyType>>> add_node_and_announce(const Node<KeyType>& node) = 0;

virtual bool remove_node(const NodeKey<KeyType>& key) = 0;
virtual bool remove_node_and_announce(const NodeKey<KeyType>& key) = 0;
   
virtual Node<KeyType>* get_node(const NodeKey<KeyType>& key) = 0;
virtual bool has_node(const NodeKey<KeyType>& key) const = 0;
virtual const std::map<NodeKey<KeyType>, Node<KeyType>>& get_all_nodes() const = 0;

virtual size_t size() const = 0;
virtual size_t internal_edges_size() const = 0;
virtual void clear() = 0;
virtual bool empty() const = 0;

//
// Потоковое распределение
//
virtual float get_streaming_euristics_change(const Node<KeyType>& node, long total_edges)  = 0;

//
// Получение наборов связанных с другим хранилищем
//

// Получить набор узлов, имеющих соседей в указанном хранилище (копии)
virtual std::set<Node<KeyType>> get_nodes_with_neighbors_in_storage(int target_storage_id) const = 0;
virtual std::set<Edge<KeyType>> get_all_edges_to_storage(int target_storage_id) const = 0;

virtual typename std::map<NodeKey<KeyType>, Node<KeyType>>::iterator begin() = 0;
virtual typename std::map<NodeKey<KeyType>, Node<KeyType>>::iterator end() = 0;
virtual typename std::map<NodeKey<KeyType>, Node<KeyType>>::const_iterator begin() const = 0;
virtual typename std::map<NodeKey<KeyType>, Node<KeyType>>::const_iterator end() const = 0;
virtual typename std::map<NodeKey<KeyType>, Node<KeyType>>::const_iterator cbegin() const = 0;
virtual typename std::map<NodeKey<KeyType>, Node<KeyType>>::const_iterator cend() const = 0;
};
#endif // VKR\_ISTORAGE