find_package(
        Clang
    REQUIRED
    CONFIG
)

find_package(
        Boost 1.78.0
    REQUIRED
    CONFIG
)

add_library(
    libtuor
        abstract_code.cpp
        code_generator.cpp
        code_output.cpp
        utils.cpp
        clang_source_parser.cpp
        clang_class_visitor.cpp
        clang_statement_visitor.cpp
        clang_expression_visitor.cpp
        clang_utils.cpp
)

target_include_directories(
        libtuor
    PUBLIC
        ${PROJECT_SOURCE_DIR}
    PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
)

target_compile_options(
        libtuor
    PRIVATE
        -std=c++20
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
)

target_link_libraries(
        libtuor
    PRIVATE
        clangAST
        clangASTMatchers
        clangBasic
        clangFrontend
        clangSerialization
        clangTooling
)