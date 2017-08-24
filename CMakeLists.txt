#-------------------------------------------------------------------------------
# libingenialink - IngeniaLink Protocol Communications Library
#
# MIT License
#
# Copyright (c) 2017 Ingenia-CAT S.L.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

#-------------------------------------------------------------------------------
# General

cmake_minimum_required(VERSION 3.0)
project(ingenialink LANGUAGES C VERSION 0.9.9)

set(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake
)

#-------------------------------------------------------------------------------
# Compiler options

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi")
elseif(${CMAKE_C_COMPILER_ID} MATCHES "(GNU|Clang)")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wextra -pedantic")
endif()

#-------------------------------------------------------------------------------
# Build options

option(WITH_GITINFO     "Include Git commit hash in version"      OFF)
option(WITH_EXAMPLES    "Build library usage example apps"        OFF)
option(WITH_DOCS        "Build library public API documentation"  OFF)
option(WITH_ERRDESC     "Support description details of errors"   ON)
option(WITH_SERCOMM     "Automatically fetch and embed sercomm"   ON)

if(WITH_GITINFO)
  find_package(Git REQUIRED)

  execute_process(
    COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PROJECT_VERSION_EXTRA
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

if(NOT BUILD_SHARED_LIBS)
  set(IL_STATIC ON)
endif()

include(TestBigEndian)
TEST_BIG_ENDIAN(IL_BIG_ENDIAN)

configure_file("config.h.in" "${CMAKE_BINARY_DIR}/config.h")

#-------------------------------------------------------------------------------
# Library

# Sources
set(ingenialink_srcs
  ingenialink/err.c
  ingenialink/frame.c
  ingenialink/net.c
  ingenialink/node.c
)

add_library(ingenialink ${ingenialink_srcs})

# Includes
target_include_directories(
  ingenialink
  PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/public>"
    "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:include>"
)

# Definitions (common)
target_compile_definitions(ingenialink PRIVATE IL_BUILDING)

# Definitions (conditional)
if(WITH_ERRDESC)
  target_compile_definitions(ingenialink PRIVATE IL_WITH_ERRDESC)
endif()

#-------------------------------------------------------------------------------
# Dependencies

if(WITH_SERCOMM)
  include(ExternalProject)

  set(SERCOMM_PREFIX ${CMAKE_BINARY_DIR}/sercomm)
  set(SERCOMM_INSTALL ${SERCOMM_PREFIX}/install)

  ExternalProject_Add(sercomm_lib
    GIT_REPOSITORY
      https://github.com/ingeniamc/sercomm.git
    PREFIX
      ${SERCOMM_PREFIX}
    CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX=${SERCOMM_INSTALL} -DWITH_PIC=ON
  )

  add_dependencies(ingenialink sercomm_lib)

  # add headers and library to ingenialink
  set(SERCOMM_INCLUDES ${SERCOMM_INSTALL}/include)
  set(SERCOMM_LIBNAME
    ${CMAKE_STATIC_LIBRARY_PREFIX}sercomm.${CMAKE_STATIC_LIBRARY_SUFFIX}
  )
  set(SERCOMM_LIB ${SERCOMM_INSTALL}/lib/${SERCOMM_LIBNAME})

  target_include_directories(ingenialink PRIVATE ${SERCOMM_INCLUDES})
  target_link_libraries(ingenialink PRIVATE ${SERCOMM_LIB})
else()
  find_package(sercomm CONFIG REQUIRED)
  target_link_libraries(ingenialink PRIVATE sercomm)
endif()

#-------------------------------------------------------------------------------
# Checks

include(StyleCheck)
if(STYLE_CHECK_AVAILABLE)
  add_style_check("ingenialink/*.c" "ingenialink/*.h" "include/**/*.h")
endif()

#-------------------------------------------------------------------------------
# Examples

if(WITH_EXAMPLES)
  add_subdirectory(examples)
endif()

#-------------------------------------------------------------------------------
# Documentation

if(WITH_DOCS)
  find_package(Doxygen REQUIRED)

  # Doxyfile variables
  set(DOCS_TMPL_DIR ${CMAKE_SOURCE_DIR}/docs/template)
  set(DOCS_OUT_DIR ${CMAKE_BINARY_DIR}/docs)

  set(DOCS_EXAMPLES_DIR ${CMAKE_SOURCE_DIR}/examples)
  set(DOCS_SRCS_LST
    ${CMAKE_SOURCE_DIR}/README.md
    ${CMAKE_SOURCE_DIR}/include/public
    ${CMAKE_SOURCE_DIR}/examples
  )

  string(REPLACE ";" " " DOCS_SRCS "${DOCS_SRCS_LST}")

  # Generate Doxyfile
  configure_file(
    "docs/template/Doxyfile.in"
    "${CMAKE_BINARY_DIR}/Doxyfile"
  )

  # Add 'docs' target
  add_custom_target(
    docs
    COMMAND
      ${DOXYGEN_EXECUTABLE}
    COMMENT "Building documentation" VERBATIM
  )
endif()

#-------------------------------------------------------------------------------
# Installation

# CMake files
include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_SOURCE_DIR}/cmake/Config.cmake.in"
  "${CMAKE_BINARY_DIR}/ingenialinkConfig.cmake"
  INSTALL_DESTINATION "lib/cmake/ingenialink"
)

write_basic_package_version_file(
  "${CMAKE_BINARY_DIR}/ingenialinkConfigVersion.cmake"
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    "${CMAKE_BINARY_DIR}/ingenialinkConfig.cmake"
    "${CMAKE_BINARY_DIR}/ingenialinkConfigVersion.cmake"
  DESTINATION
    "lib/cmake/ingenialink"
  COMPONENT cmake
)

install(
  EXPORT "ingenialinkTargets"
  DESTINATION "lib/cmake/ingenialink"
  COMPONENT cmake
)
# Library
install(
  TARGETS ingenialink
  EXPORT "ingenialinkTargets"
  LIBRARY DESTINATION "lib" COMPONENT libraries
  ARCHIVE DESTINATION "lib" COMPONENT archive
  RUNTIME DESTINATION "bin" COMPONENT binaries
  INCLUDES DESTINATION "include"
)

# Headers
install(
  DIRECTORY
    "${CMAKE_SOURCE_DIR}/include/public/ingenialink"
  DESTINATION
    "include"
  COMPONENT headers
)

install(
  FILES
    "${CMAKE_BINARY_DIR}/config.h"
  DESTINATION
    "include/ingenialink"
  COMPONENT headers
)

# Documentation
if(WITH_DOCS)
  if(EXISTS ${DOCS_OUT_DIR})
    install(
      DIRECTORY ${DOCS_OUT_DIR}
      DESTINATION "share/ingenialink"
      COMPONENT documentation
    )
  endif()
endif()

#-------------------------------------------------------------------------------
# Packaging

if(WITH_GITINFO)
  set(PACKAGE_VERSION_EXTRA "-${PROJECT_VERSION_EXTRA}")
endif()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}${PACKAGE_VERSION_EXTRA}")

if(WIN32)
  set(CPACK_GENERATOR "ZIP")
else()
  set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
