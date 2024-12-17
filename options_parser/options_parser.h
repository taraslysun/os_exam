#ifndef MYCAT_CONFIG_FILE_H
#define MYCAT_CONFIG_FILE_H

#include <boost/program_options.hpp>
#include <string>
#include <exception>
#include <stdexcept>

class OptionsParseException : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};


class command_line_options_t {
public:
    command_line_options_t();
    command_line_options_t(int ac, char **av);

    //! Explicit is better than implicit:
    command_line_options_t(const command_line_options_t&) = default;
    command_line_options_t& operator=(const command_line_options_t&) = delete;
    command_line_options_t(command_line_options_t&&) = default;
    command_line_options_t& operator=(command_line_options_t&&) = delete;
    ~command_line_options_t() = default;

    [[nodiscard]] std::string get_directory() const { return directory; };
    [[nodiscard]] int get_requests_n() const { return requests_n; };
    [[nodiscard]] int get_client_idx() const { return client_idx; };


    void parse(int ac, char **av);
private:
    int requests_n = 1;
    int client_idx = 0;
    std::string directory = "";

    boost::program_options::variables_map var_map{};
    boost::program_options::options_description opt_conf{
            "Config File Options:\n\tasync [-r] <Number of requests> [-d] <output directory>\ [-c] <client type>n"};
};

#endif //MYCAT_CONFIG_FILE_H

