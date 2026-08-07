include_guard(GLOBAL)

set(version 5.3.0)

set(args
  # jemalloc derives its version from `git describe`, which fails on the shallow
  # tag checkout that the port performs. Pin it explicitly.
  --with-version=${version}-0-g0000000000000000000000000000000000000000

  # jemalloc's static archive is built from non-position-independent objects by
  # default, which cannot be linked into a shared library. Match libmem's
  # position-independent code by adding the PIC flag to every object. jemalloc
  # appends `EXTRA_CFLAGS` to its own flags, so this leaves its optimization
  # settings intact.
  EXTRA_CFLAGS=${CMAKE_C_COMPILE_OPTIONS_PIC}
)

if("cxx" IN_LIST features)
  # Build jemalloc's C++ integration so that it also defines `operator new` and
  # `operator delete`. Their mangled names cannot be prefixed, so this is only
  # meaningful together with the global allocator override.
  list(APPEND args EXTRA_CXXFLAGS=${CMAKE_C_COMPILE_OPTIONS_PIC})
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

set(env)

if(CMAKE_C_COMPILER)
  cmake_path(GET CMAKE_C_COMPILER PARENT_PATH cc_directory)
  cmake_path(GET CMAKE_C_COMPILER FILENAME cc_filename)

  list(APPEND env "CC=${cc_filename}")
  list(APPEND env --modify "PATH=path_list_prepend:${cc_directory}")

  if(CMAKE_C_COMPILER_TARGET)
    list(APPEND env "CFLAGS=--target=${CMAKE_C_COMPILER_TARGET}")
    list(APPEND env "LDFLAGS=--target=${CMAKE_C_COMPILER_TARGET}")
  endif()
endif()

if(WIN32)
  set(lib lib/jemalloc_s.lib)
else()
  set(lib lib/libjemalloc.a)
endif()

declare_port(
  "github:jemalloc/jemalloc#${version}"
  jemalloc
  AUTOTOOLS
  ENTRYPOINT "${CMAKE_CURRENT_LIST_DIR}/autogen.sh"
  BYPRODUCTS ${lib}
  ARGS ${args}
  ENV ${env}
  PATCHES
    patches/01-install-sh-verbose.patch
)

add_library(jemalloc STATIC IMPORTED GLOBAL)

add_dependencies(jemalloc ${jemalloc})

set_target_properties(
  jemalloc
  PROPERTIES
  IMPORTED_LOCATION "${jemalloc_PREFIX}/${lib}"
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

if(MSVC)
  # jemalloc's public header relies on POSIX headers that MSVC does not provide.
  # jemalloc ships shims under msvc_compat and puts them on the include path for
  # its own build; do the same so the header compiles for consumers. The
  # directory is created up front so the imported target validates before the
  # source has been fetched, mirroring the install include directory above.
  file(MAKE_DIRECTORY "${jemalloc_SOURCE_DIR}/include/msvc_compat")

  target_include_directories(
    jemalloc
    INTERFACE "${jemalloc_SOURCE_DIR}/include/msvc_compat"
  )

  # We link jemalloc as a static archive, but its public header declares the API
  # as `__declspec(dllimport)` on MSVC unless told otherwise, which leaves the
  # references unresolved at link time. Define `JEMALLOC_EXPORT` empty so the
  # declarations match the static archive.
  target_compile_definitions(
    jemalloc
    INTERFACE JEMALLOC_EXPORT=
  )
endif()

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
