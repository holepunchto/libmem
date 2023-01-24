#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/mem.h"

bool map_called = false;
bool unmap_called = false;

void *
map (size_t size, size_t *offset) {
  map_called = true;

  return malloc(size);
}

void
unmap (void *address, size_t size, size_t offset, size_t release) {
  unmap_called = true;

  if (release) free(address);
}

int
main () {
  int e;

  mem_config_t config = {
    .map = map,
    .unmap = unmap,
  };

  e = mem_init(&config);
  assert(e == 0);

  void *ptr = mem_alloc(10);

  assert(map_called);

  e = mem_destroy();
  assert(e == 0);

  assert(unmap_called);
}
