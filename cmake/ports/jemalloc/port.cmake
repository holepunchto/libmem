include_guard(GLOBAL)

set(args
  # Prefix every exported symbol with `je_` so that jemalloc does not override
  # the global allocator; we drive it explicitly through `je_mallocx()` and
  # friends.
  --with-jemalloc-prefix=je_

  # We only use the C API.
  --disable-cxx

  # jemalloc derives its version from `git describe`, which fails on the shallow
  # tag checkout that the port performs. Pin it explicitly.
  --with-version=5.3.0-0-g0000000000000000000000000000000000000000
)

declare_port(
  "github:jemalloc/jemalloc#5.3.0"
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
