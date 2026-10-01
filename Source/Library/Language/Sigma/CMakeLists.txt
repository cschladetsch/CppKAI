cmake_minimum_required(VERSION 3.28)

project(SigmaLang CXX)

file(GLOB_RECURSE SOURCES Source/*.cpp)
file(GLOB_RECURSE HEADERS Include/*.h)

add_library(SigmaLang STATIC ${SOURCES} ${HEADERS})

target_link_libraries(SigmaLang PRIVATE Core)

target_include_directories(SigmaLang PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Include>
    $<INSTALL_INTERFACE:Include>
)