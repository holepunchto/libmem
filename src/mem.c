#include <mimalloc.h>
#include <stdbool.h>
#include <stddef.h>

#include "../include/mem.h"

struct mem_heap_s {
  mi_heap_t *heap;
};

struct mem_arena_s {
  mi_arena_id_t id;
};

int
mem_heap_init (mem_heap_config_t *config, mem_heap_t **result) {
  mem_heap_t *heap = mi_malloc(sizeof(mem_heap_t));

  if (heap == NULL) return -1;

  if (config && config->arena) {
    heap->heap = mi_heap_new_in_arena(config->arena->id);
  } else {
    heap->heap = mi_heap_new();
  }

  if (heap->heap == NULL) goto err;

  *result = heap;

  return 0;

err:
  mi_free(heap);

  return -1;
}

void
mem_heap_destroy (mem_heap_t *heap) {
  mi_heap_destroy(heap->heap);
  mi_free(heap);
}

int
mem_arena_init (void *memory, size_t size, mem_arena_config_t *config, mem_arena_t **result) {
  mem_arena_t *arena = mi_malloc(sizeof(mem_arena_t));

  if (arena == NULL) return -1;

  bool success = mi_manage_os_memory_ex(
    memory,
    size,
    config ? config->committed : true /* is_committed */,
    config ? config->large : false /* is_large */,
    config ? config->zero : true /* is_zero */,
    -1,
    true /* is_exclusive */,
    &arena->id
  );

  if (!success) goto err;

  *result = arena;

  return 0;

err:
  mi_free(arena);

  return -1;
}

void
mem_arena_destroy (mem_arena_t *arena) {
  mi_free(arena);
}

void *
mem_alloc (mem_heap_t *heap, size_t size) {
  return mi_heap_malloc(heap->heap, size);
}

void *
mem_alloc_aligned (mem_heap_t *heap, size_t size, size_t alignment) {
  return mi_heap_malloc_aligned(heap->heap, size, alignment);
}

void *
mem_zalloc (mem_heap_t *heap, size_t size) {
  return mi_heap_zalloc(heap->heap, size);
}

void *
mem_zalloc_aligned (mem_heap_t *heap, size_t size, size_t alignment) {
  return mi_heap_zalloc_aligned(heap->heap, size, alignment);
}

void *
mem_calloc (mem_heap_t *heap, size_t count, size_t size) {
  return mi_heap_calloc(heap->heap, count, size);
}

void *
mem_calloc_aligned (mem_heap_t *heap, size_t count, size_t size, size_t alignment) {
  return mi_heap_calloc_aligned(heap->heap, count, size, alignment);
}

void *
mem_realloc (mem_heap_t *heap, void *ptr, size_t size) {
  return mi_heap_realloc(heap->heap, ptr, size);
}

void *
mem_realloc_aligned (mem_heap_t *heap, void *ptr, size_t size, size_t alignment) {
  return mi_heap_realloc_aligned(heap->heap, ptr, size, alignment);
}

void *
mem_rezalloc (mem_heap_t *heap, void *ptr, size_t size) {
  return mi_heap_rezalloc(heap->heap, ptr, size);
}

void *
mem_rezalloc_aligned (mem_heap_t *heap, void *ptr, size_t size, size_t alignment) {
  return mi_heap_rezalloc_aligned(heap->heap, ptr, size, alignment);
}

void *
mem_recalloc (mem_heap_t *heap, void *ptr, size_t count, size_t size) {
  return mi_heap_recalloc(heap->heap, ptr, count, size);
}

void *
mem_recalloc_aligned (mem_heap_t *heap, void *ptr, size_t count, size_t size, size_t alignment) {
  return mi_heap_recalloc_aligned(heap->heap, ptr, count, size, alignment);
}

size_t
mem_usable_size (const void *ptr) {
  return mi_usable_size(ptr);
}

void
mem_free (void *ptr) {
  mi_free(ptr);
}
