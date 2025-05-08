cmake_minimum_required(VERSION 3.6)
set(name Console)
project(${name})
set(source_path ${APP_HOME}/${name}/Source)
set(header_path ${INCLUDE_HOME}/${name})

# Main Console app
add_executable(${name} ${source_path}/Main.cpp)
target_link_libraries(${name} Executor Core PiLang RhoLang ${Boost_LIBRARIES})

# Network examples
add_executable(SimpleServer ${source_path}/SimpleServer.cpp)
target_link_libraries(SimpleServer Executor Core Network ${Boost_LIBRARIES})

add_executable(SimpleClient ${source_path}/SimpleClient.cpp)
target_link_libraries(SimpleClient Executor Core Network ${Boost_LIBRARIES})

add_executable(MinimalServer ${source_path}/MinimalServer.cpp)
target_link_libraries(MinimalServer Executor Core ${Boost_LIBRARIES})

add_executable(MinimalClient ${source_path}/MinimalClient.cpp)
target_link_libraries(MinimalClient Executor Core ${Boost_LIBRARIES})
