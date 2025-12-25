#include "include/graph.hpp"
#include "include/storage.hpp"
#include "include/optimizer.hpp"

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <utility>

int main() {
    // Создаём два хранилища
    Storage<int> storage1(1);
    Storage<int> storage2(2);
    
    // Создаём рёбра
    Edge edge1{5};  // Ребро между 1 и 2 в storage1
    Edge edge2{3};  // Ребро между 3 и 4 в storage2  
    Edge edge3{7};  // Ребро между 2 (storage1) и 3 (storage2)
    
    // 1. Вершина 1 в storage1 (только внутреннее ребро к 2)
    NodeKey<int> key1(1);
    std::vector<typename Node<int>::Neighbour> n1;
    //n1.push_back(std::make_pair(NodeKey<int>(2), edge1));
    storage1.add_node(Node<int>(key1, n1));
    
    // 2. Вершина 2 в storage1 (внутреннее ребро к 1 + внешнее к 3)
    NodeKey<int> key2(2);
    std::vector<typename Node<int>::Neighbour> n2;
    n2.push_back(std::make_pair(NodeKey<int>(1), edge1));
    
    std::map<int, std::vector<typename Node<int>::Neighbour>> ext2;
    std::vector<typename Node<int>::Neighbour> ext_n2;
    ext_n2.push_back(std::make_pair(NodeKey<int>(3), edge3));
    ext2[2] = ext_n2;  // storage2 имеет ID=2
    
    storage1.add_node(Node<int>(key2, n2, ext2));
    
    // 3. Вершина 3 в storage2 (внутреннее ребро к 4 + внешнее к 2)
    NodeKey<int> key3(3);
    std::vector<typename Node<int>::Neighbour> n3;
    //n3.push_back(std::make_pair(NodeKey<int>(4), edge2));
    
    std::map<int, std::vector<typename Node<int>::Neighbour>> ext3;
    std::vector<typename Node<int>::Neighbour> ext_n3;
    ext_n3.push_back(std::make_pair(NodeKey<int>(2), edge3));
    ext3[1] = ext_n3;  // storage1 имеет ID=1
    
    storage2.add_node(Node<int>(key3, n3, ext3));
    
    // 4. Вершина 4 в storage2 (только внутреннее ребро к 3)
    NodeKey<int> key4(4);
    std::vector<typename Node<int>::Neighbour> n4;
    n4.push_back(std::make_pair(NodeKey<int>(3), edge2));
    storage2.add_node(Node<int>(key4, n4));

    Edge edge4{7};  // Ребро между 4 и 5 (storage2)
    NodeKey<int> key5(5);
    std::vector<typename Node<int>::Neighbour> n5;
    n5.push_back(std::make_pair(NodeKey<int>(4), edge4));
    storage2.add_node(Node<int>(key5, n5));
    
    // Вывод информации
    std::cout << "Storage 1: " << storage1.size() << " вершин" << std::endl;
    std::cout << "Storage 2: " << storage2.size() << " вершин" << std::endl;
    std::cout << "Рёбер между хранилищами: " 
              << storage1.get_total_edges_to_storage(2) << std::endl;

    StorageOptimizer optimizer(storage1, storage2);

    int weight1 = storage1.get_internal_edges_weight_sum();
    int weight2 = storage2.get_internal_edges_weight_sum();
    int weight3 = storage1.get_external_edges_weight_sum_to_storage(2);

    std::cout << "internal weight 1 = " << weight1 << std::endl;
    std::cout << "internal weight 2 = " << weight2 << std::endl;
    std::cout << "external weight   = " << weight3 << std::endl;

    std::cout << "metric = " << optimizer.calculate_gv() << std::endl;
    
    return 0;
}