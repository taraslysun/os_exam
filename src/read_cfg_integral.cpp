#include "read_cfg_integral.hpp"


void parse_cfg(const char * f_name, std::unordered_map<std::string, double>& cfg) {


  boost::program_options::options_description desc("cfg");
  desc.add_options()
      ("abs_err", boost::program_options::value<double>(&cfg["abs_err"]))
      ("rel_err", boost::program_options::value<double>(&cfg["rel_err"]))
      ("x_start", boost::program_options::value<double>(&cfg["x_start"]))
      ("x_end", boost::program_options::value<double>(&cfg["x_end"]))
      ("y_start", boost::program_options::value<double>(&cfg["y_start"]))
      ("y_end", boost::program_options::value<double>(&cfg["y_end"]))
      ("init_steps_x", boost::program_options::value<double>(&cfg["init_steps_x"]))
      ("init_steps_y", boost::program_options::value<double>(&cfg["init_steps_y"]))
      ("max_iter", boost::program_options::value<double>(&cfg["max_iter"]));

  boost::program_options::variables_map vm;

  try {
    std::ifstream cfg_file(f_name);

    boost::program_options::store(
        boost::program_options::parse_config_file(cfg_file, desc), vm);
    boost::program_options::notify(vm);

  } catch (const std::exception& e) {
    std::cerr << "Error parsing configuration file: " << e.what() << std::endl;
  }

}