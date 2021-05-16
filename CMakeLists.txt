cmake_minimum_required(VERSION 3.20.2)
project(fri-clang-ast)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  "${CMAKE_SOURCE_DIR}/cmake/modules"
)

find_package(LibClangTooling REQUIRED)

add_definitions(${LibClangTooling_DEFINITIONS})

include_directories(
  ${LibClangTooling_INCLUDE_DIRS}
)

add_executable(find-class-decls ./src/main.cpp)

target_compile_options(find-class-decls PRIVATE -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -O3)

target_link_libraries(find-class-decls
  ${LibClangTooling_LIBRARIES}
)