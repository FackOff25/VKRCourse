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
        case OptimizerType::NONE:
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

    void run_optimization() {
        if (storages.size() < 2) {
            std::cout << "run_optimization: Недостаточно хранилищ для оптимизации\n";
            return;
        }

        std::cout << "=== Запуск попарной оптимизации всех хранилищ ===\n";
        std::cout << "Количество хранилищ: " << storages.size() << "\n\n";

        // Собираем список активных id хранилищ
        std::vector<int> storage_ids;
        for (const auto& s : storages) {
            if (s) storage_ids.push_back(s->get_id());
        }

        // Генерация всех уникальных пар в желаемом порядке
        std::vector<std::pair<int, int>> pairs;

        size_t n = storage_ids.size();

        // Для 4 хранилищ: 1-2, 3-4, 1-3, 2-4, 1-4, 2-3
        if (n == 4) {
            pairs = {
                {1,2}, {3,4},
                {1,3}, {2,4},
                {1,4}, {2,3}
            };
        } 
        else {
            // Общий алгоритм для любого количества хранилищ
            // Сначала пары "по кругу", затем остальные
            for (size_t i = 0; i < n; ++i) {
                for (size_t j = i + 1; j < n; ++j) {
                    pairs.emplace_back(storage_ids[i], storage_ids[j]);
                }
            }
        }

        std::cout << "Всего пар для оптимизации: " << pairs.size() << "\n";

        int pair_count = 0;
        for (const auto& [id1, id2] : pairs) {
            pair_count++;
            std::cout << "Оптимизация пары " << pair_count << "/" << pairs.size() 
                    << ": " << id1 << " <-> " << id2 << std::endl;

            if (optimizer) {
                optimizer->optimize(id1, id2);
            } else {
                std::cerr << "  Ошибка: optimizer не инициализирован!\n";
            }
        }

    std::cout << "=== Попарная оптимизация завершена ===\n\n";
    };

    std::string find_path(Node<KeyType> from, Node<KeyType> to);
};