# Add the new test files
add_library(TestNewRhoTests OBJECT NewRhoTests.cpp)

# Make sure our tests have access to includes
target_include_directories(TestNewRhoTests PRIVATE 
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/Test/Include
)

# Make sure the test files get included in the main test binary
set_target_properties(TestNewRhoTests PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)