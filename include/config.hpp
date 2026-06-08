#ifndef VKR_CONFIG
#define VKR_CONFIG

#include "fennel_params.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

enum StreamingType {
    SIMPLE,
    FENNEL
};

enum RequestWeightType {
    SCHISM,
    WAWPART
};

enum OptimizerType {
    NONE,
    KL
};

struct Config{
public:
    const int storage_num;
    const StreamingType streaming_type;
    const RequestWeightType request_weigh_type;
    const FennelParameters fennel_params;
    const OptimizerType optimzier_type;
};


class ConfigParser {
public:
    static Config load_from_file(const std::string& filename) {
        // Промежуточные переменные
        int storage_num = 4;
        StreamingType streaming_type = FENNEL;
        RequestWeightType request_weight_type = SCHISM;
        OptimizerType optimizer_type = KL;
        int fennel_storage_number = 4;
        float fennel_balancing_number = 1.5f;

        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Ошибка: не удалось открыть файл конфигурации " << filename << std::endl;
        } else {
            std::string line;
            while (std::getline(file, line)) {
                if (line.empty() || line[0] == '#') continue;

                size_t eq_pos = line.find('=');
                if (eq_pos == std::string::npos) continue;

                std::string key = line.substr(0, eq_pos);
                std::string value = line.substr(eq_pos + 1);

                // trim spaces
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);

                if (key == "storage_num") {
                    storage_num = std::stoi(value);
                }
                else if (key == "streaming_type") {
                    streaming_type = (value == "FENNEL") ? FENNEL : SIMPLE;
                }
                else if (key == "request_weight_type") {
                    request_weight_type = (value == "WAWPART") ? WAWPART : SCHISM;
                }
                else if (key == "optimizer_type") {
                    optimizer_type = (value == "KL") ? KL : NONE;
                }
                else if (key == "fennel_balancing_number") {
                    fennel_balancing_number = std::stof(value);
                }
            }
        }

        // Создаём Config только в конце
        Config cfg = {
            storage_num,
            streaming_type,
            request_weight_type,
            {storage_num, fennel_balancing_number},
            optimizer_type
        };
        return cfg;
    }
};

#endif // VKR\_CONFIG