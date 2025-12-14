cmake_minimum_required(VERSION 3.15)
project(macoq LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB SRCS 
    src/*.c
)

add_library(macoq ${SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_definitions(macoq PRIVATE
    $<$<CONFIG:Debug>:MACOQ_DEBUG=1>
)

option(MACOQ_LIB_TESTS "Build macoq tests" ON)
if(MACOQ_LIB_TESTS)
    add_subdirectory(tests)
endif()

