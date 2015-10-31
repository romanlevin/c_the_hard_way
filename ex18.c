#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/** Our old friend die from ex17. */
void die(const char *message)
{
        if(errno) {
                perror(message);
        } else {
                printf("ERROR: %s\n", message);
        }

        exit(1);
}

// a typedef creates a fake type, in this
// case for a function pointer
typedef int (*compare_cb)(int a, int b);
typedef void (*sorting_function)(int *numbers, int count, compare_cb cmp);

int *copy_numbers(int *numbers, int count) {
        int *target = malloc(count * sizeof(int));

        if(!target) die("Memory error.");

        memcpy(target, numbers, count * sizeof(int));

        return target;
}

void compare_and_swap(int *numbers, compare_cb cmp)
{
        if(cmp(numbers[0], numbers[1]) > 0) {
                int temp = numbers[1];
                numbers[1] = numbers[0];
                numbers[0] = temp;
        }
}

/**
 * A classic bubble sort function that uses the
 * compare_cb to do the sorting.
 */
void bubble_sort(int *numbers, int count, compare_cb cmp)
{
        for(int i = 0; i < count; i++) {
                for(int j = 0; j < count - 1; j++) {
                        compare_and_swap(&numbers[j], cmp);
                }
        }
}

void merge_sort(int *numbers, int count, compare_cb cmp)
{
        if(count == 1) return;
        if(count == 2) {
                compare_and_swap(numbers, cmp);
                return;
        }
        int middle = count / 2;
        merge_sort(numbers, middle, cmp);
        merge_sort(&numbers[middle], count - middle, cmp);
        int *work_array = malloc(sizeof(int) * count);
        memcpy(work_array, numbers, count * sizeof(*work_array));
        for(int i = 0, a = 0, b = middle; i < count; i++) {
                if(b < count && (a >= middle || cmp(work_array[a], work_array[b]) > 0)) {
                        numbers[i] = work_array[b];
                        b++;
                } else {
                        numbers[i] = work_array[a];
                        a++;
                }
        }
        free(work_array);
}

void quick_sort(int *numbers, int count, compare_cb cmp)
{
        int pivot = 0;
        for(int i = pivot + 1; i < count; i++) {
                if(cmp(numbers[pivot], numbers[i]) > 0) {
                        int temp = numbers[i];
                        numbers[i] = numbers[pivot];
                        numbers[pivot] = temp;
                        pivot = i;
                }
        }
}

int sorted_order(int a, int b)
{
        return a - b;
}

int reverse_order(int a, int b)
{
        return b - a;
}

int strange_order(int a, int b)
{
        if(a == 0 || b == 0) {
                return 0;
        } else {
                return a % b;
        }
}

/**
 * Used to test that we are sorting things correctly
 * by doing the sort and printing it out.
 */
void test_sorting(int *numbers, int count, compare_cb cmp, sorting_function sort)
{
        int *target = copy_numbers(numbers, count);
        sort(target, count, cmp);

        for(int i = 0; i < count; i++) {
                printf("%d ", target[i]);
        }
        printf("\n");

        free(target);
}

int main(int argc, char *argv[])
{
      if (argc < 2) die("USAGE: ex18 4 3 1 5 6");

      int count = argc - 1;
      char **inputs = argv + 1;

      int *numbers = malloc(count * sizeof(int));
      if(!numbers) die("Memory error.");

      for(int i = 0; i < count; i++) {
              numbers[i] = atoi(inputs[i]);
      }

      printf("Bubble Sort\n");
      test_sorting(numbers, count, sorted_order, bubble_sort);
      test_sorting(numbers, count, reverse_order, bubble_sort);
      test_sorting(numbers, count, strange_order, bubble_sort);
      printf("\nMerge Sort\n");
      test_sorting(numbers, count, sorted_order, merge_sort);
      test_sorting(numbers, count, reverse_order, merge_sort);
      test_sorting(numbers, count, strange_order, merge_sort);
      free(numbers);

      return 0;
}

