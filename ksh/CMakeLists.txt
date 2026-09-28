cmake_minimum_required(VERSION 3.20)
project(ksh CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_TOOLCHAIN_FILE "" CACHE STRING "" FORCE)

add_library(ksh_core
    src/object.cpp
    src/context.cpp
    src/parser.cpp
    src/registry.cpp
)
target_include_directories(ksh_core PUBLIC include)

add_executable(ksh src/main.cpp)
target_link_libraries(ksh PRIVATE ksh_core)

add_executable(ksh_tests tests/test_ksh.cpp)
target_link_libraries(ksh_tests PRIVATE ksh_core)

enable_testing()
add_test(NAME KshUnitTests COMMAND ksh_tests)
