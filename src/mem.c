#define RPMALLOC_CONFIGURABLE 1

#include <rpmalloc.h>
#include <stdbool.h>
#include <stddef.h>

#include "../include/mem.h"

int
mem_init (const mem_config_t *config) {
  if (config == NULL) return rpmalloc_initialize();

  return rpmalloc_initialize_config(&(rpmalloc_config_t){
    .memory_map = config->map,
    .memory_unmap = config->unmap,
    .page_size = config->page_size,
  });
}

int
mem_destroy () {
  rpmalloc_finalize();

  return 0;
}

int
mem_thread_init () {
  rpmalloc_thread_initialize();

  return 0;
}

int
mem_thread_destroy () {
  rpmalloc_thread_finalize(true);

  return 0;
}

void *
mem_alloc (size_t size) {
  return rpmalloc(size);
}

void *
mem_aligned_alloc (size_t alignment, size_t size) {
  return rpaligned_alloc(alignment, size);
}

void *
mem_calloc (size_t count, size_t size) {
  return rpcalloc(count, size);
}

void *
mem_aligned_calloc (size_t alignment, size_t count, size_t size) {
  return rpaligned_calloc(alignment, count, size);
}

void *
mem_realloc (void *ptr, size_t size) {
  return rprealloc(ptr, size);
}

void *
mem_aligned_realloc (void *ptr, size_t alignment, size_t size) {
  return rpaligned_realloc(ptr, alignment, size, 0, 0);
}

size_t
mem_usable_size (const void *ptr) {
  return rpmalloc_usable_size((void *) ptr);
}

void
mem_free (void *ptr) {
  rpfree(ptr);
}
