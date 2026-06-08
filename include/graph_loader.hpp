#ifndef VKR_GRAPH_LOADER
#define VKR_GRAPH_LOADER

#include "graph.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>
#include <map>

template<typename KeyType = int>
class GraphLoader {
public:
    // Загрузка графа из METIS-файла + опционально файла координат
    static std::map<NodeKey<KeyType>, Node<KeyType>> load_graph(
        const std::string& metis_file, 
        const std::string& coords_file = "") 
    {
        std::map<NodeKey<KeyType>, Node<KeyType>> nodes;

        // === 1. Чтение METIS файла ===
        std::ifstream metis(metis_file);
        if (!metis.is_open()) {
            std::cerr << "Ошибка открытия METIS файла: " << metis_file << std::endl;
            return nodes;
        }

        int N, M;
        metis >> N >> M;

        std::cout << "Загрузка графа: " << N << " вершин, " << M << " рёбер" << std::endl;

        // Создаём все вершины
        for (int i = 1; i <= N; ++i) {
            NodeKey<KeyType> key(i);
            nodes[key] = Node<KeyType>(key);
        }

        // Читаем рёбра
        std::string line;
        std::getline(metis, line); // пропускаем остаток первой строки

        for (int i = 1; i <= N; ++i) {
            std::getline(metis, line);
            std::stringstream ss(line);
            int neighbor;
            NodeKey<KeyType> current(i);

            while (ss >> neighbor) {
                if (neighbor == i) continue; // петля

                NodeKey<KeyType> neigh_key(neighbor);
                Edge<KeyType> edge(current, neigh_key);
                nodes[current].add_edge(edge);
            }
        }

        // === 2. Загрузка координат (опционально) ===
        if (!coords_file.empty()) {
        std::ifstream coords(coords_file);
        if (coords.is_open()) {
            for (int i = 1; i <= N; ++i) {
                double x = 0.0, y = 0.0;
                if (coords >> x >> y) {
                    NodeKey<KeyType> key(i);
                    auto it = nodes.find(key);
                    if (it != nodes.end()) {
                        it->second.x = x;
                        it->second.y = y;
                    }
                }
            }
            std::cout << "Координаты загружены.\n";
        }
    }

        std::cout << "Граф успешно загружен. Вершин: " << nodes.size() << std::endl;
        return nodes;
    }

    // Упрощённая версия — только METIS
    static std::map<NodeKey<KeyType>, Node<KeyType>> load_graph(const std::string& metis_file) {
        return load_graph(metis_file, "");
    }

    // Добавление всех узлов в Master (удобный метод)
    static void load_to_master(Master<KeyType>& master, 
                             const std::string& metis_file, 
                             const std::string& coords_file = "") 
    {
        auto nodes_map = load_graph(metis_file, coords_file);

        std::cout << "Начало потоковой загрузки " << nodes_map.size() << " вершин...\n";

        for (const auto& [key, node] : nodes_map) {
            master.add_node(node);
        }

        std::cout << "Загрузка графа завершена!\n";
    }
};

#endif // VKR_GRAPH_LOADER