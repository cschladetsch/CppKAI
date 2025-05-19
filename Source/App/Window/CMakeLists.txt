cmake_minimum_required(VERSION 3.7)

# Set OpenGL policy to NEW to avoid warnings
if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

set(name Window)
project(${name})

#add_definitions(-DKAI_PLATFORM_OSX)
# macOS set(frameworks OpenGL Cocoa IOKit CoreVideo)

foreach(framework ${frameworks})
	set(${framework}_FIND_QUIETLY true) # doesn't seem to work?
	find_package(${framework})
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework ${framework}")
endforeach()

include_directories(/opt/local/include ${KAI_HOME}/Include/Imgui)
link_directories(/opt/local/lib)

# Only include ExecutorWindow.cpp and Main.cpp
set(SOURCE_FILES
    ${SOURCE_HOME}/App/${name}/Source/ExecutorWindow.cpp
    ${SOURCE_HOME}/App/${name}/Source/Main.cpp
)

link_libraries(Executor Core PiLang RhoLang ImGui)

add_executable(${name} ${SOURCE_FILES})

# Set the output directory explicitly
set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${SAFER_BIN_HOME}"
)

# Find GLFW3
find_package(glfw3 REQUIRED)

# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# Link libraries
target_link_libraries(${name} glfw OpenGL::GL)
