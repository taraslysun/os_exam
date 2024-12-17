#include "MyMPI.hpp"

// int main(int argc, char* argv[]) {
//     MyMPI mpi(argc, argv);

//     int rank = mpi.get_rank();
//     int world_size = mpi.get_world_size();

//     if (rank == 0) {
//         double data[] = {3.14, 2.71, 1.61};
//         mpi.send(1, data, 3);
//         std::cout << "Rank 0: Sent data to Rank 1\n";
//     } else if (rank == 1) {
//         double buffer[3];
//         mpi.recv(0, buffer, 3);
//         std::cout << "Rank 1: Received data: ";
//         for (int i = 0; i < 3; ++i) {
//             std::cout << buffer[i] << " ";
//         }
//         std::cout << std::endl;
//     }

//     mpi.barrier();
//     return 0;
// }
