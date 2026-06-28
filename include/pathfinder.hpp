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
    virtual ~IPathfinder() = default;
};

template<typename KeyType>
class AStarPathfinder : public IPathfinder<KeyType> {
private:
IBus<KeyType>& bus;
std::string log_file_path = "path_edges.log";

public:
AStarPathfinder(IBus<KeyType>& bus_ref) : bus(bus_ref) {}

AStarPathfinder(IBus<KeyType>& bus_ref, const std::string& log_path) 
    : bus(bus_ref), log_file_path(log_path) {}

Path<KeyType> find_path(const NodeKey<KeyType>& start, const NodeKey<KeyType>& goal) override {
    std::cout << "AStarPathfinder: " << start.key_value << " → " << goal.key_value << std::endl;
    
    Path<KeyType> full_path;
    std::unordered_set<NodeKey<KeyType>> global_visited;
    NodeKey<KeyType> current = start;
    const int max_hops = 500;
    int inter_storage_edges = 0;

    for (int hop = 0; hop < max_hops; ++hop) {
        int storage_id = bus.ask_who_has(0, current);
        if (storage_id == -1) {
            std::cerr << "Вершина " << current.key_value << " не найдена\n";
            log_edges(0, 0);
            return {};
        }

        LocalPathResult<KeyType> res = bus.request_local_path(storage_id, current, goal, global_visited);
        
        if (!res.success || res.path.empty()) {
            std::cout << " Тупиковый путь в хранилище " << storage_id 
                      << " (hop " << hop << ")\n";
            log_edges(0, 0);
            return {};
        }

        // Добавляем путь
        for (const auto& nk : res.path) {
            if (!full_path.empty() && full_path.back() == nk) continue;
            full_path.push_back(nk);
            global_visited.insert(nk);
        }

        if (full_path.back() == goal) {
            std::cout << "AStarPathfinder: Путь успешно найден (" 
                      << full_path.size() << " вершин, " 
                      << inter_storage_edges << " межшардовых)\n";
            
            size_t total_edges = full_path.size() > 0 ? full_path.size() - 1 : 0;
            log_edges(total_edges, inter_storage_edges);
            return full_path;
        }

        if (res.exit_node.key_value == 0) {
            std::cout << " Путь оборвался (нет exit_node)\n";
            log_edges(0, 0);
            return {};
        }

        current = res.exit_node;
        inter_storage_edges++;
        
        // Защита от зацикливания
        if (global_visited.size() > 5000) {   // слишком много посещённых
            std::cout << " Зацикливание обнаружено (visited > 5000)\n";
            log_edges(0, 0);
            return {};
        }
    }

    std::cout << "AStarPathfinder: Превышен лимит переходов (" 
              << max_hops << " hops) для " << start.key_value 
              << " → " << goal.key_value << "\n";
    log_edges(0, 0);
    return {};
}

private:
void log_edges(size_t total_edges, int inter_storage_edges) {
    std::ofstream log_file(log_file_path, std::ios::app);
    if (log_file.is_open()) {
        log_file << total_edges << " " << inter_storage_edges << std::endl;
    } else {
        std::cerr << "Не удалось открыть лог-файл: " << log_file_path << std::endl;
    }
}
};

#endif // VKR\_IPATHFINDER