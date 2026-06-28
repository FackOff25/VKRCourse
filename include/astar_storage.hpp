#ifndef VKR_ASTAR_STORAGE
#define VKR_ASTAR_STORAGE

#include "base_storage.hpp"
#include "local_path_result.hpp"
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <limits>
#include <cmath>
#include <algorithm>

template<typename KeyType>
class AStarStorage : virtual public BaseStorage<KeyType> {
protected:
    typedef NodeKey<KeyType> Key;

public:
    AStarStorage(int id, 
                 std::unique_ptr<IWeightAdjuster<KeyType>> _weight_adjuster,
                 std::map<Key, Node<KeyType>> _nodes = {})
        : BaseStorage<KeyType>(id, std::move(_weight_adjuster), _nodes) {}

LocalPathResult<KeyType> find_local_path(
    const NodeKey<KeyType>& start,
    const NodeKey<KeyType>& goal,
    const std::unordered_set<Key>& global_visited) override
{
   LocalPathResult<KeyType> result;

    if (!this->has_node(start))
        return result;

    using Key = NodeKey<KeyType>;

    std::unordered_set<Key> closed_set = global_visited;

    std::unordered_map<Key, Key> came_from;
    std::unordered_map<Key, double> g_score;
    std::unordered_map<Key, double> f_score;

    g_score[start] = 0.0;

    auto heuristic = [&](const Key& a) -> double
    {
        auto* na = this->get_node(a);
        auto* nb = this->get_node(goal);

        if (!na || !nb)
            return 0.0;

        double dx = na->x - nb->x;
        double dy = na->y - nb->y;

        return std::sqrt(dx * dx + dy * dy);
    };

    auto distance = [&](const Key& a, const Key& b) -> double
    {
        auto* na = this->get_node(a);
        auto* nb = this->get_node(b);

        if (!na || !nb)
            return 0.0;

        double dx = na->x - nb->x;
        double dy = na->y - nb->y;

        return std::sqrt(dx * dx + dy * dy);
    };

    auto reconstruct_path = [&](Key current) -> std::vector<Key>
    {
        std::vector<Key> path;

        while (came_from.count(current))
        {
            path.push_back(current);
            current = came_from[current];
        }

        path.push_back(start);
        std::reverse(path.begin(), path.end());

        return path;
    };

    g_score[start] = 0.0;
    f_score[start] = heuristic(start);

    auto compare = [&](const Key& a, const Key& b)
    {
        return f_score[a] > f_score[b];
    };

    std::priority_queue<
        Key,
        std::vector<Key>,
        decltype(compare)
    > open_set(compare);

    open_set.push(start);

    // ===== ЛУЧШИЙ EXIT (важное исправление) =====
    bool found_exit = false;
    Key best_exit_node{};
    std::vector<Key> best_exit_path;

    while (!open_set.empty())
    {
        Key current = open_set.top();
        open_set.pop();

        if (closed_set.count(current))
            continue;

        closed_set.insert(current);

        // ===== ЦЕЛЬ ВСЕГДА ПРИОРИТЕТ =====
        if (current == goal)
        {
            result.path = reconstruct_path(goal);
            result.success = true;
            return result;
        }

        auto* node = this->get_node(current);

        if (!node)
            continue;

        for (const auto& [neigh_key, edge] : node->edges)
        {
            if (closed_set.count(neigh_key))
                continue;

            if (!this->has_node(neigh_key)) {
                if (!found_exit || heuristic(neigh_key) < heuristic(best_exit_node)) {
                    found_exit = true;
                    best_exit_node = neigh_key;
                    best_exit_path = reconstruct_path(current);
                }
                continue;
            }

            double tentative_g =
                g_score[current] +
                distance(current, neigh_key);

            auto it = g_score.find(neigh_key);

            if (it == g_score.end() || tentative_g < it->second)
            {
                came_from[neigh_key] = current;
                g_score[neigh_key] = tentative_g;
                f_score[neigh_key] = tentative_g + heuristic(neigh_key);

                open_set.push(neigh_key);
            }
        }
    }

    // ===== если goal не найден =====
    if (found_exit)
    {
        result.exit_node = best_exit_node;
        result.path = std::move(best_exit_path);
        result.success = true;
    }

    return result;
}
};

#endif // VKR_ASTAR_STORAGE