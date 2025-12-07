cmake_minimum_required(VERSION 3.16)
project(splines)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Поиск GSL библиотеки
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSL REQUIRED gsl)

# Создание shared библиотеки с именем splines (как ожидает C# код)
add_library(splines SHARED splines.cpp)

target_compile_options(splines PRIVATE 
    -fPIC
    -Wall
    -Wextra
    -O2
)

target_include_directories(splines PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GSL_INCLUDE_DIRS}
)

target_link_libraries(splines
    ${GSL_LIBRARIES}
    m
)

# Установка правильных свойств для shared библиотеки
set_target_properties(splines PROPERTIES
    OUTPUT_NAME "splines"
    VERSION 1.0.0
    SOVERSION 1
)

# Для Linux установим правильное имя библиотеки
if(UNIX AND NOT APPLE)
    set_target_properties(splines PROPERTIES
        PREFIX "lib"
        SUFFIX ".so"
    )
endif()