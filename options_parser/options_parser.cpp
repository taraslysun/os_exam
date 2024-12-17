// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <filesystem>
#include <iostream>
#include "options_parser.h"

namespace po = boost::program_options;

command_line_options_t::command_line_options_t() {
    opt_conf.add_options()
    ("help,h", "Display help message")
    ("requests,r", po::value<int>()->required(), "Number of requests")
    ("directory,d", po::value<std::string>()->required(), "Directory path")
    ("client,c", po::value<int>()->required(), "Client type");


}

command_line_options_t::command_line_options_t(int ac, char **av):
        command_line_options_t()
{
    parse(ac, av);
}

void command_line_options_t::parse(int ac, char **av) {
    try {
        po::parsed_options parsed = po::command_line_parser(ac, av).options(opt_conf).allow_unregistered().run();
        po::store(parsed, var_map);
        if (var_map.count("help")) {
            std::cout << opt_conf << "\n";
            exit(EXIT_SUCCESS);
        }
        po::notify(var_map);
        if (var_map.count("directory")) {
            directory = var_map["directory"].as<std::string>();
        } else {
            throw OptionsParseException("Option '-d' is missing or invalid.");
        }
        if (var_map.count("requests")) {
            requests_n = var_map["requests"].as<int>();
        } else {
            throw OptionsParseException("Option '-r' is missing or invalid.");
        }
        if (var_map.count("client")) {
            client_idx = var_map["client"].as<int>();
        } else {
            throw OptionsParseException("Option '-c' is missing or invalid.");
        }


    
    } catch (std::exception &ex) {
        throw OptionsParseException(ex.what()); // Convert to our error type
    }
}
