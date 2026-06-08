#ifndef VKR_PATHFINDER
#define VKR_PATHFINDER

#include "graph.hpp"
#include <vector>

template <typename KeyType>
using Path = std::vector<NodeKey<KeyType>>;

template <typename KeyType> 
class IPathfinder {
public:
    virtual Path<KeyType> find_path(const NodeKey<KeyType>& from, const NodeKey<KeyType>& to) = 0;
};

template<typename KeyType>
class AStarPathfinder : public IPathfinder<KeyType> {
private:
    IBus<KeyType>& bus;

public:
    AStarPathfinder(IBus<KeyType>& bus_ref) : bus(bus_ref) {}

    Path<KeyType> find_path(const NodeKey<KeyType>& start, const NodeKey<KeyType>& goal) override {
        std::cout << "AStarPathfinder: " << start.key_value << " → " << goal.key_value << std::endl;

        Path<KeyType> full_path;
        std::unordered_set<NodeKey<KeyType>> global_visited;
        NodeKey<KeyType> current = start;
        int max_hops = 25;

        for (int hop = 0; hop < max_hops; ++hop) {
            int storage_id = bus.ask_who_has(0, current);
            if (storage_id == -1) {
                std::cerr << "Вершина " << current.key_value << " не найдена\n";
                return {};
            }

            LocalPathResult<KeyType> res = bus.request_local_path(storage_id, current, goal, global_visited);

            if (!res.success || res.path.empty()) {
                std::cout << "  Тупиковый путь в хранилище " << storage_id << "\n";
                return {};
            }

            // Добавляем фрагмент
            for (const auto& nk : res.path) {
                if (!full_path.empty() && full_path.back() == nk) continue;
                full_path.push_back(nk);
                global_visited.insert(nk);
            }

            if (full_path.back() == goal) {
                std::cout << "AStarPathfinder: Путь успешно найден (" << full_path.size() << " вершин)\n";
                return full_path;
            }

            if (res.exit_node.key_value == 0) {
                std::cout << "  Путь оборвался\n";
                return {};
            }

            current = res.exit_node;
            std::cout << "  → Переход через вершину " << current.key_value << std::endl;
        }

        std::cout << "AStarPathfinder: Превышен лимит переходов\n";
        return {};
    }
};

#endif // VKR\_IPATHFINDER