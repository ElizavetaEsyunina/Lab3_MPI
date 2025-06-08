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

int* mergeArrays(int arr1[], int n1, int arr2[], int n2) {
    int *merged = (int*)malloc((n1 + n2) * sizeof(int));
    int i = 0, j = 0, k = 0;

    while (i < n1 && j < n2) {
        if (arr1[i] <= arr2[j]) {
            merged[k++] = arr1[i++];
        } else {
            merged[k++] = arr2[j++];
        }
    }

    while (i < n1) {
        merged[k++] = arr1[i++];
    }

    while (j < n2) {
        merged[k++] = arr2[j++];
    }

    return merged;
}

int main(int argc, char *argv[]) {
    int rank, size;
    int array_size;
    int *arr = NULL;
    double start_time, end_time, time_taken;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        char *array_size_str = getenv("ARRAY_SIZE");
        if (array_size_str == NULL) {
            fprintf(stderr, "Переменная окружения ARRAY_SIZE не установлена.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        array_size = atoi(array_size_str);

        if (array_size <= 0) {
            fprintf(stderr, "Некорректное значение ARRAY_SIZE.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        arr = (int *)malloc(array_size * sizeof(int));
        if (arr == NULL) {
            fprintf(stderr, "Не удалось выделить память для массива.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        srand(time(NULL));
        fillArrayWithRandomNumbers(arr, array_size);
    }

    MPI_Bcast(&array_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int local_size = array_size / size;
    int *local_arr = (int *)malloc(local_size * sizeof(int));
    if (local_arr == NULL) {
      fprintf(stderr, "Не удалось выделить память для локального массива.\n", rank);
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Scatter(arr, local_size, MPI_INT, local_arr, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
      start_time = MPI_Wtime();
    }

    bubbleSort(local_arr, local_size);

    int *sorted_arr = NULL;
    if (rank == 0) {
      sorted_arr = (int *)malloc(array_size * sizeof(int));
      if (sorted_arr == NULL) {
        fprintf(stderr, "Не удалось выделить память для результирующего массива.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
      }
    }
    MPI_Gather(local_arr, local_size, MPI_INT, sorted_arr, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
      int* merged_arr = sorted_arr;
      int merged_size = local_size;

      for (int i = 1; i < size; ++i) {
          int* next_arr = sorted_arr + i * local_size;
          int* temp_merged = mergeArrays(merged_arr, merged_size, next_arr, local_size);
          free(merged_arr);
          merged_arr = temp_merged;
          merged_size += local_size;
      }
      free(sorted_arr);
      end_time = MPI_Wtime();
      time_taken = end_time - start_time;
      printf("Время, затраченное на сортировку: %f секунд\n", time_taken);
      printf("Количество процессов: %d\n", size);

      free(merged_arr);
      free(arr);

    }


    free(local_arr);

    MPI_Finalize();
    return 0;
}
