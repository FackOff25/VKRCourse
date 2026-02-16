#include "include/graph.hpp"
#include "include/bus.hpp"
#include "include/storage.hpp"
#include "include/optimizer.hpp"

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <utility>

int main() {
    SimpleBus<int> bus;
    Storage<int> storage1(1);
    Storage<int> storage2(2);
    
    bus.connect_storage(&storage1);
    bus.connect_storage(&storage2);
    
    Node<int> node1(1);
    Node<int> node2(2);
    Node<int> node3(3);
    Node<int> node4(4);
    Node<int> node5(5);
    Node<int> node6(6);

    Edge<int> edge1_2(1, 2, 5); node1.add_edge(edge1_2); node2.add_edge(edge1_2);
    Edge<int> edge2_3(2, 3, 7); node2.add_edge(edge2_3); node3.add_edge(edge2_3);
    Edge<int> edge3_4(3, 4, 3); node3.add_edge(edge3_4); node4.add_edge(edge3_4);
    Edge<int> edge3_5(3, 5, 5); node3.add_edge(edge3_5); node5.add_edge(edge3_5);
    Edge<int> edge5_6(5, 6, 6); node5.add_edge(edge5_6); node6.add_edge(edge5_6);

    bus.send_add_node(node1, 1);
    bus.send_add_node(node2, 1);
    bus.send_add_node(node3, 2);
    bus.send_add_node(node4, 2);
    bus.send_add_node(node5, 2);
    bus.send_add_node(node6, 2);

    std::cout << storage1 << std::endl;
    std::cout << storage2 << std::endl;

    std::cout << "Neigbours of 1:" << std::endl;
    for (const auto& element : bus.ask_neigbours_to_storage(1, 2)) {
        std::cout << element << " ";
    }
    std::cout << std::endl;

    std::cout << "Neigbours of 2:" << std::endl;
    for (const auto& element : bus.ask_neigbours_to_storage(2, 1)) {
        std::cout << element << " ";
    }
    std::cout << std::endl;

    std::cout << "edges between 1 to 2:" << std::endl;
    for (const auto& element : bus.ask_edges_to_storage(1, 2)) {
        std::cout << element << " ";
    }
    std::cout << std::endl;

    std::cout << "edges between 2 to 1:" << std::endl;
    for (const auto& element : bus.ask_edges_to_storage(2, 1)) {
        std::cout << element << " ";
    }
    std::cout << std::endl;
    
    //ExternalStorageOptimizer<int> optimizer(&bus);

    //std::map<int, std::map<Node<int>, float>> gvs = optimizer.calculate_gvs(1,2);

    Node<int> removed = bus.request_node(2);
    bus.send_remove_node(2);

    std::cout << storage1 << std::endl;
    std::cout << storage2 << std::endl;

    bus.send_add_node(removed, 2);

    std::cout << storage1 << std::endl;
    std::cout << storage2 << std::endl;

    return 0;
}