set(demo_name ContinuationMobilityDemo)

add_executable(${demo_name} ContinuationMobilityDemo.cpp)

set_target_properties(${demo_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
)

target_compile_features(${demo_name} PRIVATE cxx_std_23)
