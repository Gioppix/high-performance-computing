#include <mpi.h>
#include <stdio.h>

// Reduce exercise

// Compact binary tree encoding in an array:
// 0 12 3456 789
// Parent: (index-1)/2
// Left child: index*2+1
// Right child: index*2+2

int receive(int source) {
    int res;
    MPI_Recv(&res, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return res;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    int inital_num = 2;

    // Every process but leafs must receive then send
    // We can detect leafs by checking for children presence

    int left_child = (my_rank * 2) + 1;
    if (left_child < world_size) {
        inital_num += receive(left_child);
    }

    int right_child = (my_rank * 2) + 2;
    if (right_child < world_size) {
        inital_num += receive(right_child);
    }

    if (my_rank != 0) {
        int parent = (my_rank - 1) / 2;
        MPI_Send(&inital_num, 1, MPI_INT, parent, 0, MPI_COMM_WORLD);
    } else {
        printf("Final number: %d\n", inital_num);
    }

    // Finalize the MPI environment. No more MPI calls can be made after this
    MPI_Finalize();

    // Silence warning
    return 0;
}
