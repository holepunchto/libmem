cmake_minimum_required(VERSION 4.0)

find_package(cmake-ports REQUIRED PATHS node_modules/cmake-ports)

project(mem C)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

find_port(jemalloc)

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
)

target_link_libraries(
  mem
  PUBLIC
    jemalloc
)

add_library(mem_shared SHARED)

set_target_properties(
  mem_shared
  PROPERTIES
  OUTPUT_NAME mem
  WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_link_libraries(
  mem_shared
  PUBLIC
    mem
)

add_library(mem_static STATIC)

set_target_properties(
  mem_static
  PROPERTIES
  OUTPUT_NAME mem
  PREFIX lib
)

target_link_libraries(
  mem_static
  PUBLIC
    mem
)

install(TARGETS mem_shared mem_static)

install(FILES include/mem.h DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()

  add_subdirectory(test)
endif()
