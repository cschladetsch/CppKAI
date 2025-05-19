# cmake_minimum_required(VERSION 3.6)

set(name NetworkGenerate)

project(${name})
set(SOURCE_ROOT ${SOURCE_HOME}/App/${name}/Source/*.cpp)
file(GLOB_RECURSE SOURCE_FILES ${SOURCE_ROOT})
add_executable(${name} ${SOURCE_FILES})

# Link against libraries in the correct order to resolve dependencies
target_link_libraries(${name} TauLang RhoLang PiLang Executor Core)

# Set the output directory explicitly
set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SAFER_BIN_HOME}"
)

#cotire(${name})

# Find Boost components explicitly including program_options
find_package(Boost REQUIRED COMPONENTS system filesystem program_options)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${name} ${Boost_LIBRARIES})

