#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

void add(double *arr1, double *arr2, double *result, int start, int end) {
        for (int i = start; i < end; ++i) {
                result[i] = arr1[i] + arr2[i];
        }
}

void substract(double *arr1, double *arr2, double *result, int start, int end) {
        for (int i = start; i < end; ++i) {
                result[i] = arr1[i] + arr2[i];
        }
}

void multiply(double *arr1, double *arr2, double *result, int start, int end) {
        for (int i = start; i < end; ++i) {
                result[i] = arr1[i] * arr2[i];
        }
}

void divide(double *arr1, double *arr2, double *result, int start, int end) {
        for (int i = start; i < end; ++i) {
                result[i] = arr1[i] / arr2[i];
        }
}

int main(int argc, char *argv[]) {

        double *arr1, *arr2, *sum, *difference, *product, *quotient;
        double *local_arr1, *local_arr2, *local_sum, *local_diff, *local_prod, *local_quot;
        int i, j, array_size, num_iterations;
        double total_time = 0.0, average_time;
        char *endptr;

        int rank, size;
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        if (rank == 0) {
                char *array_size_str = getenv("ARRAY_SIZE");
                if (array_size_str == NULL) {
                        fprintf(stderr, "Error: ARRAY_SIZE environment variable not set. \n");
                        MPI_Abort(MPI_COMM_WORLD, 1);
                        return 1;
                }
                array_size = strtol(array_size_str, &endptr, 10);
                if (*endptr != '\0' || array_size <= 0) {
                        fprintf(stderr, "Error: Incorrect ARRAY_SIZE value: %s\n", array_size_str);
                        MPI_Abort(MPI_COMM_WORLD, 1);
                        return 1;
                }
                char *num_iterations_str = getenv("NUM_ITERATIONS");
                if (num_iterations_str == NULL) {
                        fprintf(stderr, "Error: NUM_ITERATIONS environment variable not set. \n");
                        MPI_Abort(MPI_COMM_WORLD, 1);
                        return 1;
                }
                num_iterations = strtol(num_iterations_str, &endptr, 10);
                if (*endptr != '\0' || num_iterations <= 0) {
                        fprintf(stderr, "Error: Incorrect NUM_ITERATIONS value: %s\n", num_iterations_str);
                        MPI_Abort(MPI_COMM_WORLD, 1);
                        return 1;
                }
        }

        MPI_Bcast(&array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&num_iterations, 1, MPI_INT, 0, MPI_COMM_WORLD);

        int local_size = array_size / size;
        int remainder = array_size % size;

        int *counts = (int *)malloc(size * sizeof(int));
        int *displs = (int *)malloc(size * sizeof(int));

        for (i = 0; i < size; i++) {
                counts[i] = local_size;
                if (i < remainder) counts[i]++;
                displs[i] = (i == 0) ? 0 : (displs[i-1] + counts[i-1]);
        }

        int my_count = counts[rank];
        int my_displ = displs[rank];

        local_arr1 = (double *)malloc(my_count * sizeof(double));
        local_arr2 = (double *)malloc(my_count * sizeof(double));
        local_sum = (double *)malloc(my_count * sizeof(double));
        local_diff = (double *)malloc(my_count * sizeof(double));
        local_prod = (double *)malloc(my_count * sizeof(double));
        local_quot = (double *)malloc(my_count * sizeof(double));

        if (rank == 0) {
                arr1 = (double *)malloc(array_size * sizeof(double));
                arr2 = (double *)malloc(array_size * sizeof(double));
                sum = (double *)malloc(array_size * sizeof(double));
                difference = (double *)malloc(array_size * sizeof(double));
                product = (double *)malloc(array_size * sizeof(double));
                quotient = (double *)malloc(array_size * sizeof(double));

                if (arr1 == NULL || arr2 == NULL || sum == NULL || difference == NULL || product == NULL || quotient == NULL) {
                        fprintf(stderr, "Ошибка выделения памяти\n");
                        MPI_Abort(MPI_COMM_WORLD, 1);
                        return 1;
                }
        }
        MPI_Barrier(MPI_COMM_WORLD);
        double start_time, end_time;

        for (j = 0; j < num_iterations; ++j) {
                if (rank == 0) {
                        srand(time(NULL)+j);
                        for (i = 0; i < array_size; ++i) {
                                arr1[i] = (double)rand() / RAND_MAX * 100.0;
                                arr2[i] = (double)rand() / RAND_MAX * 100.0 + 1.0;
                        }
                }

                if (rank == 0) start_time = MPI_Wtime();

                MPI_Scatterv(arr1, counts, displs, MPI_DOUBLE, local_arr1, my_count, MPI_DOUBLE, 0, MPI_COMM_WORLD);
                MPI_Scatterv(arr2, counts, displs, MPI_DOUBLE, local_arr2, my_count, MPI_DOUBLE, 0, MPI_COMM_WORLD);

                add(local_arr1, local_arr2, local_sum, 0, my_count);
                substract(local_arr1, local_arr2, local_diff, 0, my_count);
                multiply(local_arr1, local_arr2, local_prod, 0, my_count);
                divide(local_arr1, local_arr2, local_quot, 0, my_count);

                MPI_Gatherv(local_sum, my_count, MPI_DOUBLE, sum, counts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
                MPI_Gatherv(local_diff, my_count, MPI_DOUBLE, difference, counts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
                MPI_Gatherv(local_prod, my_count, MPI_DOUBLE, product, counts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);
                MPI_Gatherv(local_quot, my_count, MPI_DOUBLE, quotient, counts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

                if (rank == 0) {
                        end_time = MPI_Wtime();
                        total_time += (end_time - start_time);
                }
        }

        if (rank == 0) {
                average_time = total_time / num_iterations;
                printf("Average sequential execution time (over %d iterations): %f sec\n", num_iterations, average_time);
                printf("num_threads: %d\n", size);
        }

        free(local_arr1);
        free(local_arr2);
        free(local_sum);
        free(local_diff);
        free(local_prod);
        free(local_quot);
        free(counts);
        free(displs);

        if (rank == 0) {
                free(arr1);
                free(arr2);
                free(sum);
                free(difference);
                free(product);
                free(quotient);
        }
        MPI_Finalize();
        return 0;
}
