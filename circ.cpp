#include "MyMPI.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    MyMPI mpi(argc, argv);

    int rank = mpi.get_rank();
    int world_size = mpi.get_world_size();

    int value = 0;
    const int num_iterations = 5;

    if (rank == 0) {
        value = 42;
        std::cout << "Rank 0 starts the circle with value: " << value << std::endl;
    }
    sleep(3);

    for (int iteration = 0; iteration < num_iterations; ++iteration) {
        int next_rank = (rank + 1) % world_size; 
        int prev_rank = (rank - 1 + world_size) % world_size;

        if (rank == 0 && iteration == 0) {
            mpi.send(next_rank, &value, 1);
            std::cout << "Rank " << rank << " sent value: " << value << " to Rank " << next_rank << std::endl;
        }

        mpi.recv(prev_rank, &value, 1);
        std::cout << "Rank " << rank << " received value: " << value << " from Rank " << prev_rank << std::endl;
        value ++;

        mpi.send(next_rank, &value, 1);
        std::cout << "Rank " << rank << " sent updated value: " << value << " to Rank " << next_rank << std::endl;

        mpi.barrier();
    }

    if (rank == 0) {
        std::cout << "Circular communication completed after " << num_iterations << " iterations." << std::endl;
    }

    return 0;
}
