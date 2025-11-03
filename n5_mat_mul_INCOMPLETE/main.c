#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Returns a matrix in row-major order (e.g. [<row1><row2><row3>])
int *generate_matrix(unsigned long size) {
    int *mat = malloc(size * size * sizeof(int));
    return mat;
}

int *generate_vector(unsigned long size) {
    int *vec = malloc(size * sizeof(int));
    return vec;
}

int *serial_vec_mat_multiplication(const int *mat, const int *vec, unsigned long size) {
    int *result = malloc(size * sizeof(int));

    for (unsigned long i = 0; i < size; i++) {
        result[i] = 0;
        for (unsigned long j = 0; j < size; j++) {
            result[i] += mat[(i * size) + j] * vec[j];
        }
    }

    return result;
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    for (unsigned long size = 1; size < (1UL << 31); size = size << 1) {
        int *mat = generate_matrix(size);
        int *vec = generate_vector(size);

        int *serial_res = serial_vec_mat_multiplication(mat, vec, size);

        int rows_per_proc = (size + world_size - 1) / world_size;

        free(mat);
        free(vec);
        free(serial_res);
    }

    // MPI_Send(&inital_num, 1, MPI_INT, parent, 0, MPI_COMM_WORLD);
    // MPI_Recv(&res, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Finalize the MPI environment. No more MPI calls can be made after this
    MPI_Finalize();

    // Silence warning
    return 0;
}

// void Mat_vect_mult(double local_A[] /* in  */, double local_x[] /* in  */,
//                    double local_y[] /* out */, int local_m /* in  */, int n /* in  */,
//                    int local_n /* in  */, MPI_Comm comm /* in  */) {
//     double *x;
//     int local_i, j;
//     int local_ok = 1;

//     x = malloc(n * sizeof(double));
//     MPI_Allgather(local_x, local_n, MPI_DOUBLE, x, local_n, MPI_DOUBLE, comm);

//     for (local_i = 0; local_i < local_m; local_i++) {
//         local_y[local_i] = 0.0;
//         for (j = 0; j < n; j++)
//             local_y[local_i] += local_A[local_i * n + j] * x[j];
//     }
//     free(x);
// } /* Mat_vect_mult */
