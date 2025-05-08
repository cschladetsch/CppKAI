cmake_minimum_required(VERSION 3.6)

set(name KaiTest)
project(${name})
set(source_path ${TEST_HOME}/Source)
set(header_path ${TEST_HOME}/Include)
set(script_path ${TEST_HOME}/Scripts)

# Include all test files
file(GLOB_RECURSE all_source_files ${source_path}/*.cpp)

# Use all source files
set(sources ${all_source_files})

file(GLOB_RECURSE headers ${header_path}/*.h)
list(APPEND sources ${TEST_HOME}/Common/TestCommon.cpp)
list(APPEND sources ${TEST_HOME}/Common/MyTestStruct.cpp)

include_directories(${header_path})
link_libraries(Executor Core PiLang RhoLang TauLang Network ${GTEST_BOTH_LIBRARIES} ${Boost_LIBRARIES} gtest_main)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}/Test")
add_executable(${name} ${sources})
#cotire(${name})


