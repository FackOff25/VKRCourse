#include <vector>
#include <memory>
#include <map>
#include <string>

#include "graph.hpp"
#include "config.hpp"
#include "i_storage.hpp"
#include "interface_bus.hpp"
#include "optimizer.hpp"

template<typename KeyType>
class Master {
private:
    Config cfg;
    IBus<KeyType>* bus;
    std::vector<IStorage<KeyType>*> storages;
    ExternalStorageOptimizer<KeyType>* optimizer = nullptr;
    IPathfinder pathfinder;

public:
    Master() = default;
    explicit Master(IBus<KeyType>* _bus)
        : bus(_bus) {}
    void set_bus(IBus<KeyType>* _bus);
    void set_optimizer(Optimizer<KeyType>* _optimizer);
    void add_storage(IStorage<KeyType>* storage);
    void remove_storage(int storage_id);
    IStorage<KeyType>* get_storage(int storage_id);
    const std::vector<IStorage<KeyType>*>& get_storages() const;
    size_t storage_count() const;
    size_t total_nodes() const;
    size_t total_internal_edges() const;
    void inititalize(Config cfg);
    void run_optimization();
    void print_statistics() const;

    void add_node(Node<KeyType> node);

    std::string find_path(Node<KeyType> from, Node<KeyType> to);
};