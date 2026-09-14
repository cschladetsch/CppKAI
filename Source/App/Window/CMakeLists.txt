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

include_directories(/opt/local/include ${EXTERNAL}/imgui ${EXTERNAL}/cpp-httplib ${SOURCE_HOME}/App/Window/Source)
link_directories(/opt/local/lib)

set(SOURCE_FILES
    # ExecutorWindow.cpp (~1700 lines) split by responsibility on 2026-09-14
    # into ExecutorWindow.h plus these six .cpp files, since the single
    # file had grown too large to navigate comfortably.
    ${SOURCE_HOME}/App/Window/Source/ExecutorWindowCore.cpp
    ${SOURCE_HOME}/App/Window/Source/ExecutorWindowConsole.cpp
    ${SOURCE_HOME}/App/Window/Source/ExecutorWindowDebugger.cpp
    ${SOURCE_HOME}/App/Window/Source/ExecutorWindowTree.cpp
    ${SOURCE_HOME}/App/Window/Source/ExecutorWindowCommand.cpp
    # Assistant tab (2026-09-14): talks to a locally-running `cppcoder
    # --serve` (CppLocalLlmCodeAssist) chat server over HTTP, via the
    # header-only cpp-httplib vendored under Ext/cpp-httplib.
    ${SOURCE_HOME}/App/Window/Source/ExecutorWindowAssistant.cpp
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

# Find Threads (std::thread, used by the Assistant tab's background HTTP
# request) - on some platforms/toolchains linking pthread isn't automatic.
find_package(Threads REQUIRED)

# Link libraries
target_link_libraries(${name} PiLang RhoLang Executor ConsoleLib CommonLang Core ImGuiLib glfw OpenGL::GL Threads::Threads)

if(WIN32)
    # The Windows SDK only ships OpenGL 1.1 headers (no GL/glext.h with
    # modern GL prototypes, unlike Linux's Mesa headers), so the
    # "no loader, just declare the prototypes yourself" approach used below
    # for other platforms doesn't work here. Use GLEW as a real loader
    # instead (available via vcpkg: `vcpkg install glew`).
    find_package(GLEW REQUIRED)
    target_link_libraries(${name} GLEW::GLEW)
    target_compile_definitions(${name} PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)

    # cpp-httplib (Assistant tab) needs plain Winsock even without TLS -
    # CPPHTTPLIB_OPENSSL_SUPPORT is intentionally not defined below since
    # this only ever talks to a plaintext http://127.0.0.1 cppcoder server,
    # so ws2_32 alone is enough (no crypt32/OpenSSL needed).
    target_link_libraries(${name} ws2_32)
else()
    # Use the local GL headers directly instead of depending on GLEW.
    target_compile_definitions(${name} PRIVATE IMGUI_IMPL_OPENGL_LOADER_CUSTOM GL_GLEXT_PROTOTYPES)
endif()
