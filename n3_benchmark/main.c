#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    const int max_pass = 32;

    for (long current_pass = 0; current_pass <= max_pass; current_pass++) {

        double start_time = MPI_Wtime();

        int array_size = 1 << current_pass;
        int *big_array = malloc((unsigned long)array_size);

        double alloc_time = MPI_Wtime();

        if (my_rank == 0) {
            MPI_Send(big_array, array_size, MPI_CHAR, 1, 0, MPI_COMM_WORLD);
        } else if (my_rank == 1) {
            MPI_Recv(big_array, array_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        double end_time = MPI_Wtime();

        if (my_rank == 0) {
            printf("Sender: Elapsed time = %f seconds, Alloc time = %f seconds, Pass = %ld\n",
                   end_time - start_time, alloc_time - start_time, current_pass);
        } else if (my_rank == 1) {
            printf("Receiver: Elapsed time = %f seconds, Alloc time = %f seconds, Pass = %ld\n",
                   end_time - start_time, alloc_time - start_time, current_pass);
        }

        free(big_array);
    }

    // Finalize the MPI environment. No more MPI calls can be made after this
    MPI_Finalize();

    // Silence warning
    return 0;
}
