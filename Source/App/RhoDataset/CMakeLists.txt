cmake_minimum_required(VERSION 3.6)

set(name RhoDataset)
project(${name})

set(source_path ${APP_HOME}/${name}/Source)
add_executable(${name} ${source_path}/Main.cpp)
target_link_libraries(${name} LLM Core ${Boost_LIBRARIES})
set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
)
