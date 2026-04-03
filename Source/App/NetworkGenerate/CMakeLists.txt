set(name NetworkGenerate)

project(${name})
set(SOURCE_ROOT ${SOURCE_HOME}/App/${name}/Source/*.cpp)
file(GLOB_RECURSE SOURCE_FILES ${SOURCE_ROOT})
add_executable(${name} ${SOURCE_FILES})

target_link_libraries(${name} TauLang RhoLang PiLang Executor Core ${Boost_LIBRARIES})

set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
)
