#include "read_cfg.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <string>
#include <vector>

void parse_cfg(const char* f_name, std::unordered_map<std::string, std::string>& cfg) {
    std::ifstream cfg_file(f_name);

    if (!cfg_file.is_open()) {
        throw std::runtime_error("Unable to open configuration file.");
    }

    std::string line;
    int line_number = 0;
    int mode = -1;
    int process_count = 0;
    std::vector<std::string> ip_addresses;

    try {
        while (std::getline(cfg_file, line)) {
            if (line.empty()) continue;

            if (line_number == 0) {
                mode = std::stoi(line);
                cfg["mode"] = std::to_string(mode);
            } else if (line_number == 1) {
                process_count = std::stoi(line);
                cfg["process_count"] = std::to_string(process_count);
            } else {
                if (mode == 0 && line_number == 2) {
                    cfg["shared_memory"] = line;
                } else if (mode == 1) {
                    ip_addresses.push_back(line);
                }
            }

            ++line_number;
        }

        if (mode == 1) {
            for (size_t i = 0; i < ip_addresses.size(); ++i) {
                cfg["ip_address_" + std::to_string(i)] = ip_addresses[i];
            }
        }

        if (mode == 1 && ip_addresses.size() != static_cast<size_t>(process_count)) {
            throw std::runtime_error("Number of IP addresses does not match process count.");
        }

    } catch (const std::exception& e) {
        std::cerr << "Error parsing configuration file: " << e.what() << std::endl;
    }
}
