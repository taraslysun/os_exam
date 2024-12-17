#include <iostream>
#include <boost/program_options.hpp>
#include <fstream>


void parse_cfg(const char* f_name, std::unordered_map<std::string, std::string>& cfg);