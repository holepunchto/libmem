#ifndef MEM_H
#define MEM_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mem_heap_s mem_heap_t;
typedef struct mem_heap_config_s mem_heap_config_t;

struct mem_heap_config_s {
  /**
   * The optional memory backing the heap. When `NULL`, the heap is backed by
   * memory obtained from the operating system. When non-`NULL`, all allocations
   * made from the heap are served from this region, which must remain valid for
   * the lifetime of the heap and is owned by the caller.
   */
  void *memory;

  /**
   * The size of the memory backing the heap. Only read when `memory` is non-`NULL`.
   */
  size_t size;

  /**
   * `true` if the memory backing the heap is already committed, otherwise `false`.
   */
  bool committed;

  /**
   * `true` if the memory backing the heap uses large OS pages, otherwise `false`.
   */
  bool large;

  /**
   * `true` if the memory backing the heap has already been zero'ed, otherwise `false`.
   */
  bool zero;
};

int
mem_heap_init(mem_heap_config_t *config, mem_heap_t **result);

void
mem_heap_destroy(mem_heap_t *heap);

void *
mem_alloc(mem_heap_t *heap, size_t size);

void *
mem_alloc_aligned(mem_heap_t *heap, size_t size, size_t alignment);

void *
mem_zalloc(mem_heap_t *heap, size_t size);

void *
mem_zalloc_aligned(mem_heap_t *heap, size_t size, size_t alignment);

void *
mem_calloc(mem_heap_t *heap, size_t count, size_t size);

void *
mem_calloc_aligned(mem_heap_t *heap, size_t count, size_t size, size_t alignment);

void *
mem_realloc(mem_heap_t *heap, void *ptr, size_t size);

void *
mem_realloc_aligned(mem_heap_t *heap, void *ptr, size_t size, size_t alignment);

void *
mem_rezalloc(mem_heap_t *heap, void *ptr, size_t size);

void *
mem_rezalloc_aligned(mem_heap_t *heap, void *ptr, size_t size, size_t alignment);

void *
mem_recalloc(mem_heap_t *heap, void *ptr, size_t count, size_t size);

void *
mem_recalloc_aligned(mem_heap_t *heap, void *ptr, size_t count, size_t size, size_t alignment);

size_t
mem_usable_size(const void *ptr);

void
mem_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif // MEM_H
