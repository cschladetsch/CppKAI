cmake_minimum_required(VERSION 3.6)
set(name Console)
project(${name})
set(source_path ${APP_HOME}/${name}/Source)
set(header_path ${INCLUDE_HOME}/${name})

# Main Console app
add_executable(${name} ${source_path}/Main.cpp)
target_link_libraries(${name} PiLang RhoLang Executor CommonLang Core ${Boost_LIBRARIES})
# Set the output directory explicitly for the Console app
set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
)

# Copy Console to ~/bin if it exists
add_custom_command(TARGET ${name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Checking if ~/bin directory exists..."
    COMMAND test -d "$ENV{HOME}/bin" && ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${name}> "$ENV{HOME}/bin/${name}" || echo "~/bin directory does not exist, skipping copy"
    COMMENT "Copying Console to ~/bin if directory exists"
)

# Network examples - only build if RakNet is enabled
if (KAI_BUILD_RAKNET)
    add_executable(SimpleServer ${source_path}/SimpleServer.cpp)
    target_link_libraries(SimpleServer Executor Core Network ${Boost_LIBRARIES})
    # Set the output directory explicitly for SimpleServer
    set_target_properties(SimpleServer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
    )

    add_executable(SimpleClient ${source_path}/SimpleClient.cpp)
    target_link_libraries(SimpleClient Executor Core Network ${Boost_LIBRARIES})
    # Set the output directory explicitly for SimpleClient
    set_target_properties(SimpleClient PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
    )
endif()

# Minimal examples - only build if network apps are enabled
if (KAI_BUILD_NETWORK_APPS)
    add_executable(MinimalServer ${source_path}/MinimalServer.cpp)
    target_link_libraries(MinimalServer Executor Core ${Boost_LIBRARIES})
    # Set the output directory explicitly for the MinimalServer
    set_target_properties(MinimalServer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
    )

    add_executable(MinimalClient ${source_path}/MinimalClient.cpp)
    target_link_libraries(MinimalClient Executor Core ${Boost_LIBRARIES})
    # Set the output directory explicitly for the MinimalClient
    set_target_properties(MinimalClient PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
    )
endif()

# Network-related executables - only build if explicitly enabled
if (KAI_BUILD_NETWORK_APPS)
    # Peer-to-peer networking implementation with SSH-like semantics
    add_executable(NetworkPeer ${source_path}/NetworkPeer.cpp)
    target_link_libraries(NetworkPeer Executor Core ${Boost_LIBRARIES})
    # Set the output directory explicitly
    set_target_properties(NetworkPeer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
    )

    # Configurable server and client for network testing
    add_executable(ConfigurableServer ${source_path}/ConfigurableServer.cpp)
    target_link_libraries(ConfigurableServer Executor Core ${Boost_LIBRARIES})
    # Set the output directory explicitly
    set_target_properties(ConfigurableServer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
    )

    add_executable(ConfigurableClient ${source_path}/ConfigurableClient.cpp)
    target_link_libraries(ConfigurableClient Executor Core ${Boost_LIBRARIES})
    # Set the output directory explicitly
    set_target_properties(ConfigurableClient PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
    )
endif()
