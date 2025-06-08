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

#define NUM_RUNS 100
int main() {
  char *array_size_str = getenv("ARRAY_SIZE");
  if (array_size_str == NULL) {
    fprintf(stderr, "Переменная окружения ARRAY_SIZE не установлена.\n");
    return 1;
  }

  int array_size = atoi(array_size_str);

  if (array_size <= 0) {
    fprintf(stderr, "Некорректное значение ARRAY_SIZE.  Должно быть положительным целым числом.\n");
    return 1;
  }

  int *arr = (int *)malloc(array_size * sizeof(int));
  if (arr == NULL) {
    fprintf(stderr, "Не удалось выделить память для массива.\n");
    return 1;
  }
  
  srand(time(NULL));
  fillArrayWithRandomNumbers(arr, array_size);
  clock_t start_time = clock();
  bubbleSort(arr, array_size);
  clock_t end_time = clock();
  double time_taken = (double)(end_time - start_time) / CLOCKS_PER_SEC;
  printf("Время, затраченное на сортировку: %f секунд\n", time_taken);
  free(arr);
  return 0;
}
