include_directories(${TEST_HOME}/Include})

add_subdirectory(Source)
add_subdirectory(Language)
#add_subdirectory(Common)

# Add Console tests
add_subdirectory(Console)

# Add our logging test
add_subdirectory(LogTest)

# Add example programs
add_subdirectory(Examples)

# Add Window tests
add_subdirectory(Window)

# Add Performance benchmark tests
add_subdirectory(Performance)

if (KAI_BUILD_TEST_NETWORK)
    add_subdirectory(Network)
    add_subdirectory(Network/ProxyTest)
else()
    message(WARNING "Not building Network unit tests")
endif()


