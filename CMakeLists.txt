cmake_minimum_required(VERSION 3.22)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(WIN32)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

project(mem C)

add_library(mem OBJECT)

set_target_properties(
  mem
  PROPERTIES
  C_STANDARD 99
  POSITION_INDEPENDENT_CODE 1
)

target_sources(
  mem
  INTERFACE
    include/mem.h
  PRIVATE
    src/mem.c
    vendor/rpmalloc/rpmalloc/rpmalloc.h
    vendor/rpmalloc/rpmalloc/rpmalloc.c
)

target_include_directories(
  mem
  PUBLIC
    include
  PRIVATE
    vendor/rpmalloc/rpmalloc
)

add_library(mem_shared SHARED $<TARGET_OBJECTS:mem>)

set_target_properties(
  mem_shared
  PROPERTIES
  OUTPUT_NAME mem
)

add_library(mem_static STATIC $<TARGET_OBJECTS:mem>)

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
