# libmem

General purpose memory allocator for C built on <https://github.com/jemalloc/jemalloc>. It's built around explicit heaps and provides no global allocation outside of [overriding the system allocator](#overriding-the-system-allocator).

## API

See [`include/mem.h`](include/mem.h) for the public API.

### Overriding the system allocator

By default, libmem does not touch the global allocator; jemalloc is built with a `je_` symbol prefix and is driven only through the explicit heap API.

Configure with `MEM_OVERRIDE=ON` to instead have jemalloc provide the standard `malloc()`, `free()`, and friends and override the system allocator process-wide. This routes every allocation through jemalloc, including those made by code loaded at runtime, such as native addons opened with `dlopen()`:

- On Linux, jemalloc's symbols interpose through the global dynamic symbol scope.
- On macOS, jemalloc installs itself as the default malloc zone.
- On Windows, overriding the system allocator is not supported.

The consumer must link the jemalloc objects with whole-archive semantics so that the macOS zone registration, which is pulled in through a constructor, is not dropped from the static archive.

Set `MEM_OVERRIDE_CXX=ON`, which requires `MEM_OVERRIDE`, to additionally override the C++ `operator new` and `operator delete` directly. This links the C++ runtime, so it is off by default; without it, C++ allocations are still served by jemalloc transitively through the overridden `malloc()` and `free()`.

## License

Apache-2.0
