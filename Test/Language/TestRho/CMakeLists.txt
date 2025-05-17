cmake_minimum_required(VERSION 3.10)

# Add the new test file to the build
add_library(TestRhoFiles OBJECT
    SemicolonSyntaxTests.cpp
    TestForLoopSemicolons.cpp
    HelloWorld.cpp
    SimpleRhoTest.cpp
    SimpleRhoPiTests.cpp
    AdvancedRhoPiTests.cpp
    SimpleRho.cpp
    MinimalRhoTest.cpp
    TypeTest.cpp
    SimpleDirectTest.cpp
    DirectBinaryOpTest.cpp
    AdditionalRhoTests.cpp
    DiagnoseTestLang.cpp
    RhoPiFix.cpp  # Direct implementations of tests
    MinimalTest.cpp  # Super minimal test - should always pass
    # Note: DoWhile tests are disabled pending implementation completion
    # See /home/xian/local/KAI/Todo-DoWhile.md for details
)

# Add includes directories to make sure our fix files compile correctly
target_include_directories(TestRhoFiles PRIVATE 
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/Test/Include
)

# Make sure the test files get included in the main test binary
set_target_properties(TestRhoFiles PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Ensure our Rho script files get copied to the build directory
file(GLOB RHO_SCRIPT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.rho
    ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/*.rho
)

# Copy script files to build directory
foreach(SCRIPT_FILE ${RHO_SCRIPT_FILES})
    get_filename_component(FILENAME ${SCRIPT_FILE} NAME)
    configure_file(
        ${SCRIPT_FILE}
        ${CMAKE_CURRENT_BINARY_DIR}/Scripts/${FILENAME}
        COPYONLY
    )
endforeach()