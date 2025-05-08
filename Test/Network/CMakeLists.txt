cmake_minimum_required(VERSION 3.28...3.30)

# Network test project
project(Test_Network)

# Only build if testing is enabled
if(KAI_BUILD_TEST_NETWORK)
    # Source files
    set(SOURCES
        TestNode.cpp
    )

    # Create the test executable
    add_executable(Test_Network ${SOURCES})

    # Link libraries
    target_link_libraries(Test_Network 
        Core 
        Network 
        Executor
        ${GTEST_BOTH_LIBRARIES}
        ${Boost_LIBRARIES}
        pthread
    )

    # Add include directories
    target_include_directories(Test_Network PRIVATE
        ${KAI_HOME}/Include
        ${TEST_HOME}/Include
        ${GTEST_INCLUDE_DIRS}
    )

    # Set compile flags
    set_target_properties(Test_Network PROPERTIES COMPILE_FLAGS "-g -Wno-deprecated")

    # Add the test to CTest
    add_test(NAME Test_Network COMMAND Test_Network)
endif()