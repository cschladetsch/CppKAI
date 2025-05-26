cmake_minimum_required(VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}/Test")
set(core_libs Executor Core PiLang RhoLang TauLang)

macro(build_lang_test lang)
endmacro()
message(STATUS "GOOGLETESTDIR - ${CMAKE_BINARY_DIR}/googletest")
#include(${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include)

set(langs Tau Rho Pi)
foreach (lang ${langs})
    project(Test${lang})
    set(testName Test${lang})
    message(STATUS "--- building unit tests for ${testName} ---")
    set(home ${TEST_HOME}/Language/Test${lang})
    set(source_path ${home})
    set(header_path ${home}/Include)
    set(script_path ${TEST_HOME}/Language/Test${lang}/Scripts/)
    string(TOLOWER ${lang} ext)
    
    # Special handling for Rho language tests to deal with duplicates
    if(lang STREQUAL "Rho")
        # Include only the specific files we want for Rho tests
        set(sources
            ${source_path}/SemicolonSyntaxTests.cpp
            ${source_path}/TestForLoopSemicolons.cpp
            ${source_path}/TypeTest.cpp
            ${source_path}/SimpleDirectTest.cpp
            ${source_path}/DirectBinaryOpTest.cpp         # Re-enabled after fixing dup + pattern issue
            ${source_path}/AdditionalBinaryOpTests.cpp    # Re-enabling Binary Op Tests (Fixed)
            ${source_path}/AdvancedBinaryOpTests.cpp      # Re-enabling Advanced Binary Op Tests (Fixed)
            ${source_path}/AdditionalRhoTests.cpp
            ${source_path}/DiagnoseTestLang.cpp
            ${source_path}/MinimalTest.cpp
            ${source_path}/RhoPiFix.cpp
            ${source_path}/TestDoWhile.cpp                # Re-enabling TestDoWhile tests
            ${source_path}/ExtendedBinaryOpTests.cpp      # Adding extended binary op tests
            ${source_path}/BasicTest.cpp                  # Adding basic tests
            ${source_path}/BasicTests.cpp                 # Adding more basic tests
            ${source_path}/RhoAssertTest.cpp              # Adding assert tests
            ${source_path}/PiAssertInRhoTest.cpp          # Adding Pi assert in Rho tests
            ${source_path}/Simple20Plus20Test.cpp         # Adding simple 20+20 test
            ${source_path}/SimpleTest.cpp                 # Adding simple test
            ${source_path}/ForLoopTests.cpp               # Adding for loop tests
            ${source_path}/ExtendedRhoTests.cpp           # Added 20 new test cases for Rho
            ${source_path}/RhoAdvancedOperationsTests.cpp  # Advanced operations tests
            ${source_path}/RhoControlStructuresTests.cpp   # Control structures tests
            ${source_path}/RhoFunctionAndScopeTests.cpp    # Function and scope tests
            ${source_path}/RhoAdvancedDataTests.cpp        # Advanced data structure tests
            ${source_path}/RhoDemo.cpp                    # Demo runner for showcasing features
            # New Rho tests added
            ${source_path}/RhoPatternMatchingTests.cpp     # Pattern matching tests
            ${source_path}/RhoLambdaTests.cpp              # Lambda expression tests
            ${source_path}/RhoErrorHandlingTests.cpp       # Error handling tests
            ${source_path}/RhoGeneratorTests.cpp           # Generator and lazy evaluation tests
        )
    elseif(lang STREQUAL "Pi")
        # Explicitly list Pi tests to avoid including unwanted files
        set(sources
            ${source_path}/PiBinaryOpTests.cpp   # Re-enabled with simplified implementation
            ${source_path}/SimpleTest.cpp
            ${source_path}/BasicTest.cpp
            ${source_path}/TestPi.cpp
            ${source_path}/ArrayOpTest.cpp
            ${source_path}/PiAssertTest.cpp
            ${source_path}/StandalonePiTest.cpp
            ${source_path}/ExtendedPiTests.cpp   # Added 20 new test cases for Pi
            ${source_path}/AdvancedPiTests.cpp   # Added 20 more advanced test cases for Pi
            ${source_path}/TestPiLabels.cpp      # Test for label operations
            # New Pi tests added
            ${source_path}/PiStackManipulationTests.cpp     # Stack manipulation tests
            ${source_path}/PiControlFlowTests.cpp           # Control flow tests
            ${source_path}/PiStringOperationsTests.cpp      # String operation tests
            ${source_path}/PiMathOperationsTests.cpp        # Mathematical operation tests
            # Larger tests to include if needed:
            #${source_path}/TestPiAdvanced.cpp
            #${source_path}/TestPiAdvanced2.cpp
            #${source_path}/TestPiParser.cpp
            #${source_path}/TutorialTest.cpp
        )
    elseif(lang STREQUAL "Tau")
        # Explicitly list Tau tests including our new network connection tests
        set(sources
            ${source_path}/TestTau.cpp
            ${source_path}/TauNamespaceTests.cpp
            ${source_path}/TauClassTests.cpp
            ${source_path}/TauCodeGenTests.cpp
            ${source_path}/NetworkConnectionTests.cpp
            ${source_path}/TauAdvancedTypeTests.cpp        # Advanced type system tests
            ${source_path}/TauCodeGenerationTests.cpp      # Code generation tests
            ${source_path}/TauGenerateStructTests.cpp      # Struct generation tests
            ${source_path}/TauSeparateGenerationTests.cpp  # Tests for separated proxy/agent generation
            # New Tau tests added
            ${source_path}/TauTemplateTests.cpp            # Template and generic programming tests
            ${source_path}/TauAsyncTests.cpp               # Async/await functionality tests
            ${source_path}/TauInterfaceTests.cpp           # Interface and contract tests
            ${source_path}/TauAttributeTests.cpp           # Attribute and metadata tests
        )
    else()
        # For other languages, use the standard approach
        file(GLOB_RECURSE sources ${source_path}/*.cpp)
        # Exclude backups and old_backups directories
        list(FILTER sources EXCLUDE REGEX ".*/backups/.*")
        list(FILTER sources EXCLUDE REGEX ".*/old_backups/.*")
    endif()
    
    # Common source files for all language tests
    list(APPEND sources ${TEST_HOME}/Language/TestLangCommon.cpp)
    list(APPEND sources ${TEST_HOME}/Common/TestCommon.cpp)
    list(APPEND sources ${TEST_HOME}/Common/MyTestStruct.cpp)
    
    file(GLOB_RECURSE headers ${header_path}/*.h)
    file(GLOB_RECURSE scripts ${script_path}/*.${ext})
    include_directories(${header_path})
    include_directories(${TEST_HOME}/Include)
    enable_testing()
    
    message(STATUS ">>>> SOURCES for ${testName}: ${sources} <<<<")
    
    add_executable(${testName} ${sources} "${TEST_HOME}/Source/Main.cpp")
    target_link_libraries(${testName} ${core_libs} ${GTEST_BOTH_LIBRARIES} ${Boost_LIBRARIES})
    include(GoogleTest)
    
    # Try to discover tests if the module is available
    if(COMMAND gtest_discover_tests)
        gtest_discover_tests(${testName})
    endif()
    
    source_group("Scripts" FILES ${scripts} )
    target_compile_definitions(${testName} PRIVATE 
        KAI_SCRIPT_ROOT=${script_path} 
        KAI_LANG_NAME=${lang} 
        KAI_LANG_EXT=.${ext}
    )
endforeach()

