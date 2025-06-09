#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

void bubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void fillArrayWithRandomNumbers(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = rand();
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int array_size;
    int *arr = NULL;
    int num_iterations;
    double start_time, end_time, time_taken, total_time = 0.0, average_time;
    char *endptr;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        char *array_size_str = getenv("ARRAY_SIZE");
        if (array_size_str == NULL) {
            fprintf(stderr, "Переменная ARRAY_SIZE не установлена.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        array_size = atoi(array_size_str);

        if (array_size <= 0) {
            fprintf(stderr, "Некорректное значение ARRAY_SIZE.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        char *num_iterations_str = getenv("NUM_ITERATIONS");
        if (num_iterations_str == NULL) {
            fprintf(stderr, "Переменная NUM_ITERATIONS не установлена.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        num_iterations = strtol(num_iterations_str, &endptr, 10);
        if (*endptr != '\0' || num_iterations <= 0) {
            fprintf(stderr, "Некорректное значение NUM_ITERATIONS: %s\n", num_iterations_str);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        arr = (int *)malloc(array_size * sizeof(int));
        if (arr == NULL) {
            fprintf(stderr, "Не удалось выделить память под массив.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        srand(time(NULL));

        for (int i = 0; i < num_iterations; i++) {
            fillArrayWithRandomNumbers(arr, array_size);
            start_time = MPI_Wtime();
            bubbleSort(arr, array_size);
            end_time = MPI_Wtime();
            time_taken = end_time - start_time;
            total_time += time_taken;
            printf("Итерация %d: \nВремя, затраченное на сортировку: %f секунд\n", i + 1, time_taken);
        }

        average_time = total_time / num_iterations;

        printf("Среднее время, затраченное на сортировку: %f секунд\n", average_time);
        printf("Количество процессов: %d\n", size);

        free(arr);
    }

    MPI_Finalize();
    return 0;
}
