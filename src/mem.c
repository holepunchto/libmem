#include <jemalloc/jemalloc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "../include/mem.h"

// When `MEM_OVERRIDE` is set, jemalloc is built without a symbol prefix so that
// it provides the standard `malloc()`, `free()`, and friends and overrides the
// system allocator; its non-standard API is then unprefixed. Otherwise, it is
// built with a `je_` prefix and does not touch the global allocator.
#ifdef MEM_OVERRIDE
#define mem__malloc             malloc
#define mem__free               free
#define mem__mallocx            mallocx
#define mem__rallocx            rallocx
#define mem__dallocx            dallocx
#define mem__mallctl            mallctl
#define mem__malloc_usable_size malloc_usable_size
#else
#define mem__malloc             je_malloc
#define mem__free               je_free
#define mem__mallocx            je_mallocx
#define mem__rallocx            je_rallocx
#define mem__dallocx            je_dallocx
#define mem__mallctl            je_mallctl
#define mem__malloc_usable_size je_malloc_usable_size
#endif

// Bookkeeping for a heap backed by a caller-provided memory region. The region
// is handed out to jemalloc through extent hooks by a simple bump allocator;
// nothing is ever returned to the operating system since the caller owns the
// memory. The `extent_hooks_t` member must come first so that jemalloc's hooks,
// which receive a pointer to it, can recover the enclosing structure with a cast.
struct mem_backing_s {
  extent_hooks_t hooks;
  uint8_t *base;
  size_t size;
  size_t offset;
  bool committed;
  bool zero;
};

struct mem_heap_s {
  unsigned arena;
  struct mem_backing_s *backing;
};

static void *
mem__extent_alloc(extent_hooks_t *hooks, void *new_addr, size_t size, size_t alignment, bool *zero, bool *commit, unsigned arena_ind) {
  struct mem_backing_s *backing = (struct mem_backing_s *) hooks;

  uintptr_t cursor = (uintptr_t) backing->base + backing->offset;
  uintptr_t aligned = (cursor + (alignment - 1)) & ~((uintptr_t) alignment - 1);

  size_t padding = (size_t) (aligned - cursor);
  size_t available = backing->size - backing->offset;

  // Signal exhaustion of the fixed region by returning `NULL`, which propagates
  // to the caller as an allocation failure.
  if (padding > available || size > available - padding) return NULL;

  void *ptr = (void *) aligned;

  // The contract requires us to return `new_addr` exactly or fail.
  if (new_addr != NULL && new_addr != ptr) return NULL;

  backing->offset += padding + size;

  *commit = backing->committed;

  // We never reuse memory within the bump allocator, so freshly handed out
  // memory is zeroed exactly when the whole region was zeroed to begin with.
  *zero = backing->zero;

  return ptr;
}

static bool
mem__extent_dalloc(extent_hooks_t *hooks, void *addr, size_t size, bool committed, unsigned arena_ind) {
  // Opt out of deallocation; the region is owned by the caller.
  return true;
}

static void
mem__extent_destroy(extent_hooks_t *hooks, void *addr, size_t size, bool committed, unsigned arena_ind) {}

static bool
mem__extent_commit(extent_hooks_t *hooks, void *addr, size_t size, size_t offset, size_t length, unsigned arena_ind) {
  // The region is always committed; report success.
  return false;
}

static bool
mem__extent_decommit(extent_hooks_t *hooks, void *addr, size_t size, size_t offset, size_t length, unsigned arena_ind) {
  // Opt out of decommitting so the region stays committed and valid.
  return true;
}

static bool
mem__extent_purge(extent_hooks_t *hooks, void *addr, size_t size, size_t offset, size_t length, unsigned arena_ind) {
  // Opt out of purging so the region stays resident.
  return true;
}

static bool
mem__extent_split(extent_hooks_t *hooks, void *addr, size_t size, size_t size_a, size_t size_b, bool committed, unsigned arena_ind) {
  // Allow splitting; it only affects jemalloc's own metadata.
  return false;
}

static bool
mem__extent_merge(extent_hooks_t *hooks, void *addr_a, size_t size_a, void *addr_b, size_t size_b, bool committed, unsigned arena_ind) {
  // Allow merging; it only affects jemalloc's own metadata.
  return false;
}

int
mem_heap_init(mem_heap_config_t *config, mem_heap_t **result) {
  mem_heap_t *heap = mem__malloc(sizeof(mem_heap_t));

  if (heap == NULL) return -1;

  heap->backing = NULL;

  unsigned arena;
  size_t len = sizeof(arena);

  if (config && config->memory) {
    struct mem_backing_s *backing = mem__malloc(sizeof(struct mem_backing_s));

    if (backing == NULL) goto err;

    backing->hooks = (extent_hooks_t) {
      .alloc = mem__extent_alloc,
      .dalloc = mem__extent_dalloc,
      .destroy = mem__extent_destroy,
      .commit = mem__extent_commit,
      .decommit = mem__extent_decommit,
      .purge_lazy = mem__extent_purge,
      .purge_forced = mem__extent_purge,
      .split = mem__extent_split,
      .merge = mem__extent_merge,
    };

    backing->base = config->memory;
    backing->size = config->size;
    backing->offset = 0;
    backing->committed = config->committed;
    backing->zero = config->zero;

    heap->backing = backing;

    extent_hooks_t *hooks = &backing->hooks;

    if (mem__mallctl("arenas.create", &arena, &len, &hooks, sizeof(hooks)) != 0) {
      mem__free(backing);

      goto err;
    }
  } else {
    if (mem__mallctl("arenas.create", &arena, &len, NULL, 0) != 0) goto err;
  }

  heap->arena = arena;

  *result = heap;

  return 0;

err:
  mem__free(heap);

  return -1;
}

void
mem_heap_destroy(mem_heap_t *heap) {
  char command[32];
  snprintf(command, sizeof(command), "arena.%u.destroy", heap->arena);

  // Discards all of the arena's live allocations at once, matching the bulk
  // free semantics of the previous mimalloc based implementation.
  mem__mallctl(command, NULL, NULL, NULL, 0);

  mem__free(heap->backing);
  mem__free(heap);
}

// Every allocation bypasses the thread cache so that destroying a heap's arena
// is always a clean bulk free; a thread cache holding an arena's extents would
// otherwise prevent the arena from being destroyed.
#define mem__flags(heap) (MALLOCX_ARENA((heap)->arena) | MALLOCX_TCACHE_NONE)

static inline int
mem__align_flags(size_t alignment) {
  return alignment > 1 ? MALLOCX_ALIGN(alignment) : 0;
}

static inline bool
mem__mul_overflows(size_t count, size_t size, size_t *result) {
  if (size != 0 && count > SIZE_MAX / size) return true;

  *result = count * size;

  return false;
}

static inline void *
mem__alloc(mem_heap_t *heap, size_t size, size_t alignment, bool zero) {
  int flags = mem__flags(heap) | mem__align_flags(alignment);

  if (zero) flags |= MALLOCX_ZERO;

  // jemalloc requires a non-zero size; hand out a minimal allocation instead.
  return mem__mallocx(size == 0 ? 1 : size, flags);
}

static inline void *
mem__realloc(mem_heap_t *heap, void *ptr, size_t size, size_t alignment, bool zero) {
  if (ptr == NULL) return mem__alloc(heap, size, alignment, zero);

  if (size == 0) {
    mem_free(ptr);

    return NULL;
  }

  int flags = mem__flags(heap) | mem__align_flags(alignment);

  if (zero) flags |= MALLOCX_ZERO;

  return mem__rallocx(ptr, size, flags);
}

void *
mem_alloc(mem_heap_t *heap, size_t size) {
  return mem__alloc(heap, size, 0, false);
}

void *
mem_alloc_aligned(mem_heap_t *heap, size_t size, size_t alignment) {
  return mem__alloc(heap, size, alignment, false);
}

void *
mem_zalloc(mem_heap_t *heap, size_t size) {
  return mem__alloc(heap, size, 0, true);
}

void *
mem_zalloc_aligned(mem_heap_t *heap, size_t size, size_t alignment) {
  return mem__alloc(heap, size, alignment, true);
}

void *
mem_calloc(mem_heap_t *heap, size_t count, size_t size) {
  size_t total;

  if (mem__mul_overflows(count, size, &total)) return NULL;

  return mem__alloc(heap, total, 0, true);
}

void *
mem_calloc_aligned(mem_heap_t *heap, size_t count, size_t size, size_t alignment) {
  size_t total;

  if (mem__mul_overflows(count, size, &total)) return NULL;

  return mem__alloc(heap, total, alignment, true);
}

void *
mem_realloc(mem_heap_t *heap, void *ptr, size_t size) {
  return mem__realloc(heap, ptr, size, 0, false);
}

void *
mem_realloc_aligned(mem_heap_t *heap, void *ptr, size_t size, size_t alignment) {
  return mem__realloc(heap, ptr, size, alignment, false);
}

void *
mem_rezalloc(mem_heap_t *heap, void *ptr, size_t size) {
  return mem__realloc(heap, ptr, size, 0, true);
}

void *
mem_rezalloc_aligned(mem_heap_t *heap, void *ptr, size_t size, size_t alignment) {
  return mem__realloc(heap, ptr, size, alignment, true);
}

void *
mem_recalloc(mem_heap_t *heap, void *ptr, size_t count, size_t size) {
  size_t total;

  if (mem__mul_overflows(count, size, &total)) return NULL;

  return mem__realloc(heap, ptr, total, 0, true);
}

void *
mem_recalloc_aligned(mem_heap_t *heap, void *ptr, size_t count, size_t size, size_t alignment) {
  size_t total;

  if (mem__mul_overflows(count, size, &total)) return NULL;

  return mem__realloc(heap, ptr, total, alignment, true);
}

size_t
mem_usable_size(const void *ptr) {
  return mem__malloc_usable_size((void *) (uintptr_t) ptr);
}

void
mem_free(void *ptr) {
  if (ptr == NULL) return;

  mem__dallocx(ptr, MALLOCX_TCACHE_NONE);
}
