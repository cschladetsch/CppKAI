cmake_minimum_required(VERSION 3.20)
project(kaish CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_TOOLCHAIN_FILE "" CACHE STRING "" FORCE)

add_library(kaish_core
    src/object.cpp
    src/context.cpp
    src/parser.cpp
    src/registry.cpp
)
target_include_directories(kaish_core PUBLIC include)

add_executable(kaish src/main.cpp)
target_link_libraries(kaish PRIVATE kaish_core)

add_executable(kaish_tests tests/test_kaish.cpp)
target_link_libraries(kaish_tests PRIVATE kaish_core)

enable_testing()
add_test(NAME KaiShUnitTests COMMAND kaish_tests)
