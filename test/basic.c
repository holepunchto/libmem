#include <assert.h>

#include "../include/mem.h"

int
main () {
  int e;

  mem_heap_t *heap;
  e = mem_heap_init(NULL, &heap);
  assert(e == 0);

  void *ptr = mem_alloc(heap, 1024);
  assert(ptr != NULL);

  mem_free(ptr);

  mem_heap_destroy(heap);
}
