find_package(
        Boost 1.78.0
    REQUIRED
    CONFIG
    COMPONENTS
        program_options
)

add_executable(
    tuor
        main.cpp
)

target_include_directories(
        tuor
    PRIVATE
        ${Boost_INCLUDE_DIRS}
)

target_compile_options(
        tuor
    PRIVATE
        -std=c++20
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
)

target_link_libraries(
        tuor
    PRIVATE
        ${Boost_LIBRARIES}
        libtuor
)