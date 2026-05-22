#set(Boost_DEBUG 1)

message(STATUS "Looking for boost")

if (WIN32)
	set(Boost_NO_SYSTEM_PATHS on CACHE BOOL "Do not search system for Boost")
	if (EXISTS "c:/boost/boost_1_84_0/boost/version.hpp")
		set(_kai_boost_root "c:/boost/boost_1_84_0")
	else()
		set(_kai_boost_root "c:/boost/boost_1_75_0")
	endif()
	set(BOOST_ROOT "${_kai_boost_root}" CACHE PATH "Boost library path" FORCE)
	set(Boost_INCLUDE_DIR "${_kai_boost_root}" CACHE PATH "Boost include path" FORCE)
	set(Boost_LIBRARY_DIR "${_kai_boost_root}/stage/lib" CACHE PATH "Boost library path" FORCE)
endif()

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_DEBUG_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.67.0 REQUIRED COMPONENTS
    chrono filesystem system date_time)

set(KAI_BOOST_PROGRAM_OPTIONS FALSE)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(_kai_boost_arch_suffix "x64")
else()
	set(_kai_boost_arch_suffix "x32")
endif()
unset(KAI_BOOST_PROGRAM_OPTIONS_DEBUG CACHE)
unset(KAI_BOOST_PROGRAM_OPTIONS_RELEASE CACHE)
find_library(KAI_BOOST_PROGRAM_OPTIONS_DEBUG
	NAMES "libboost_program_options-vc143-mt-gd-${_kai_boost_arch_suffix}-1_84"
	PATHS "${Boost_LIBRARY_DIR}"
	NO_DEFAULT_PATH)
find_library(KAI_BOOST_PROGRAM_OPTIONS_RELEASE
	NAMES "libboost_program_options-vc143-mt-${_kai_boost_arch_suffix}-1_84"
	PATHS "${Boost_LIBRARY_DIR}"
	NO_DEFAULT_PATH)

if (KAI_BOOST_PROGRAM_OPTIONS_DEBUG AND KAI_BOOST_PROGRAM_OPTIONS_RELEASE)
	set(KAI_BOOST_PROGRAM_OPTIONS TRUE)
	set(KAI_BOOST_PROGRAM_OPTIONS_LIBRARIES
		debug "${KAI_BOOST_PROGRAM_OPTIONS_DEBUG}"
		optimized "${KAI_BOOST_PROGRAM_OPTIONS_RELEASE}")
endif()

message(STATUS "====> Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
message(STATUS "====> Boost_LIBRARY_DIR_DEBUG: ${Boost_LIBRARY_DIR_DEBUG}")
message(STATUS "====> Boost_LIBRARIES: ${Boost_LIBRARIES}")

include_directories(${Boost_INCLUDE_DIRS})
