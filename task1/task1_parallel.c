#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define MAX_VALUE 1000
#define NUM_RUNS 100

int main(int argc, char** argv) {
    int array_size;
    int num_processes;
    int rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        char* array_size_str = getenv("ARRAY_SIZE");
        if (array_size_str == NULL) {
            fprintf(stderr, "Переменная окружения ARRAY_SIZE не установлена\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        char* endptr;
        array_size = strtol(array_size_str, &endptr, 10);
        if (*endptr != '\0' || array_size <= 0) {
            fprintf(stderr, "Некорректное значение переменной окружения ARRAY_SIZE: %s\n", array_size_str);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
    }

    MPI_Bcast(&array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);


    double total_time = 0.0;

    for (int run = 0; run < NUM_RUNS; run++) {
        int* local_array = NULL;
        int local_size;
        long long local_sum = 0;
        double start_time, end_time;


        local_size = array_size / num_processes;
        if (rank < array_size % num_processes) {
            local_size++;
        }

        local_array = (int*)malloc(local_size * sizeof(int));
        if (local_array == NULL) {
            fprintf(stderr, "Ошибка выделения памяти для процесса %d\n", rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }

        srand(time(NULL) + run * num_processes + rank);

        int global_index = 0;
        int offset = 0;
        for (int i = 0; i < rank; i++) {
            offset += array_size / num_processes;
            if (i < array_size % num_processes) {
                offset++;
            }
        }

        for (int i = 0; i < local_size; i++) {
            local_array[i] = rand() % MAX_VALUE;
        }


        MPI_Barrier(MPI_COMM_WORLD);
        start_time = MPI_Wtime();

        for (int i = 0; i < local_size; i++) {
            local_sum += local_array[i];
        }

        end_time = MPI_Wtime();
        double time_spent = end_time - start_time;

        long long global_sum;
        MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            total_time += time_spent;
        }

        free(local_array);
        local_array = NULL;
    }

    if (rank == 0) {
        double average_time = total_time / NUM_RUNS;
        printf("Среднее время выполнения: %f секунд\n", average_time);
        printf("Количество потоков: %d\n", num_processes);
    }

    MPI_Finalize();
    return 0;
}