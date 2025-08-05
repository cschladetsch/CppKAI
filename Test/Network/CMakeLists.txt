cmake_minimum_required(VERSION 3.28...3.30)

# Network test project
project(Test_Network)

# Always build network tests
# if(KAI_BUILD_TEST_NETWORK)
    # GTest-based test files
    set(GTEST_SOURCES
        TestNode.cpp
        TestGenerateProxy.cpp
        TauNetworkCommunicationTest.cpp
        AdvancedNetworkTests.cpp
        # ICQStyleChatTest.cpp
        # ChatProxyGenerationTest.cpp
        # ChatFunctionalityTests.cpp
        # ChatAdvancedTests.cpp
    )

    # Create the GTest-based test executable
    add_executable(Test_Network ${GTEST_SOURCES})
    
    # Set the runtime output directory for the test executable
    set_target_properties(Test_Network PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin"
    )

    # Link libraries
    target_link_libraries(Test_Network 
        Network
        TauLang
        Executor
        Core 
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
    
    # Build standalone test programs separately
    # Console Connection Test
    add_executable(ConsoleConnectionTest ConsoleConnectionTest.cpp)
    set_target_properties(ConsoleConnectionTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin"
    )
    target_link_libraries(ConsoleConnectionTest 
        Core Network Executor
        ${GTEST_BOTH_LIBRARIES}
        ${Boost_LIBRARIES}
        pthread
    )
    target_include_directories(ConsoleConnectionTest PRIVATE
        ${KAI_HOME}/Include
        ${TEST_HOME}/Include
    )
    
    # Integrated Console Test
    add_executable(IntegratedConsoleTest IntegratedConsoleTest.cpp)
    set_target_properties(IntegratedConsoleTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin"
    )
    target_link_libraries(IntegratedConsoleTest 
        Core Network Executor
        ${GTEST_BOTH_LIBRARIES}
        ${Boost_LIBRARIES}
        pthread
    )
    target_include_directories(IntegratedConsoleTest PRIVATE
        ${KAI_HOME}/Include
        ${TEST_HOME}/Include
    )
    
    # Calculation Test
    add_executable(CalculationTest CalculationTest.cpp)
    set_target_properties(CalculationTest PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Bin"
    )
    target_link_libraries(CalculationTest 
        Core Network Executor
        ${GTEST_BOTH_LIBRARIES}
        ${Boost_LIBRARIES}
        pthread
    )
    target_include_directories(CalculationTest PRIVATE
        ${KAI_HOME}/Include
        ${TEST_HOME}/Include
    )
# endif()