cmake_minimum_required(VERSION 3.6)
project(Languages)
set(lang_root_dir ${KAI_SOURCE_ROOT_DIR}/Library/Language)

# Add Common language source files explicitly
set(COMMON_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Source/Language.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common/Source/Structure.cpp
)

# Create a separate library for Common language components
add_library(CommonLang ${COMMON_SOURCES})

# Create the main language libraries
set(langs Pi Rho Tau)
foreach(l ${langs})
    def_lang_library(${l})
    # Link each language with the common library
    target_link_libraries(${l}Lang CommonLang)
endforeach()
