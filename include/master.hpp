#include <vector>
#include <memory>
#include <map>
#include <string>

#include "graph.hpp"
#include "config.hpp"
#include "i_storage.hpp"
#include "fennel.hpp"
#include "storage.hpp"
#include "bus.hpp"
#include "optimizer.hpp"
#include "weight_adjuster.hpp"

#include <vector>
#include <memory>

template<typename KeyType>
class Master {
private:
    Config cfg;
    std::unique_ptr<SimpleBus<KeyType>> bus;
    std::vector<std::unique_ptr<IStorage<KeyType>>> storages;
    std::unique_ptr<IExternalStorageOptimizer<KeyType>> optimizer = nullptr;
    std::unique_ptr<IPathfinder<KeyType>> pathfinder = nullptr;

    int next_storage_id = 0;

public:
    Master(Config _cfg) : cfg(_cfg) {
        bus = std::make_unique<SimpleBus<KeyType>>();

        switch (cfg.optimzier_type)
        {
        case OptimizerType::KL:
            optimizer = std::make_unique<KLExternalStorageOptimizer<KeyType>>(bus.get());
            break;
        case OptimizerType::None:
            optimizer = std::make_unique<DummyExternalStorageOptimizer<KeyType>>();
            break;
        }
    };

    int create_storage(int id, StreamingType streaming_type, RequestWeightType request_weight_type) {
        std::unique_ptr<IStorage<KeyType>> storage;
        std::unique_ptr<IWeightAdjuster<KeyType>> adjuster;

        switch (request_weight_type)
        {
        case RequestWeightType::SCHISM:
            adjuster = std::make_unique<SchismAdjuster<KeyType>>();
            break;
        case RequestWeightType::WAWPART:
            adjuster = std::make_unique<WAWAdjuster<KeyType>>();
            break;
        }

        switch (streaming_type)
        {
        case StreamingType::FENNEL:
            storage = std::make_unique<FennelStorage<KeyType>>(id, std::move(adjuster), cfg.fennel_params);
            break;
        default:
            storage = std::make_unique<SimpleStorage<KeyType>>(id, std::move(adjuster));
            break;
        }
        
        bus->connect_storage(storage.get()); 
        storages.push_back(std::move(storage));
        std::cout << "Master: Создано хранилище #" << id << std::endl;
        return id;
    }

    void set_config(Config _cfg) { cfg = _cfg; };
    void set_optimizer(std::unique_ptr<KLExternalStorageOptimizer<KeyType>> _optimizer) { optimizer = std::move(_optimizer); };
    size_t storage_count() const { return storages.size(); };

    void inititalize() {      
        std::cout << "Master: Инициализация с " << cfg.storage_num 
                  << " хранилищами\n";
        
        // Создаём хранилища
        for (int i = 0; i < cfg.storage_num; ++i) {
            create_storage(i+1, cfg.streaming_type, cfg.request_weigh_type);
        }
    };

    void log_storages() const {
        for (const auto& storage_ptr : storages) {
        const BaseStorage<KeyType>* base = dynamic_cast<const BaseStorage<KeyType>*>(storage_ptr.get());
        
        std::cout << *base << "\n\n";           // используем operator<< из BaseStorage
    }
    }

    void add_node(Node<KeyType> node) {
        std::cout << bus->send_add_node(node);
    };

    void run_optimization();
    void print_statistics() const;

    std::string find_path(Node<KeyType> from, Node<KeyType> to);
};