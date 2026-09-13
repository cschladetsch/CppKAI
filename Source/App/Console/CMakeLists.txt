cmake_minimum_required(VERSION 3.28...3.30)
set(name Console)
project(${name})
set(source_path ${APP_HOME}/${name}/Source)
set(header_path ${INCLUDE_HOME}/${name})

# Main Console app
add_executable(${name} ${source_path}/Main.cpp)
target_link_libraries(${name} ConsoleLib PiLang RhoLang Executor CommonLang Core ${Boost_LIBRARIES})
# Set the output directory explicitly for the Console app
set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
)

# See Test/Language/CMakeLists.txt for why: Pi's `if`/`ife` continuation
# dispatch (ExecuteContinuationInline) costs several native stack frames per
# Pi-level recursive call, so a real Rho/Pi script doing moderately deep
# recursion can overflow Windows' default 1MB thread stack in the same way
# PiAdvancedTests.ExtremeRecursiveSum did in the test suite.
if(WIN32)
    # clang's driver misparses a bare "/STACK:16777216" as a file path
    # (leading "/" looks like an absolute path on Windows) and refuses to
    # link ("no such file or directory"). -Xlinker forces it straight
    # through to lld-link/link.exe unchanged; MSVC's cl.exe accepts the
    # same spelling directly, so this works for both toolchains.
    target_link_options(${name} PRIVATE -Xlinker /STACK:16777216)
endif()

# Copy Console to ~/bin if it exists (Linux/macOS only)
if(NOT WIN32)
add_custom_command(TARGET ${name} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Checking if ~/bin directory exists..."
    COMMAND test -d "$ENV{HOME}/bin" && ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${name}> "$ENV{HOME}/bin/${name}" || echo "~/bin directory does not exist, skipping copy"
    COMMENT "Copying Console to ~/bin if directory exists"
)
endif()

# Network examples - only build when networking is enabled
if (KAI_NETWORKING)
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

    add_executable(ContinuationMigrationDemo
        ${source_path}/ContinuationMigrationDemo.cpp)
    target_link_libraries(ContinuationMigrationDemo
        ConsoleLib PiLang RhoLang Executor CommonLang Core Network
        ${Boost_LIBRARIES})
    set_target_properties(ContinuationMigrationDemo PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
    )
endif()
