cmake_minimum_required(VERSION 3.11)
project("radsh") # re-bourne disabled again shell

SET(SRCS
    src/arena.c
    src/defer.c
    src/string_view.c
    src/token.c
)

set(CMAKE_C_COMPILER "gcc")

add_library(radshlib STATIC ${SRCS})
target_include_directories(radshlib PUBLIC "include")

add_executable(radsh src/radsh.c)
target_link_libraries(radsh radshlib)

foreach(target IN ITEMS radshlib radsh)
    target_compile_features(${target} PRIVATE c_std_17)
    target_compile_options(${target} PRIVATE $<$<CONFIG:Debug>:-g3;-ggdb;-O0>)
endforeach()
