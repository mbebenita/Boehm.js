#include "gc.h"
#include <assert.h>
#include <stdio.h>

int main() {
  GC_INIT();

  for (int i = 0; i < 10000; ++i) {
    int size = i % 100;

    int *p = (int*)GC_MALLOC_UNCOLLECTABLE(sizeof(int) * size);

    //printf("Step %d - Allocated: %d - At: %x\n", i, size, p);

    for (int j = 0; j < size; j++) {
      assert(p[j] == 0);
      p[j] = j;
      assert(p[j] == j);
    }

    if (i % 1000 == 0) {
      printf("Heap size = %d\n", GC_get_heap_size());
    }
  }

  return 0;
}