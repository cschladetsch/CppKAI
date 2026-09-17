cmake_minimum_required(VERSION 3.28...3.30)
set(name WebConsole)
project(${name})

add_executable(${name} ${CMAKE_CURRENT_LIST_DIR}/Main.cpp)

target_link_libraries(${name} ConsoleLib PiLang RhoLang Executor CommonLang Core ${Boost_LIBRARIES})

if(WIN32)
    target_link_libraries(${name} ws2_32)
    target_link_options(${name} PRIVATE -Xlinker /STACK:16777216)
endif()

set_target_properties(${name} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
)
