#include <assert.h>
#include <stdio.h>

#include "../include/mem.h"

#define ARENA_SIZE (128 * 1024 * 1024)

int
main () {
  int e;

  mem_heap_t *global_heap;
  e = mem_heap_init(NULL, &global_heap);
  assert(e == 0);

  void *mem = mem_zalloc_aligned(global_heap, ARENA_SIZE, MEM_ARENA_ALIGNMENT);
  assert(mem != NULL);

  mem_arena_t *arena;
  e = mem_arena_init(mem, ARENA_SIZE, NULL, &arena);

  mem_heap_t *heap;
  e = mem_heap_init(&(mem_heap_config_t){.arena = arena}, &heap);
  assert(e == 0);

  void *ptr = mem_alloc(heap, 1024);
  assert(ptr != NULL);

  mem_free(ptr);

  mem_heap_destroy(heap);
  mem_heap_destroy(global_heap);
}
