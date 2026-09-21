cmake_minimum_required(VERSION 3.28...3.30)
set(name Calc)
project(${name})
set(source_path ${APP_HOME}/${name}/Source)

add_executable(${name} ${source_path}/Main.cpp)
target_link_libraries(${name} ConsoleLib PiLang RhoLang Executor CommonLang Core ${Boost_LIBRARIES})

set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
)

# Same rationale as Console/CMakeLists.txt: Pi's continuation dispatch uses
# several native stack frames per Pi-level call, and Windows' default 1MB
# thread stack can be too small for a moderately deep recursive expression.
if(WIN32)
    target_link_options(${name} PRIVATE -Xlinker /STACK:16777216)
endif()

# Also install as `calc` so it's a drop-in replacement on PATH, alongside its
# own Calc/kaicalc name. On Windows this deliberately does NOT touch
# C:\Windows\System32\calc.exe (nor could it without admin rights) - see
# Source/App/Calc/README.md for how to actually shadow the built-in calc
# from a PowerShell profile.
if(WIN32)
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${name}> "${BIN_HOME}/kaicalc.exe"
        COMMENT "Copying Calc to kaicalc.exe alongside it"
    )
else()
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${name}> "${BIN_HOME}/kaicalc"
        COMMENT "Copying Calc to kaicalc alongside it"
    )
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Checking if ~/bin directory exists..."
        COMMAND test -d "$ENV{HOME}/bin" && ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${name}> "$ENV{HOME}/bin/kaicalc" || echo "~/bin directory does not exist, skipping copy"
        COMMENT "Copying kaicalc to ~/bin if directory exists"
    )
endif()
