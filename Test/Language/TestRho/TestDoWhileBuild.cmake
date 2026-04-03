# Add TestDoWhile executable
add_executable(TestDoWhile
    TestDoWhile.cpp
    ${CMAKE_SOURCE_DIR}/Test/Common/MyTestStruct.cpp
    ${CMAKE_SOURCE_DIR}/Test/Common/TestCommon.cpp
    ${CMAKE_SOURCE_DIR}/Test/Language/TestLangCommon.cpp
)

# Link to the required libraries
target_link_libraries(TestDoWhile PRIVATE Core Language Executor)

# Include directories
target_include_directories(TestDoWhile PRIVATE
    ${CMAKE_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/Test/Include
)