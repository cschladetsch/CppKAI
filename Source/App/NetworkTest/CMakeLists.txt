cmake_minimum_required(VERSION 3.6)
set(name NetworkTest)
project(${name})

include_directories(${INCLUDE_HOME})
include_directories(${KAI_HOME}/Ext/RakNet/Source)

set(source_path ${APP_HOME}/${name}/Source)
add_executable(${name} ${source_path}/Main.cpp)

set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
)

target_link_libraries(${name} Network Core Executor PiLang RhoLang TauLang CommonLang)

set_property(TARGET ${name} PROPERTY FOLDER Apps)