include_directories(${TEST_HOME}/Include})

add_subdirectory(Source)
add_subdirectory(Language)
#add_subdirectory(Common)

# Add our logging test
add_subdirectory(LogTest)

# Add example programs
add_subdirectory(Examples)

if (KAI_BUILD_TEST_NETWORK)
    add_subdirectory(Network)
    add_subdirectory(Network/ProxyTest)
else()
    message(WARNING "Not building Network unit tests")
endif()


