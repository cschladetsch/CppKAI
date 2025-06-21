# Modern C++ Examples for KAI

# Ensure we're using C++20 for concepts and other modern features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/Include)

# Link libraries
set(KAI_LIBS Core Executor CommonLang)

# Modern C++ Demo
add_executable(ModernCppDemo 
    ModernCppDemo.cpp
)

target_link_libraries(ModernCppDemo ${KAI_LIBS})

# Set compiler flags for modern C++
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(ModernCppDemo PRIVATE 
        -Wall -Wextra -Wpedantic
        -fconcepts  # Enable concepts for GCC/Clang
    )
endif()

if(MSVC)
    target_compile_options(ModernCppDemo PRIVATE 
        /W4
        /std:c++20
    )
endif()

# Optional: Add a test to run the demo
if(BUILD_TESTING)
    add_test(NAME ModernCppDemo COMMAND ModernCppDemo)
endif()