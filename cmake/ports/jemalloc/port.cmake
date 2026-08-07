include_guard(GLOBAL)

set(version 5.3.0)

set(args
  # jemalloc derives its version from `git describe`, which fails on the shallow
  # tag checkout that the port performs. Pin it explicitly.
  --with-version=${version}-0-g0000000000000000000000000000000000000000
)

if("cxx" IN_LIST features)
  # Build jemalloc's C++ integration so that it also defines `operator new` and
  # `operator delete`. Their mangled names cannot be prefixed, so this is only
  # meaningful together with the global allocator override.
else()
  # We only use the C API.
  list(APPEND args --disable-cxx)
endif()

if("override" IN_LIST features)
  # Build without a symbol prefix so that jemalloc provides the standard
  # `malloc()`, `free()`, and friends and overrides the system allocator. On
  # Darwin this additionally installs jemalloc as the default malloc zone
  # (`--enable-zone-allocator`, on by default), which is what routes
  # allocations made by dynamically loaded code, such as native addons, through
  # jemalloc.
  list(APPEND args --with-jemalloc-prefix=)
else()
  # Prefix every exported symbol with `je_` so that jemalloc does not override
  # the global allocator; we drive it explicitly through `je_mallocx()` and
  # friends.
  list(APPEND args --with-jemalloc-prefix=je_)
endif()

declare_port(
  "github:jemalloc/jemalloc#${version}"
  jemalloc
  AUTOTOOLS
  ENTRYPOINT "${CMAKE_CURRENT_LIST_DIR}/autogen.sh"
  BYPRODUCTS lib/libjemalloc.a
  ARGS ${args}
)

add_library(jemalloc STATIC IMPORTED GLOBAL)

add_dependencies(jemalloc ${jemalloc})

set_target_properties(
  jemalloc
  PROPERTIES
  IMPORTED_LOCATION "${jemalloc_PREFIX}/lib/libjemalloc.a"
)

if("cxx" IN_LIST features)
  # jemalloc's C++ integration references the C++ runtime, so anything linking
  # the archive must link it too.
  set_target_properties(
    jemalloc
    PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES CXX
  )
endif()

file(MAKE_DIRECTORY "${jemalloc_PREFIX}/include")

target_include_directories(
  jemalloc
  INTERFACE "${jemalloc_PREFIX}/include"
)

if(WIN32)
  target_link_libraries(
    jemalloc
    INTERFACE
      psapi
  )
else()
  target_link_libraries(
    jemalloc
    INTERFACE
      Threads::Threads
      ${CMAKE_DL_LIBS}
  )

  if(NOT APPLE)
    target_link_libraries(
      jemalloc
      INTERFACE
        rt
    )
  endif()
endif()
