#include <omp.h>
#include <stdio.h>

int collatz_reaches_one(long long n) {
    while (n != 1) {
        if (n % 2 == 0) {
            n = n / 2;
        } else {
            n = 3 * n + 1;
        }
        // Safety check to prevent infinite loops
        if (n < 1) {
            return 0;
        }
    }
    return 1;
}

int main() {
    int START_N = 1000000000;
    int END_N = 1500000000;

    int all_reach_one = 1;
    int thread_count = 100;

    double start_time = omp_get_wtime();

#pragma omp parallel for num_threads(thread_count) reduction(&& : all_reach_one)
    for (int i = START_N; i <= END_N; i++) {
        int result = collatz_reaches_one(i);
        all_reach_one = all_reach_one && result;
    }

    double end_time = omp_get_wtime();
    double elapsed_time = end_time - start_time;

    printf("All numbers from %d to %d reach 1 in Collatz sequence: %s\n", START_N, END_N,
           all_reach_one ? "true" : "false");
    printf("Time taken: %.6f seconds\n", elapsed_time);
    return 0;
}
