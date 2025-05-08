project(NetworkTest)

include_directories(${INCLUDE_HOME})
include_directories(${EXTERNAL}/raknet)
set(HOME ${APP_HOME}/NetworkTest)
set(SOURCES ${HOME}/Source/*.cpp)

file(GLOB_RECURSE sources ${SOURCES})
add_executable(NetworkTest ${sources})

target_link_libraries(NetworkTest Network Core Executor PiLang RhoLang TauLang RakNetLibStatic)

set_property(TARGET NetworkTest PROPERTY FOLDER Apps)