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

# Network examples - only build if network apps are enabled
if (KAI_BUILD_NETWORK_APPS)
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
