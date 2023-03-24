cmake_minimum_required(VERSION 3.22)

project(mem C)

set(MEM_SECURE OFF CACHE BOOL "Use full security mitigations, such as guard pages, allocation randomization, double-free mitigation, and free-list corruption detection")

if(NOT TARGET mimalloc)
  # Make sure to force every available configuration option so mimalloc is
  # compiled exactly as we expect. Projects further up the tree can still
  # define their own mimalloc target if they wish to customize anything.
  set(MI_SECURE ${MEM_SECURE})
  set(MI_DEBUG_FULL OFF)
  set(MI_PADDING OFF)
  set(MI_OVERRIDE OFF)
  set(MI_XMALLOC OFF)
  set(MI_SHOW_ERRORS OFF)
  set(MI_VALGRIND OFF)
  set(MI_ASAN OFF)
  set(MI_USE_CXX OFF)
  set(MI_SEE_ASM OFF)
  set(MI_OSX_INTERPOSE OFF)
  set(MI_OSX_ZONE OFF)
  set(MI_WIN_REDIRECT OFF)
  set(MI_LOCAL_DYNAMIC_TLS OFF)
  set(MI_BUILD_SHARED ON)
  set(MI_BUILD_STATIC OFF)
  set(MI_BUILD_OBJECT OFF)
  set(MI_BUILD_TESTS OFF)
  set(MI_DEBUG_TSAN OFF)
  set(MI_DEBUG_UBSAN OFF)
  set(MI_SKIP_COLLECT_ON_EXIT OFF)
  set(MI_CHECK_FULL OFF)
  set(MI_INSTALL_TOPLEVEL OFF)
  set(MI_USE_LIBATOMIC OFF)

  add_subdirectory(vendor/mimalloc EXCLUDE_FROM_ALL)

  target_compile_definitions(
    mimalloc
    PRIVATE
      MI_NO_GETENV=1
      MI_DEBUG=$<IF:$<CONFIG:Debug>,1,0>
  )

  target_compile_options(
    mimalloc
    PRIVATE
      -Wno-unused-parameter
  )
endif()

add_library(mem OBJECT)

set_target_properties(
  mem
  PROPERTIES
  C_STANDARD 99
  POSITION_INDEPENDENT_CODE ON
)

target_sources(
  mem
  INTERFACE
    include/mem.h
  PRIVATE
    src/mem.c
)

target_include_directories(
  mem
  PUBLIC
    include
  PRIVATE
    $<TARGET_PROPERTY:mimalloc,INTERFACE_INCLUDE_DIRECTORIES>
)

add_library(mem_shared SHARED $<TARGET_OBJECTS:mem> $<TARGET_OBJECTS:mimalloc>)

set_target_properties(
  mem_shared
  PROPERTIES
  OUTPUT_NAME mem
  WINDOWS_EXPORT_ALL_SYMBOLS ON
)

add_library(mem_static STATIC $<TARGET_OBJECTS:mem> $<TARGET_OBJECTS:mimalloc>)

set_target_properties(
  mem_static
  PROPERTIES
  OUTPUT_NAME mem
  PREFIX lib
)

install(TARGETS mem_shared mem_static)

install(FILES include/mem.h DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(test)
endif()
