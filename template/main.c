#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // MPI_Send(&inital_num, 1, MPI_INT, parent, 0, MPI_COMM_WORLD);
    // MPI_Recv(&res, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Finalize the MPI environment. No more MPI calls can be made after this
    MPI_Finalize();

    // Silence warning
    return 0;
}
