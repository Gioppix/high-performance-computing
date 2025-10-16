#include <mpi.h>
#include <stdio.h>

// Closed loop exercise

int receive(int source_process) {
    int received_number = -1;

    MPI_Recv(&received_number, 1, MPI_INT, source_process, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    return received_number;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int received_number = my_rank == 0 ? 0 : -1;

    // All processes but number 0 must receive first
    if (my_rank != 0) {
        received_number = receive(my_rank - 1);
    } else {
        printf("Initial number: %d\n", received_number);
    }

    received_number++;
    MPI_Send(&received_number, 1, MPI_INT, (my_rank + 1) % world_size, 0, MPI_COMM_WORLD);

    // Process 0 must receive last
    if (my_rank == 0) {
        received_number = receive(world_size - 1);
        printf("Final number: %d\n", received_number);
    }

    // Finalize the MPI environment. No more MPI calls can be made after this
    MPI_Finalize();
}
