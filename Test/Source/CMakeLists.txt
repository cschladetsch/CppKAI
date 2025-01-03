cmake_minimum_required(VERSION 3.6)

set(name KaiTest)
project(${name})
set(source_path ${TEST_HOME}/Source)
set(header_path ${TEST_HOME}/Include)
set(script_path ${TEST_HOME}/Scripts)
file(GLOB_RECURSE sources ${source_path}/*.cpp)
file(GLOB_RECURSE headers ${header_path}/*.h)
list(APPEND sources ${TEST_HOME}/Common/TestCommon.cpp)
list(APPEND sources ${TEST_HOME}/Common/MyTestStruct.cpp)
include_directories(${header_path})
link_libraries(Executor Core PiLang RhoLang TauLang ${GTEST_BOTH_LIBRARIES} ${Boost_LIBRARIES} gtest_main)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}/Test")
#cotire(${name})


