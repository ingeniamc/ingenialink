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
project(ingenialink LANGUAGES C VERSION 4.1.0)

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

option(WITH_EXAMPLES     "Build library usage example apps"                   OFF)
option(WITH_DOCS         "Build library public API documentation"             OFF)
option(WITH_PIC          "Generate position independent code"                 OFF)
option(WITH_PROT_EUSB    "Build with EUSB protocol support"                    ON)
option(WITH_PROT_MCB     "Build with MCB protocol support"                     ON)
option(WITH_PROT_ETH     "Build with ETH protocol support"                     ON)
option(WITH_PROT_ECAT     "Build with ECAT protocol support (EXPERIMENTAL)"     ON)
option(WITH_PROT_VIRTUAL "Build with Virtual protocol support (EXPERIMENTAL)" OFF)

# Build a shared library by default
if(NOT DEFINED BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS ON)
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
  ingenialink/dict.c
  ingenialink/dict_labels.c
  ingenialink/err.c
  ingenialink/net.c
  ingenialink/poller.c
  ingenialink/servo.c
  ingenialink/utils.c
  ingenialink/version.c
)

# Sources (protocols)
list(APPEND ingenialink_srcs
  ingenialink/base/net.c
  ingenialink/base/servo.c
)

if(WITH_PROT_EUSB)
  list(APPEND ingenialink_srcs
    ingenialink/eusb/net.c
    ingenialink/eusb/frame.c
    ingenialink/eusb/monitor.c
    ingenialink/eusb/registers.c
    ingenialink/eusb/servo.c
  )
endif()

if(WITH_PROT_MCB)
  list(APPEND ingenialink_srcs
    ingenialink/mcb/frame.c
    ingenialink/mcb/net.c
    ingenialink/mcb/servo.c
    ingenialink/mcb/registers.c
  )
endif()

if(WITH_PROT_ETH)
  list(APPEND ingenialink_srcs
    ingenialink/eth/frame.c
    ingenialink/eth/net.c
    ingenialink/eth/servo.c
  )
endif()

if(WITH_PROT_ECAT)
  list(APPEND ingenialink_srcs
    ingenialink/ecat/frame.c
    ingenialink/ecat/net.c
    ingenialink/ecat/servo.c

    

    ingenialink/ecat/lwip.c
		ingenialink/ecat/lwip/def.c
		ingenialink/ecat/lwip/etharp.c
		ingenialink/ecat/lwip/ethernet.c
		ingenialink/ecat/lwip/inet_chksum.c
		ingenialink/ecat/lwip/init.c
		ingenialink/ecat/lwip/ip.c
		ingenialink/ecat/lwip/ip4_addr.c
		ingenialink/ecat/lwip/ip4_frag.c
		ingenialink/ecat/lwip/ip4.c
		ingenialink/ecat/lwip/mem.c
		ingenialink/ecat/lwip/memp.c
		ingenialink/ecat/lwip/netif.c
		ingenialink/ecat/lwip/pbuf.c
		ingenialink/ecat/lwip/stats.c
		ingenialink/ecat/lwip/timeouts.c
		ingenialink/ecat/lwip/udp.c

    
    ingenialink/ecat/lwipopts.h
		ingenialink/ecat/lwip/etharp.h
		ingenialink/ecat/lwip/prot/etharp.h
		ingenialink/ecat/lwip/prot/ethernet.h
		ingenialink/ecat/lwip/inet_chksum.h
		ingenialink/ecat/lwip/init.h
		ingenialink/ecat/lwip/ip.h
		ingenialink/ecat/lwip/ip4_addr.h
		ingenialink/ecat/lwip/ip4_frag.h
		ingenialink/ecat/lwip/ip4.h
		ingenialink/ecat/lwip/mem.h
		ingenialink/ecat/lwip/memp.h
		ingenialink/ecat/lwip/netif.h
		ingenialink/ecat/lwip/pbuf.h
		ingenialink/ecat/lwip/stats.h
		ingenialink/ecat/lwip/timeouts.h
		ingenialink/ecat/lwip/udp.h
		
		ingenialink/ecat/lwip.h
		ingenialink/ecat/lwip/opt.h
		ingenialink/ecat/lwip/err.h
		ingenialink/ecat/lwip/sys.h

    external/SOEM/soem/ethercatmain.h
  )
endif()

if(WITH_PROT_VIRTUAL)
  list(APPEND ingenialink_srcs
    ingenialink/virtual/net.c
    ingenialink/virtual/servo.c
  )
endif()

# Sources (osal)
if(UNIX)
  list(APPEND ingenialink_srcs
    osal/posix/clock.c
    osal/posix/cond.c
    osal/posix/mutex.c
    osal/posix/thread.c
    osal/posix/timer.c
  )
elseif(WIN32)
  list(APPEND ingenialink_srcs
    osal/win/clock.c
    osal/win/cond.c
    osal/win/mutex.c
    osal/win/thread.c
    osal/win/timer.c
  )
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

add_library(ingenialink ${ingenialink_srcs})

# Options
if(WITH_PIC)
  set_target_properties(ingenialink PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

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

# Definitions
target_compile_definitions(ingenialink PRIVATE IL_BUILDING)

if(WITH_PROT_EUSB)
  target_compile_definitions(ingenialink PRIVATE IL_HAS_PROT_EUSB)
endif()

if(WITH_PROT_MCB)
  target_compile_definitions(ingenialink PRIVATE IL_HAS_PROT_MCB)
endif()

if(WITH_PROT_ETH)
  include_directories(./ ./ingenialink/ecat/lwip)
  target_compile_definitions(ingenialink PRIVATE IL_HAS_PROT_ETH)
endif()

if(WITH_PROT_ECAT)
  target_compile_definitions(ingenialink PRIVATE IL_HAS_PROT_ECAT)
endif()

if(WITH_PROT_VIRTUAL)
  target_compile_definitions(ingenialink PRIVATE IL_HAS_PROT_VIRTUAL)
endif()

# Dependencies
find_package(sercomm CONFIG REQUIRED)
target_link_libraries(ingenialink PRIVATE sercomm)

find_package(LibXml2 REQUIRED)
target_link_libraries(ingenialink PRIVATE ${LIBXML2_LIBRARIES})
target_include_directories(ingenialink PRIVATE ${LIBXML2_INCLUDE_DIR})


target_link_libraries(ingenialink PRIVATE ../external/SOEM/_build/soem)

target_include_directories(ingenialink PRIVATE ./external/SOEM/soem)
target_include_directories(ingenialink PRIVATE ./external/SOEM/osal)
target_include_directories(ingenialink PRIVATE ./external/SOEM/osal/win32)

target_include_directories(ingenialink PRIVATE ./external/SOEM/oshw/win32)
target_include_directories(ingenialink PRIVATE ./external/SOEM/oshw/win32/wpcap/Include)
target_include_directories(ingenialink PRIVATE ./external/SOEM/oshw/win32/wpcap/Include/pcap)

target_link_libraries(ingenialink PRIVATE ../external/SOEM/oshw/win32/wpcap/Lib/wpcap)
target_link_libraries(ingenialink PRIVATE ../external/SOEM/WinMM)

if(UNIX)
  # FIX: disable TRY_RUN for pthreads when cross-compiling
  if(CMAKE_CROSSCOMPILING)
    set(THREADS_PTHREAD_ARG OFF)
  endif()
  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)

  target_link_libraries(ingenialink PRIVATE ${CMAKE_THREAD_LIBS_INIT})
elseif(WIN32)
  # required by: libxml2
  target_link_libraries(ingenialink PRIVATE ws2_32)
  
endif()

#-------------------------------------------------------------------------------
# Checks

include(StyleCheck)
if(STYLE_CHECK_AVAILABLE)
  add_style_check(
    "ingenialink/*.c"
    "ingenialink/*.h"
    "osal/**/*.c"
    "osal/**/*.h"
    "include/ingenialink/**/*.h"
    "include/osal/**/*.h"
    "include/public/**/*.h")
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
    "docs/Doxyfile.in"
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

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

if(WIN32)
  set(CPACK_GENERATOR "ZIP")
else()
  set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
