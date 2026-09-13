cmake_minimum_required(VERSION 3.28...3.30)

project(ImGuiLib)

# Use external imgui source
set(IMGUI_ROOT ${EXTERNAL}/imgui)

# Include directories
include_directories(${IMGUI_ROOT})

# Source files
set(IMGUI_SOURCES
    ${IMGUI_ROOT}/imgui.cpp
    ${IMGUI_ROOT}/imgui_demo.cpp
    ${IMGUI_ROOT}/imgui_draw.cpp
    ${IMGUI_ROOT}/imgui_widgets.cpp
    # Add any additional ImGui source files here
)

# Create the ImGui library
add_library(ImGuiLib STATIC ${IMGUI_SOURCES})
set_target_properties(ImGuiLib PROPERTIES OUTPUT_NAME ImGui)

# This vendored (older) ImGui version's ImGuiInputTextState constructor does
# memset(this, 0, sizeof(*this)) on a type Clang considers non-trivially
# copyable, which is safe in practice here (it really is a POD-shaped state
# struct) but triggers -Wnontrivial-memcall. Not our code to rewrite - just
# silence it for this target.
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(ImGuiLib PRIVATE -Wno-nontrivial-memcall)
endif()

# Install the library
install(TARGETS ImGuiLib DESTINATION lib)
