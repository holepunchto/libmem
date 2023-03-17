cmake_minimum_required(VERSION 3.22)

project(mem C)

option(MEM_SECURE OFF)

set(MI_OVERRIDE OFF)
set(MI_SECURE ${MEM_SECURE})
set(MI_BUILD_STATIC OFF)
set(MI_BUILD_OBJECT OFF)
set(MI_BUILD_TESTS OFF)

if(NOT TARGET mimalloc)
  add_subdirectory(vendor/mimalloc EXCLUDE_FROM_ALL)
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
)

install(TARGETS mem_shared mem_static)

install(FILES include/mem.h DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(test)
endif()
