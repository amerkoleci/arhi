cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "In-source builds are not supported. Please choose a different binary directory.")
	return()
endif()

# Only 64 bit is supported
if (CMAKE_SIZEOF_VOID_P EQUAL 4)
	message(FATAL_ERROR "32 bit isn't supported")
    return()
endif()

# Determine if engine is built as a subproject (using add_subdirectory)
# or if it is the master project.
if (NOT DEFINED ARHI_MASTER_PROJECT)
    set(ARHI_MASTER_PROJECT OFF)
    if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(ARHI_MASTER_PROJECT ON)
        message(STATUS "CMake version: ${CMAKE_VERSION}")
    endif ()
endif ()

# Use solution folders to organize projects
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Configure CMake global variables
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INSTALL_MESSAGE LAZY)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_DEBUG_POSTFIX "_d")

# Set default symbol visibility to hidden. This prevents all Jolt's symbols to be exported on macOS. This can cause crashes if a program loads multiple shared libraries with different versions of Jolt.
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_OSX_DEPLOYMENT_TARGET "11" CACHE STRING "Minimum macOS deployment version")
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for OS X")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# Build version
set(ARHI_VERSION_MAJOR 1)
set(ARHI_VERSION_MINOR 0)
set(ARHI_VERSION_PATCH 0)
set(ARHI_VERSION ${ARHI_VERSION_MAJOR}.${ARHI_VERSION_MINOR}.${ARHI_VERSION_PATCH})

# Project declaration
project(arhi
    VERSION ${ARHI_VERSION}
    DESCRIPTION "A low-level cross platform Render Hardware Interface (RHI) library"
    HOMEPAGE_URL "https://github.com/amerkoleci/arhi"
    LANGUAGES C CXX
)

if (MSVC AND ARHI_MASTER_PROJECT)
    if (NOT DEFINED CMAKE_MSVC_RUNTIME_LIBRARY)
        set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif ()
endif ()

# Add the cmake directory to the module path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include common utilities
include(GNUInstallDirs)
include(CMakeDependentOption)
include(FetchPackage)
include(CPM)

# Define standard configurations (Debug, Release)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# Set the default CMAKE_BUILD_TYPE to Release.
# This should be done before the project command since the latter can set
# CMAKE_BUILD_TYPE itself (it does so for nmake).
if (ARHI_MASTER_PROJECT AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The default build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})
endif ()

execute_process(COMMAND git rev-list --count HEAD
    OUTPUT_VARIABLE ARHI_REVISION_ID
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if ("${ARHI_REVISION_ID}" STREQUAL "")
    # We don't have any source control so just hard-code all of this information to empty
    set (ARHI_REVISION_ID 0)
    set (ARHI_SHORT_CHANGE_SET 0)
    set (ARHI_CHANGE_SET 0)
    set (ARHI_CHANGE_SET_DATE "")
else ()
    execute_process(COMMAND git rev-parse --short HEAD
        OUTPUT_VARIABLE ARHI_SHORT_CHANGE_SET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(COMMAND git rev-parse HEAD
        OUTPUT_VARIABLE ARHI_CHANGE_SET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(COMMAND git log -1 --abbrev=12 --date=format:%Y-%m-%d --pretty=format:%cd
        OUTPUT_VARIABLE ARHI_CHANGE_SET_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif ()

# Setting Platform
if(EMSCRIPTEN)
    set(ARHI_PLATFORM_NAME "Web")
elseif(ANDROID)
    set(ARHI_PLATFORM_NAME "Android")
elseif(IOS)
    set(ARHI_PLATFORM_NAME "iOS")
elseif(APPLE)
    set(ARHI_PLATFORM_NAME "macOS")
elseif(WIN32)
    set(ARHI_PLATFORM_NAME "Win32")
elseif(UNIX)
    set(ARHI_PLATFORM_NAME "Unix")
else()
    message(FATAL_ERROR "Unrecognized platform: ${CMAKE_SYSTEM_NAME}")
endif()

# Setting Architecture
if (DEFINED VCPKG_TARGET_ARCHITECTURE)
    set(ARHI_ARCH ${VCPKG_TARGET_ARCHITECTURE})
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Ww][Ii][Nn]32$")
    set(ARHI_ARCH x86)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Xx]64$")
    set(ARHI_ARCH x64)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Aa][Rr][Mm]$")
    set(ARHI_ARCH arm)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Aa][Rr][Mm]64$")
    set(ARHI_ARCH arm64)
elseif(CMAKE_GENERATOR_PLATFORM MATCHES "^[Aa][Rr][Mm]64EC$")
    set(ARHI_ARCH arm64ec)
elseif(CMAKE_VS_PLATFORM_NAME_DEFAULT MATCHES "^[Ww][Ii][Nn]32$")
    set(ARHI_ARCH x86)
elseif(CMAKE_VS_PLATFORM_NAME_DEFAULT MATCHES "^[Xx]64$")
    set(ARHI_ARCH x64)
elseif(CMAKE_VS_PLATFORM_NAME_DEFAULT MATCHES "^[Aa][Rr][Mm]$")
    set(ARHI_ARCH arm)
elseif(CMAKE_VS_PLATFORM_NAME_DEFAULT MATCHES "^[Aa][Rr][Mm]64$")
    set(ARHI_ARCH arm64)
elseif(CMAKE_VS_PLATFORM_NAME_DEFAULT MATCHES "^[Aa][Rr][Mm]64EC$")
    set(ARHI_ARCH arm64ec)
endif()

# Determines host architecture
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "[Aa][Rr][Mm]64|aarch64|arm64")
    set(ARHI_HOST_ARCH arm64)
else()
    set(ARHI_HOST_ARCH x64)
endif()

# Detect use of the clang-cl.exe frontend, which does not support all of clangs normal options
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    if ("${CMAKE_CXX_SIMULATE_ID}" STREQUAL "MSVC")
        set(MSVC_CLANG TRUE)
    endif()
endif()

# Set general compiler flags
if (MSVC)
    # Multiprocessor compiler
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive- /JMC- /Zc:__cplusplus  /Zc:inline")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gm- /nologo /diagnostics:classic /FC /fp:except-")

    # Set compiler flag for disabling RTTI
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")

    # Disable exceptions
	# Remove any existing compiler flag that enables exceptions
	string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

	# Disable warning about STL and compiler-generated types using noexcept when exceptions are disabled
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4577")

    # Disable warning about C++ exception handler used, but unwind semantics not enabled
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4530")

    if (CMAKE_INTERPROCEDURAL_OPTIMIZATION)
      # Enable function-level linking
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Gy /Gw")
    endif()
else()
    # Set compiler flag for disabling RTTI
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

	# Set compiler flag for disabling exception-handling
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")

    if (EMSCRIPTEN)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os")
    elseif(ANDROID)
    endif()
endif ()

# Library type
if (IOS OR EMSCRIPTEN)
    set(ARHI_BUILD_SHARED OFF CACHE BOOL "Always Disable shared library on (IOS, WEB)" FORCE)
elseif(ANDROID)
    set(ARHI_BUILD_SHARED ON CACHE BOOL "Always enable shared library on (ANDROID)" FORCE)
else ()
    option(ARHI_BUILD_SHARED "Build engine as shared library" ON)
endif ()

option(ARHI_INSTALL "Generate the install target" ${ARHI_MASTER_PROJECT})
option(ARHI_BUILD_SAMPLES "Build samples" ${ARHI_MASTER_PROJECT})

# Determine available GPU backends
if (WIN32)
    set(ARHI_HAS_D3D12 ON) 
    set(ARHI_HAS_VULKAN ON)
    set(ARHI_HAS_METAL OFF)
elseif(APPLE)
    set(ARHI_HAS_METAL ON)
    set(ARHI_HAS_D3D12 OFF)
    set(ARHI_HAS_VULKAN OFF)
elseif(UNIX OR ANDROID)
    set(ARHI_HAS_VULKAN ON)
    set(ARHI_HAS_D3D12 OFF)
    set(ARHI_HAS_METAL OFF)
endif()

cmake_dependent_option(ARHI_VULKAN "Enable Vulkan backend" ON "ARHI_HAS_VULKAN" OFF)
cmake_dependent_option(ARHI_D3D12 "Enable D3D12 backend" ON "ARHI_HAS_D3D12" OFF)
cmake_dependent_option(ARHI_METAL "Enable Metal backend" ON "ARHI_HAS_METAL" OFF)

# Print current build configuration
message(STATUS "Engine version: ${ARHI_VERSION}")
message(STATUS "GIT Revision ID: ${ARHI_REVISION_ID}")
message(STATUS "GIT Short ChangeSet: ${ARHI_SHORT_CHANGE_SET}")
message(STATUS "GIT ChangeSet: ${ARHI_CHANGE_SET}")
message(STATUS "GIT ChangeSet Date: ${ARHI_CHANGE_SET_DATE}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

message(STATUS "Detected platform: ${ARHI_PLATFORM_NAME}")
message(STATUS "Detected architecture: ${ARHI_ARCH}")
message(STATUS "Detected host architecture: ${ARHI_HOST_ARCH}")

if (ARHI_BUILD_SHARED)
    set(ARHI_LIBRARY_TYPE SHARED)
    message(STATUS "  Library         SHARED")
else ()
    set(ARHI_LIBRARY_TYPE STATIC)
    message(STATUS "  Library         STATIC")
endif ()

if (ARHI_VULKAN)
    message(STATUS "      - Vulkan:      ARHI_VULKAN")
endif()
if (ARHI_D3D12)
    message(STATUS "      - D3D12:       ARHI_D3D12")
endif ()
if (ARHI_METAL)
    message(STATUS "      - Metal:      ARHI_METAL")
endif()

message(STATUS "  Install         ${ARHI_INSTALL}")
message(STATUS "  Samples         ${ARHI_BUILD_SAMPLES}")

# Add subdirectories
add_subdirectory(third_party)

# Setup the main library
set(TARGET_NAME arhi)

set(SOURCE_FILES
    include/arhi.h
    src/arhi_internal.h
    src/arhi.cpp
    src/arhi_check.c
)

add_library(${TARGET_NAME} ${ARHI_LIBRARY_TYPE} ${SOURCE_FILES})
target_include_directories(${TARGET_NAME}
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_compile_definitions(${TARGET_NAME} PRIVATE ARHI_IMPLEMENTATION)
if (ARHI_BUILD_SHARED)
    target_compile_definitions (${TARGET_NAME} PRIVATE ARHI_SHARED_LIBRARY)
endif ()

if (ARHI_VULKAN)
    target_sources(${TARGET_NAME} PRIVATE
        src/vulkan/arhi_vulkan_funcs.h
        src/vulkan/arhi_vulkan.cpp
    )

    target_link_libraries(${TARGET_NAME} PRIVATE vulkan)
    target_compile_definitions (${TARGET_NAME} PRIVATE ARHI_VULKAN)
endif()

if (ARHI_D3D12)
    target_sources(${TARGET_NAME} PRIVATE
        src/d3d12/arhi_d3d12.cpp
    )
    target_link_libraries(${TARGET_NAME} PRIVATE DirectX-Headers)
    target_compile_definitions (${TARGET_NAME} PRIVATE ARHI_D3D12)
endif()

if (ARHI_METAL)
    target_sources(${TARGET_NAME} PRIVATE
        src/metal/arhi_metal.cpp
    )
    target_compile_definitions (${TARGET_NAME} PRIVATE ARHI_METAL)
    target_link_libraries(${TARGET_NAME} PRIVATE arhi-metal-cpp)
endif()

# Add samples subdirectoy
#add_subdirectory(samples)

# README and LICENSE
if(ARHI_INSTALL)
    message(STATUS "Generating install")
    install (FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/README.md
        ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
        DESTINATION .
    )
endif ()
