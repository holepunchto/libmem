#ifndef MEM_H
#define MEM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mem_config_s mem_config_t;

typedef void *(*mem_map_cb)(size_t size, size_t *offset);
typedef void (*mem_unmap_cb)(void *address, size_t size, size_t offset, size_t release);

struct mem_config_s {
  mem_map_cb map;
  mem_unmap_cb unmap;
  size_t page_size;
};

int
mem_init (const mem_config_t *config);

int
mem_destroy ();

int
mem_thread_init ();

int
mem_thread_destroy ();

void *
mem_alloc (size_t size);

void *
mem_aligned_alloc (size_t alignment, size_t size);

void *
mem_calloc (size_t count, size_t size);

void *
mem_aligned_calloc (size_t alignment, size_t count, size_t size);

void *
mem_realloc (void *ptr, size_t size);

void *
mem_aligned_realloc (void *ptr, size_t alignment, size_t size);

void
mem_free (void *ptr);

#ifdef __cplusplus
}
#endif

#endif // MEM_H
