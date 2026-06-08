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

        pathfinder = std::make_unique<AStarPathfinder<KeyType>>(*bus.get());
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
            storage = std::make_unique<FennelAStarStorage<KeyType>>(id, std::move(adjuster), cfg.fennel_params);
            break;
        default:
            storage = std::make_unique<SimpleAStarStorage<KeyType>>(id, std::move(adjuster));
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

        std::vector<int> ids;
        for (const auto& s : storages) {
            if (s) ids.push_back(s->get_id());
        }

        size_t n = ids.size();
        std::vector<std::pair<int, int>> all_pairs;

        // Генерация расписания по кругам (round-robin)
        if (n % 2 == 1) {
            // Если нечётное количество — добавляем фиктивное хранилище
            ids.push_back(-1);
            n++;
        }

        int rounds = n - 1;  // количество кругов

        std::cout << "Количество кругов: " << rounds << "\n";

        for (int round = 0; round < rounds; ++round) {
            std::cout << "\n--- Круг " << (round + 1) << " ---\n";

            std::vector<std::pair<int, int>> round_pairs;

            // Фиксируем первую пару (1-й с последним в текущем круге)
            for (size_t i = 0; i < n / 2; ++i) {
                int a = ids[i];
                int b = ids[n - 1 - i];

                if (a != -1 && b != -1 && a < b) {  // исключаем фиктивное и дубли
                    round_pairs.emplace_back(a, b);
                    std::cout << "  " << a << " <-> " << b << std::endl;
                }
            }

            // Добавляем пары текущего круга в общее расписание
            all_pairs.insert(all_pairs.end(), round_pairs.begin(), round_pairs.end());

            // Поворот для следующего круга (классический round-robin)
            if (n > 2) {
                int last = ids.back();
                ids.erase(ids.begin() + 1);
                ids.insert(ids.begin() + ids.size() - 1, last);  // ротация
            }
        }

        std::cout << "\n=== Выполнение оптимизации по парам ===\n";

        int pair_count = 0;
        for (const auto& [id1, id2] : all_pairs) {
            pair_count++;
            std::cout << "Оптимизация " << pair_count << "/" << all_pairs.size() 
                    << ": " << id1 << " <-> " << id2 << std::endl;

            if (optimizer) {
                optimizer->optimize(id1, id2);
            } else {
                std::cerr << "  Предупреждение: optimizer не установлен!\n";
            }
        }

        std::cout << "=== Круговая оптимизация завершена (всего пар: " 
                << all_pairs.size() << ") ===\n\n";
    };

    std::string find_path(NodeKey<KeyType> from, NodeKey<KeyType> to) {
        Path<KeyType> path = pathfinder->find_path(from, to);

        std::ostringstream oss;
        for (size_t i = 0; i < path.size(); ++i) {
            if (i != 0)
                oss << " -> ";

            oss << path[i].key_value;
        }

        return oss.str();
    };
};