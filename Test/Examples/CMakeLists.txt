# CMakeLists.txt for Test/Examples

# Create example directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/Bin/Test/Examples)

# Add simple color example executable
add_executable(SimpleColorExample SimpleColorExample.cpp)

# Set output directory
set_target_properties(SimpleColorExample PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin/Test/Examples
)

# Include directories for the rang library
target_include_directories(SimpleColorExample PRIVATE
    ${CMAKE_SOURCE_DIR}/Ext/rang/include
)

set(demo_test_name ContinuationMobilityDemoTests)

add_executable(${demo_test_name} ContinuationMobilityDemoTests.cpp)

set_target_properties(${demo_test_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin/Test/Examples
)

target_link_libraries(${demo_test_name}
    ${GTEST_BOTH_LIBRARIES}
    gtest_main
)

add_test(NAME ${demo_test_name} COMMAND ${demo_test_name})
