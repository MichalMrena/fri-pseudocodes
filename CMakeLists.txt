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

add_executable(generate-pseudocode ./src/main.cpp ./src/abstract_code.cpp ./src/code_generator.cpp ./src/cxx_source_parser.cpp)


target_compile_options(generate-pseudocode PRIVATE -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -O3)
# TODO debug config, check that c++14 being used
# target_compile_options(generate-pseudocode PRIVATE -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -g)

target_link_libraries(generate-pseudocode
  ${LibClangTooling_LIBRARIES}
)