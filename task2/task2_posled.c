#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

int main() {
  char *array_size_str = getenv("ARRAY_SIZE");
  char *num_iterations_str = getenv("NUM_ITERATIONS");
  char *endptr; // Для strtol

  if (array_size_str == NULL) {
    fprintf(stderr, "Переменная окружения ARRAY_SIZE не установлена.\n");
    return 1;
  }

  if (num_iterations_str == NULL) {
    fprintf(stderr, "Переменная окружения NUM_ITERATIONS не установлена.\n");
    return 1;
  }
  
  int array_size = atoi(array_size_str);
  int num_iterations = strtol(num_iterations_str, &endptr, 10);

  if (array_size <= 0) {
    fprintf(stderr, "Некорректное значение ARRAY_SIZE.  Должно быть положительным целым числом.\n");
    return 1;
  }

  if (*endptr != '\0' || num_iterations <= 0) {
    fprintf(stderr, "Некорректное значение NUM_ITERATIONS. Должно быть положительным целым числом.\n");
    return 1;
  }

  int *arr = (int *)malloc(array_size * sizeof(int));
  if (arr == NULL) {
    fprintf(stderr, "Не удалось выделить память для массива.\n");
    return 1;
  }
  
  srand(time(NULL));

  double total_time = 0.0;
  for (int i = 0; i < num_iterations; i++) {
    fillArrayWithRandomNumbers(arr, array_size);
    clock_t start_time = clock();
    bubbleSort(arr, array_size);
    clock_t end_time = clock();
    double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    total_time += time_taken;

    printf("Итерация %d: Время, затраченное на сортировку: %f секунд\n", i + 1, time_taken);
  }

  double average_time = total_time / num_iterations;
  printf("Среднее время, затраченное на сортировку: %f секунд\n", average_time);

  free(arr);
  return 0;
}
