#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

#define NUM_RUNS 100

void add(double** arr1, double** arr2, double** result, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i][j] = arr1[i][j] + arr2[i][j];
        }
    }
}

void substract(double** arr1, double** arr2, double** result, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i][j] = arr1[i][j] - arr2[i][j];
        }
    }
}

void multiply(double** arr1, double** arr2, double** result, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i][j] = arr1[i][j] * arr2[i][j];
        }
    }
}

void divide(double** arr1, double** arr2, double** result, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            result[i][j] = arr1[i][j] / arr2[i][j];
        }
    }
}

double** allocate_matrix(int rows, int cols) {
    double** matrix = (double**)malloc(rows * sizeof(double*));
    for (int i = 0; i < rows; ++i) {
        matrix[i] = (double*)malloc(cols * sizeof(double));
    }
    return matrix;
}

void free_matrix(double** matrix, int rows) {
    for (int i = 0; i < rows; ++i) {
        free(matrix[i]);
    }
    free(matrix);
}

void initialize_matrix(double** matrix, int rows, int cols, int seed) {
    srand(time(NULL) + seed);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            matrix[i][j] = (double)rand() / RAND_MAX * 100.0;
        }
    }
}

int main(int argc, char** argv) {
    int rank, size;
    int rows, cols;
    double** local_arr1, ** local_arr2;
    double** local_sum, ** local_diff, ** local_prod, ** local_quot;
    double total_time = 0.0, average_time;
    char* endptr;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (rank == 0) {
        char* array_size_str = getenv("ARRAY_SIZE");
        if (array_size_str == NULL) {
            fprintf(stderr, "Error: ARRAY_SIZE environment variable not set.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        rows = strtol(array_size_str, &endptr, 10);
        if (*endptr != '\0' || rows <= 0) {
            fprintf(stderr, "Error: Incorrect ARRAY_SIZE value: %s\n", array_size_str);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        cols = rows;
    }
    
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    int rows_per_proc = rows / size;
    int remainder = rows % size;
    
    int local_rows = rows_per_proc + (rank < remainder ? 1 : 0);
    
    local_arr1 = allocate_matrix(local_rows, cols);
    local_arr2 = allocate_matrix(local_rows, cols);
    local_sum = allocate_matrix(local_rows, cols);
    local_diff = allocate_matrix(local_rows, cols);
    local_prod = allocate_matrix(local_rows, cols);
    local_quot = allocate_matrix(local_rows, cols);
    
    int* sendcounts = NULL;
    int* displs = NULL;
    
    if (rank == 0) {
        sendcounts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        
        int offset = 0;
        for (int i = 0; i < size; ++i) {
            sendcounts[i] = (rows_per_proc + (i < remainder ? 1 : 0)) * cols;
            displs[i] = offset;
            offset += sendcounts[i];
        }
    }
    
    for (int run = 0; run < NUM_RUNS; ++run) {
        double start_time, end_time;
        
        initialize_matrix(local_arr1, local_rows, cols, run * size + rank);
        initialize_matrix(local_arr2, local_rows, cols, (run * size + rank) * 2);
        
        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();
        
        add(local_arr1, local_arr2, local_sum, local_rows, cols);
        substract(local_arr1, local_arr2, local_diff, local_rows, cols);
        multiply(local_arr1, local_arr2, local_prod, local_rows, cols);
        divide(local_arr1, local_arr2, local_quot, local_rows, cols);
        
        MPI_Barrier(MPI_COMM_WORLD);
        end_time = MPI_Wtime();
        
        if (rank == 0) {
            total_time += (end_time - start_time);
        }
    }
    
    if (rank == 0) {
        average_time = total_time / NUM_RUNS;
        printf("Average parallel execution time (over %d iterations): %f sec\n", NUM_RUNS, average_time);
        printf("Number of processes used: %d\n", size);
    }
    
    free_matrix(local_arr1, local_rows);
    free_matrix(local_arr2, local_rows);
    free_matrix(local_sum, local_rows);
    free_matrix(local_diff, local_rows);
    free_matrix(local_prod, local_rows);
    free_matrix(local_quot, local_rows);
    
    if (rank == 0) {
        free(sendcounts);
        free(displs);
    }
    
    MPI_Finalize();
    return 0;
}