cmake_minimum_required(VERSION 3.6)
project(Languages)
set(lang_root_dir ${KAI_SOURCE_ROOT_DIR}/Library/Language)

# CommonLang now comes from the CppKaiCore submodule (Core + Executor + Common).

# Pi and Rho are always built. Tau is the IDL compiler and requires networking.
set(core_langs Pi Rho)
foreach(l ${core_langs})
    def_lang_library(${l})
    target_link_libraries(${l}Lang CommonLang Executor)
endforeach()

if(KAI_NETWORKING)
    def_lang_library(Tau)
    target_link_libraries(TauLang CommonLang Executor)
endif()
