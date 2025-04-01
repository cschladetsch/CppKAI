# cmake_minimum_required(VERSION 3.6)

set(name NetworkGenerate)

project(${name})
set(SOURCE_ROOT ${SOURCE_HOME}/App/${name}/Source/*.cpp)
file(GLOB_RECURSE SOURCE_FILES ${SOURCE_ROOT})
link_libraries(Executor Core PiLang RhoLang)# platform-Linux)
add_executable(${name} ${SOURCE_FILES})
#cotire(${name})

# Find Boost components explicitly including program_options
find_package(Boost REQUIRED COMPONENTS system filesystem program_options)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${name} ${Boost_LIBRARIES})

