cmake_minimum_required(VERSION 3.6)

set(name KaiTest)
project(${name})
set(source_path ${TEST_HOME}/Source)
set(header_path ${TEST_HOME}/Include)
set(script_path ${TEST_HOME}/Scripts)

# Exclude problematic files for now
set(EXCLUDE_FILES
    "TestEvents.cpp"
    "TestRakNetStub.cpp"  # Will add this back once we fix all issues
)

file(GLOB_RECURSE all_source_files ${source_path}/*.cpp)

# Filter out excluded files
set(sources "")
foreach(file ${all_source_files})
    get_filename_component(filename ${file} NAME)
    list(FIND EXCLUDE_FILES ${filename} exclude_index)
    if(${exclude_index} EQUAL -1)
        list(APPEND sources ${file})
    endif()
endforeach()

file(GLOB_RECURSE headers ${header_path}/*.h)
list(APPEND sources ${TEST_HOME}/Common/TestCommon.cpp)
list(APPEND sources ${TEST_HOME}/Common/MyTestStruct.cpp)

include_directories(${header_path})
link_libraries(Executor Core PiLang RhoLang TauLang Network ${GTEST_BOTH_LIBRARIES} ${Boost_LIBRARIES} gtest_main)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}/Test")
add_executable(${name} ${sources})
#cotire(${name})


