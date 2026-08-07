#include <assert.h>

#include "../include/mem.h"

#define ARENA_SIZE      (128 * 1024 * 1024)
#define ARENA_ALIGNMENT (2 * 1024 * 1024)

int
main() {
  int e;

  mem_heap_t *global_heap;
  e = mem_heap_init(NULL, &global_heap);
  assert(e == 0);

  void *mem = mem_zalloc_aligned(global_heap, ARENA_SIZE, ARENA_ALIGNMENT);
  assert(mem != NULL);

  mem_heap_t *heap;
  e = mem_heap_init(&(mem_heap_config_t) {.memory = mem, .size = ARENA_SIZE, .zero = true}, &heap);
  assert(e == 0);

  void *ptr = mem_alloc(heap, 1024);
  assert(ptr != NULL);

  mem_free(ptr);

  mem_heap_destroy(heap);

  mem_free(mem);

  mem_heap_destroy(global_heap);
}
