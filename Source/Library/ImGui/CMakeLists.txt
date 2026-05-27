cmake_minimum_required(VERSION 3.6)

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

# Install the library
install(TARGETS ImGuiLib DESTINATION lib)
