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

# Pi, Rho and Lisp are always built. Tau is the IDL compiler and requires
# networking.
set(core_langs Pi Rho Lisp)
foreach(l ${core_langs})
    def_lang_library(${l})
    target_link_libraries(${l}Lang CommonLang Executor)
endforeach()

if(KAI_NETWORKING)
    def_lang_library(Tau)
    target_link_libraries(TauLang CommonLang Executor)
endif()
