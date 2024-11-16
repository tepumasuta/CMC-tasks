cmake_minimum_required(VERSION 3.11)
project("radsh") # re-bourne disabled again shell

SET(SRCS
    src/arena.c
)

add_library(radshlib STATIC ${SRCS})
target_include_directories(radshlib PUBLIC "include")

add_executable(radsh src/radsh.c)
target_link_libraries(radsh)
