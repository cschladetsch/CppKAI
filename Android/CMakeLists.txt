cmake_minimum_required(VERSION 3.20)

project(KAIAndroid LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(KAI_ANDROID ON CACHE BOOL "Build the Android KAI runtime subset" FORCE)
set(KAI_HOME "${CMAKE_CURRENT_LIST_DIR}/..")
set(SOURCE_HOME "${KAI_HOME}/Source")
set(LIBRARY_HOME "${SOURCE_HOME}/Library")
set(EXTERNAL "${KAI_HOME}/Ext")

if(NOT DEFINED BIN_HOME)
    set(BIN_HOME "${CMAKE_BINARY_DIR}/Bin")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${BIN_HOME}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${BIN_HOME}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BIN_HOME}")

add_compile_definitions(
    KAI_ANDROID
    KAI_USE_NAMESPACES
    KAI_NAMESPACE_NAME=kai
    KAI_USE_EXCEPTIONS
    KAI_BOOST_UNORDERED_REGISTRY
    KAI_FORMAT_COMPATIBLE
    KAI_DEBUG
    KAI_USE_ENET
    KAI_LOG_DIR="./Logs"
    KAI_VERSION_MAJOR=0
    KAI_VERSION_MINOR=3
    KAI_VERSION_PATCH=0)

include_directories(
    "${KAI_HOME}/Include"
    "${EXTERNAL}/CppKaiCore/Include"
    "${EXTERNAL}/CppKaiLanguage/Include"
    "${EXTERNAL}/ENet/include"
    "${EXTERNAL}/rang/include")

add_subdirectory("${EXTERNAL}/ENet" "${CMAKE_BINARY_DIR}/enet")
add_subdirectory("${EXTERNAL}/CppKaiCore" "${CMAKE_BINARY_DIR}/CppKaiCore")
add_subdirectory("${EXTERNAL}/CppKaiLanguage" "${CMAKE_BINARY_DIR}/CppKaiLanguage")

file(GLOB_RECURSE kai_network_src
    "${LIBRARY_HOME}/Network/Source/*.cpp")
add_library(Network STATIC ${kai_network_src})
target_include_directories(Network PUBLIC
    "${KAI_HOME}/Include"
    "${KAI_HOME}/Include/KAI/Network"
    "${EXTERNAL}/ENet/include")
target_compile_definitions(Network PRIVATE KAI_NETWORK_DLL)
target_link_libraries(Network PUBLIC Core enet)

add_library(ConsoleLib STATIC
    "${EXTERNAL}/CppKaiCore/Source/Library/Executor/Source/Console.cpp")
target_include_directories(ConsoleLib PUBLIC "${KAI_HOME}/Include")
target_link_libraries(ConsoleLib PUBLIC
    RhoLang PiLang Executor CommonLang Core Network enet)

add_library(kai SHARED "${CMAKE_CURRENT_LIST_DIR}/jni/Kai_jni.cpp")
target_link_libraries(kai PRIVATE
    ConsoleLib RhoLang PiLang Executor CommonLang Core Network enet)

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    target_link_libraries(kai PRIVATE log android)
else()
    find_package(JNI REQUIRED)
    target_include_directories(kai PRIVATE ${JNI_INCLUDE_DIRS})
endif()

set_target_properties(kai PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${BIN_HOME}")
