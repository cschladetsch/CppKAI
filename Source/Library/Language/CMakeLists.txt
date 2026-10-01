cmake_minimum_required(VERSION 3.28...3.30)
project(Languages)
set(lang_root_dir ${KAI_SOURCE_ROOT_DIR}/Library/Language)

# CommonLang now comes from the CppKaiCore submodule (Core + Executor + Common).

# Pi and Rho now come from the CppKaiLanguage submodule (downstream of
# CppKaiCore). Tau is the IDL compiler and requires networking.
add_subdirectory(${KAI_HOME}/Ext/CppKaiLanguage ${CMAKE_BINARY_DIR}/CppKaiLanguage)

# Sigma: statically typed, compiles to Rho (see Doc/SigmaLanguage.md).
def_lang_library(Sigma)
target_include_directories(SigmaLang PUBLIC ${KAI_HOME}/Include)
target_link_libraries(SigmaLang PUBLIC RhoLang PiLang CommonLang Executor Core)

if(KAI_NETWORKING AND NOT KAI_ANDROID)
    def_lang_library(Tau)
    target_link_libraries(TauLang CommonLang Executor)
endif()
