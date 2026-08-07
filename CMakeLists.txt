cmake_minimum_required(VERSION 4.0)

find_package(cmake-ports REQUIRED PATHS node_modules/cmake-ports)

project(mem C)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

option(MEM_OVERRIDE "Override the system allocator so that all malloc()/free() calls, including those made by dynamically loaded code, are served by jemalloc" OFF)

option(MEM_OVERRIDE_CXX "Additionally override the C++ operator new and operator delete; requires MEM_OVERRIDE and links the C++ runtime" OFF)

if(MEM_OVERRIDE_CXX AND NOT MEM_OVERRIDE)
  message(FATAL_ERROR "MEM_OVERRIDE_CXX requires MEM_OVERRIDE")
endif()

set(mem_features)

if(MEM_OVERRIDE)
  list(APPEND mem_features override)
endif()

if(MEM_OVERRIDE_CXX)
  enable_language(CXX)

  list(APPEND mem_features cxx)
endif()

find_port(jemalloc FEATURES ${mem_features})

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

if(MEM_OVERRIDE)
  target_compile_definitions(
    mem
    PRIVATE
      MEM_OVERRIDE
  )

  # Force every jemalloc object to be linked, including the Darwin malloc zone
  # registration that is pulled in through a constructor and would otherwise be
  # dropped from the static archive, so that the override takes effect.
  target_link_libraries(
    mem
    PUBLIC
      "$<LINK_LIBRARY:WHOLE_ARCHIVE,jemalloc>"
  )
else()
  target_link_libraries(
    mem
    PUBLIC
      jemalloc
  )
endif()

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
