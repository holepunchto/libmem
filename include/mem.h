#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if INTPTR_MAX > INT32_MAX
#define MEM_ARENA_ALIGNMENT (32 * 1024 * 1024)
#else
#define MEM_ARENA_ALIGNMENT (4 * 1024 * 1024)
#endif

typedef struct mem_heap_s mem_heap_t;
typedef struct mem_arena_s mem_arena_t;

int
mem_heap_init (mem_arena_t *arena, mem_heap_t **result);

void
mem_heap_destroy (mem_heap_t *heap);

int
mem_arena_init (void *memory, size_t size, mem_arena_t **result);

void
mem_arena_destroy (mem_arena_t *arena);

void *
mem_alloc (mem_heap_t *heap, size_t size);

void *
mem_alloc_aligned (mem_heap_t *heap, size_t size, size_t alignment);

void *
mem_zalloc (mem_heap_t *heap, size_t size);

void *
mem_zalloc_aligned (mem_heap_t *heap, size_t size, size_t alignment);

void *
mem_calloc (mem_heap_t *heap, size_t count, size_t size);

void *
mem_calloc_aligned (mem_heap_t *heap, size_t count, size_t size, size_t alignment);

void *
mem_realloc (mem_heap_t *heap, void *ptr, size_t size);

void *
mem_realloc_aligned (mem_heap_t *heap, void *ptr, size_t size, size_t alignment);

void *
mem_rezalloc (mem_heap_t *heap, void *ptr, size_t size);

void *
mem_rezalloc_aligned (mem_heap_t *heap, void *ptr, size_t size, size_t alignment);

void *
mem_recalloc (mem_heap_t *heap, void *ptr, size_t count, size_t size);

void *
mem_recalloc_aligned (mem_heap_t *heap, void *ptr, size_t count, size_t size, size_t alignment);

size_t
mem_usable_size (const void *ptr);

void
mem_free (void *ptr);

#ifdef __cplusplus
}
#endif

#endif // MEM_H
