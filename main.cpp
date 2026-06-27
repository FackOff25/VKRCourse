#include "master.hpp"
#include "config.hpp"
#include "graph_loader.hpp"
#include <iostream>
#include <string>
#include <iomanip>

int main(int argc, char* argv[]) {
    std::string config_path = argv[1];

    unsigned int seed = 42;
    if (argc >= 3) {
        seed = std::stoul(argv[2]);
    }

    std::cout << "=== Запуск с seed = " << seed << " ===\n";

    Config config = ConfigParser::load_from_file(config_path);
    Master<int> master(config, seed);
    master.inititalize();
    std::string command;

    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, command);

        if (command.empty()) continue;

        if (command == "exit" || command == "quit" || command == "q") {
            std::cout << "Завершение работы...\n";
            break;
        }
        else if (command == "help" || command == "h") {
            std::cout << "Доступные команды:\n"
                      << "  load <metis_file> [coords_file] [optimize every N nodes]  - загрузить граф\n"
                      << "  optimize                        - выполнить оптимизацию\n"
                      << "  path <from> <to>                - найти путь между вершинами\n"
                      << "  stats                           - полная статистика\n"
                      << "  cut                             - показать процент рёбер между хранилищами\n"
                      << "  print                           - вывести все хранилища\n"
                      << "  help                            - эта справка\n"
                      << "  exit / quit                     - выход\n";
        }
        else if (command.substr(0, 4) == "load") {
            std::string metis_file, coords_file;
            int optimize_every = 0;
            std::stringstream ss(command.substr(4));
            ss >> metis_file >> coords_file >> optimize_every;

            if (metis_file.empty()) {
                std::cout << "Использование: load <metis_file> [coords_file]\n";
                continue;
            }

            try {
                GraphLoader<int>::load_to_master(master, metis_file, coords_file, optimize_every);
            } catch (const std::exception& e) {
                std::cerr << "Ошибка при загрузке: " << e.what() << std::endl;
            }
        }
        else if (command == "optimize" || command == "opt") {
            master.run_optimization();
        }
        else if (command.substr(0, 4) == "path") {
            int from_id = -1, to_id = -1;
            std::stringstream ss(command.substr(4));
            ss >> from_id >> to_id;

            if (from_id <= 0 || to_id <= 0) {
                std::cout << "Использование: path <from_id> <to_id>\n";
                continue;
            }

            try {
                NodeKey<int> from_node(from_id);
                NodeKey<int> to_node(to_id);

                std::string result = master.find_path(from_node, to_node);
                std::cout << "Результат поиска пути " << from_id << " → " << to_id << ":\n";
                std::cout << result << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Ошибка при поиске пути: " << e.what() << std::endl;
            }
        }
        else if (command == "cut") {
            double percent = master.get_cut_percent();
            std::cout << "Процент рёбер между хранилищами: " 
                    << std::fixed << std::setprecision(3) << percent << "%\n";
        }
        else if (command == "stats") {
            master.print_stats();
        }
        else if (command == "print") {
            master.log_storages();
        }
        else {
            std::cout << "Неизвестная команда. Введите 'help' для справки.\n";
        }
    }

    return 0;
};
