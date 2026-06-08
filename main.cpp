#include "include/config.hpp"
#include "include/master.hpp"
#include "include/graph.hpp"
#include "include/graph_loader.hpp"


int main() {
    Config cfg = {
        2,
        StreamingType::FENNEL,
        RequestWeightType::SCHISM,
        { 2, 1.5 },
        OptimizerType::KL
    };

    Master<int> master = Master<int>(cfg);
    master.inititalize();

    GraphLoader<int>::load_to_master(
        master, 
        "graph.metis",           // путь к METIS файлу
        "coordinates.txt"        // путь к файлу координат (опционально)
    );

    master.run_optimization();

    master.log_storages();

    return 0;
}