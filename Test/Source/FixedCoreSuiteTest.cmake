# FixedCoreSuiteTest.cmake - Custom CMake file for building the fixed core test suite

# Find GTest
find_package(GTest REQUIRED)

# Add the executable
add_executable(FixedCoreSuite
    FixedCoreSuite.cpp
    ${TEST_COMMON_FILES}
)

# Link against Core and GTest
target_link_libraries(FixedCoreSuite
    Core
    GTest::GTest
    GTest::Main
)

# Set output directory
set_target_properties(FixedCoreSuite PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Bin/Test"
)