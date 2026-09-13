cmake_minimum_required(VERSION 3.28...3.30)

# Set OpenGL policy to NEW to avoid warnings
if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

set(name ImGui)
project(${name})

#add_definitions(-DKAI_PLATFORM_OSX)
# macOS set(frameworks OpenGL Cocoa IOKit CoreVideo)

foreach(framework ${frameworks})
	set(${framework}_FIND_QUIETLY true) # doesn't seem to work?
	find_package(${framework})
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework ${framework}")
endforeach()

include_directories(/opt/local/include ${EXTERNAL}/imgui ${SOURCE_HOME}/App/Window/Source)
link_directories(/opt/local/lib)

set(SOURCE_FILES
    ${SOURCE_HOME}/App/Window/Source/ExecutorWindow.cpp
    ${SOURCE_HOME}/App/Window/Source/Main.cpp
    ${SOURCE_HOME}/App/Window/Source/imgui_impl_glfw.cpp
    ${SOURCE_HOME}/App/Window/Source/imgui_impl_opengl3.cpp
)

add_executable(${name} ${SOURCE_FILES})

# Set the output directory explicitly
set_target_properties(${name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}"
)

# Find GLFW3
find_package(glfw3 REQUIRED)

# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# Link libraries
target_link_libraries(${name} PiLang RhoLang Executor ConsoleLib CommonLang Core ImGuiLib glfw OpenGL::GL)

if(WIN32)
    # The Windows SDK only ships OpenGL 1.1 headers (no GL/glext.h with
    # modern GL prototypes, unlike Linux's Mesa headers), so the
    # "no loader, just declare the prototypes yourself" approach used below
    # for other platforms doesn't work here. Use GLEW as a real loader
    # instead (available via vcpkg: `vcpkg install glew`).
    find_package(GLEW REQUIRED)
    target_link_libraries(${name} GLEW::GLEW)
    target_compile_definitions(${name} PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)
else()
    # Use the local GL headers directly instead of depending on GLEW.
    target_compile_definitions(${name} PRIVATE IMGUI_IMPL_OPENGL_LOADER_CUSTOM GL_GLEXT_PROTOTYPES)
endif()
