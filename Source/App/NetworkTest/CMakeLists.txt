cmake_minimum_required(VERSION 3.6)
set(name NetworkTest)
project(${name})

include_directories(${INCLUDE_HOME})
include_directories(${EXTERNAL}/raknet/Source)

set(source_path ${APP_HOME}/${name}/Source)
add_executable(${name} ${source_path}/Main.cpp)
target_link_libraries(${name} Network Core Executor PiLang RhoLang TauLang RakNet)

set_property(TARGET ${name} PROPERTY FOLDER Apps)