cmake_minimum_required(VERSION 3.6)
set(name Console)
project(${name})
set(source_path ${APP_HOME}/${name}/Source)
set(header_path ${INCLUDE_HOME}/${name})

# Main Console app
add_executable(${name} ${source_path}/Main.cpp)
target_link_libraries(${name} Executor Core PiLang RhoLang ${Boost_LIBRARIES})
# Set the output directory explicitly for the Console app
set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SAFER_BIN_HOME}"
)

# Network examples - only build if RakNet is enabled
if (KAI_BUILD_RAKNET)
    add_executable(SimpleServer ${source_path}/SimpleServer.cpp)
    target_link_libraries(SimpleServer Executor Core Network ${Boost_LIBRARIES})
    # Set the output directory explicitly for SimpleServer
    set_target_properties(SimpleServer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${SAFER_BIN_HOME}"
    )

    add_executable(SimpleClient ${source_path}/SimpleClient.cpp)
    target_link_libraries(SimpleClient Executor Core Network ${Boost_LIBRARIES})
    # Set the output directory explicitly for SimpleClient
    set_target_properties(SimpleClient PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${SAFER_BIN_HOME}"
    )
endif()

# Minimal examples that don't require networking
add_executable(MinimalServer ${source_path}/MinimalServer.cpp)
target_link_libraries(MinimalServer Executor Core ${Boost_LIBRARIES})
# Set the output directory explicitly for the MinimalServer
set_target_properties(MinimalServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SAFER_BIN_HOME}"
)

add_executable(MinimalClient ${source_path}/MinimalClient.cpp)
target_link_libraries(MinimalClient Executor Core ${Boost_LIBRARIES})
# Set the output directory explicitly for the MinimalClient
set_target_properties(MinimalClient PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SAFER_BIN_HOME}"
)

# Peer-to-peer networking implementation with SSH-like semantics
add_executable(NetworkPeer ${source_path}/NetworkPeer.cpp)
target_link_libraries(NetworkPeer Executor Core ${Boost_LIBRARIES})
# Set the output directory explicitly
set_target_properties(NetworkPeer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SAFER_BIN_HOME}"
)

# Configurable server and client for network testing
add_executable(ConfigurableServer ${source_path}/ConfigurableServer.cpp)
target_link_libraries(ConfigurableServer Executor Core ${Boost_LIBRARIES})
# Set the output directory explicitly
set_target_properties(ConfigurableServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SAFER_BIN_HOME}"
)

add_executable(ConfigurableClient ${source_path}/ConfigurableClient.cpp)
target_link_libraries(ConfigurableClient Executor Core ${Boost_LIBRARIES})
# Set the output directory explicitly
set_target_properties(ConfigurableClient PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SAFER_BIN_HOME}"
)
