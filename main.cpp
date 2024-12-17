#include <iostream>
#include <cmath>
#include <vector>
#include <thread>
#include "time_measurements.hpp"
#include "read_cfg_integral.hpp"
#include <sys/stat.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/stat.h>


constexpr double pi = 3.14159265358979323846;

using ToIntegrateFunction = double (*)(double x1, double x2);

double function1(double x1, double x2) {
  double res = 0;
  for (int i = -2; i < 3; ++i) {
    for (int j = -2; j < 3; ++j) {
      double s = 1/(5*( i + 2 ) + j + 3 +
          std::pow( (x1 - 16*j), 6 ) + std::pow( (x2 - 16*i), 6 ));
      res += s;
    }
  }

  res = (1/(res+0.002));

  return res;
}

double function2(double x1, double x2) {
  double a = 20;
  double b = 0.2;
  double c = 2*pi;
  double res = (-1*a) * exp(-b * sqrt(0.5 *( x1*x1 + x2*x2 )))
      - exp(0.5*( cos(c*x1) + cos(c*x2) ) ) + a + exp(1) ;
  return res;
}

double function3(double x1, double x2) {
  size_t m = 5;
  int a1[5] = {1, 2, 1, 1, 5};
  int a2[5] = {4, 5, 1, 2, 4};
  int c[5] = {2, 1, 4, 7, 2};
  double res = 0;
  for (size_t i = 0; i < m; ++i) {
    double s = c[i] * exp( (-1.0/pi) * ( (x1 - a1[i] ) * (x1 - a1[i] )
        + ( x2 - a2[i] ) * ( x2 - a2[i] ) ) ) * cos( pi*( (x1 - a1[i] ) * (x1 - a1[i] )
        + ( x2 - a2[i] ) * ( x2 - a2[i] ) ) )  ;
    res += s;
  }

  return -1 * res;
}


void integrate(ToIntegrateFunction func, double x1, const double& x2,
                 double y1, const double& y2 , int steps_x, int steps_y,
                 double* results, size_t idx, double fx=0)
{
  double res = fx * 0.5;
  double delta_x = (x2 - x1)/steps_x;
  double delta_y = (y2 - y1)/steps_y;

  x1 = fx == 0 ? x1 : x1 + delta_x;
  y1 = fx == 0 ? y1 : y1 + delta_y;

  double m = fx == 0 ? 1 : 2;

  double temp_y1 = y1;
  while(x1 < x2)
  {
    double xi = (x1 + x1 + delta_x*m)*0.5;
    y1 = temp_y1;

    while ( y1 < y2 ) {
      double yi = (y1 + y1 + delta_y*m)*0.5;
      y1 += delta_y*m;
      res += func(xi, yi) * delta_y * delta_x * m ;
    }
    x1 += delta_x*m;
  }
  results[idx] = res;
}

bool exists(const char *fileName)
{
  struct stat buffer;
  return (stat (fileName, &buffer) == 0);
}


int roundToPow(int num){
  int i=2;
  while (i<num) i*=2;
  return i;
}


int main (int argc, char* argv[]) {

  int type;
  size_t n_proc;

  const char* fileName;
  if (argc != 4) {
    std::cerr << "Wrong amount of input arguments" << std::endl;
    return 1;
  } else {
    fileName = argv[2];
    type = std::stoi(argv[1]);
    n_proc = std::stoi(argv[3]);
    if (!exists(fileName)){
      std::cout << "Couldn't read config file" << std::endl;
      return 3;
    }

    if (type < 1 || type > 3) {
      std::cerr << "Wrong method's number"<< std::endl;
      return 2;
    }
  }

  std::vector<ToIntegrateFunction> functionVector(3);
  functionVector[0]=&function1;
  functionVector[1]=&function2;
  functionVector[2]=&function3;


  std::unordered_map<std::string, double> cfg;
  parse_cfg(fileName, cfg);

  double x1 = cfg["x_start"];
  double x2 = cfg["x_end"];
  double y1 = cfg["y_start"];
  double y2 = cfg["y_end"];
  double cfg_rel_err = cfg["rel_err"];
  double cfg_abs_err = cfg["abs_err"];


  int steps_x = roundToPow(static_cast<int>(cfg["init_steps_x"])
      /  static_cast<int>(n_proc));
  int steps_y = roundToPow(static_cast<int>(cfg["init_steps_y"])) ;
  double abs_err = -1;
  double rel_err = -1;
  int i = 0;
  double cur_res = 0;
  double prev_res;

  double x_dst = fabs( x2 - x1 );

  std::vector<double> prev_results(n_proc, 0);

  auto before = get_current_time_fenced();

  while ( i++ < cfg["max_iter"] ) {

    for (size_t j = 0; j < n_proc; ++j) {
        pid_t pid = fork();
    }

    for (size_t j = 0; j < n_proc; ++j) {
        wait(nullptr);
    }

    prev_res = cur_res;
    cur_res = 0;
    for ( size_t j = 0; j < n_proc; ++j ) {
      cur_res += results[j];
      prev_results[j] = results[j];
    }

//    std::cout << "Curr res: " << cur_res << std::endl;
    steps_x *=2, steps_y *= 2;

    abs_err = fabs(cur_res-prev_res);
    rel_err = fabs((cur_res-prev_res)/cur_res );

  //  std::cout << "Abs, rel : " << abs_err << " " << rel_err << std::endl;

    if ( abs_err < cfg_abs_err && rel_err < cfg_rel_err ) break;
  }

  auto time_to_calculate = get_current_time_fenced() - before;
  if ( abs_err > cfg_abs_err || rel_err > cfg_rel_err ){
    std::cout << "couldn't reach requested accuracy" << std::endl;
    std::cout << "Current accuracy: Abs err - " << abs_err << " Rel err -" << rel_err << std::endl;
    return 6;
  }


  munmap(results, sizeof(double) * n_proc);
  shm_unlink("/integrate_results");
  sem_close(sem);
  sem_unlink("/integrate_semaphore");
  std::cout << cur_res << "\n";
  std::cout << abs_err << "\n";
  std::cout << rel_err << "\n";
  std::cout << to_us(time_to_calculate) << "\n";
  return 0;
}
