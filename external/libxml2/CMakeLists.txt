#-------------------------------------------------------------------------------
# CMake build script for libxml2
#
# MIT License
#
# Copyright (c) 2018 Ingenia-CAT S.L.
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
project(xml2 LANGUAGES C VERSION 2.9.7)

set(PROJECT_VERSION_NUMBER 209070)
set(PROJECT_VERSION_EXTRA "")

#-------------------------------------------------------------------------------
# Compiler options

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4996 /D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi")
elseif(${CMAKE_C_COMPILER_ID} MATCHES "(GNU|Clang)")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall")
endif()

#-------------------------------------------------------------------------------
# Build options

option(WITH_PIC "Generate position independent code" OFF)

# Build a shared library by default
if(NOT DEFINED BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS ON)
endif()

# xmlversion.h options
# TODO: select sources based on these options
option(WITH_TRIO
       "defined if the trio support need to be configured in" OFF)
option(LIBXML_THREAD_ENABLED
        "Whether the thread support is configured in" ON)
option(LIBXML_THREAD_ALLOC_ENABLED
       " Whether the allocation hooks are per-thread" OFF)
option(LIBXML_TREE_ENABLED
       "Whether the DOM like tree manipulation API support is configured in" ON)
option(LIBXML_OUTPUT_ENABLED
       "Whether the serialization/saving support is configured in" ON)
option(LIBXML_PUSH_ENABLED
       "Whether the push parsing interfaces are configured in" ON)
option(LIBXML_READER_ENABLED
       "Whether the xmlReader parsing interface is configured in" ON)
option(LIBXML_PATTERN_ENABLED
       "Whether the xmlPattern node selection interface is configured in" ON)
option(LIBXML_WRITER_ENABLED
       " Whether the xmlWriter saving interface is configured in" ON)
option(LIBXML_SAX1_ENABLED
       "Whether the older SAX1 interface is configured in" ON)
option(LIBXML_FTP_ENABLED
       "Whether the FTP support is configured in" ON)
option(LIBXML_HTTP_ENABLED
       "Whether the HTTP support is configured in" ON)
option(LIBXML_VALID_ENABLED
       "Whether the DTD validation support is configured in" ON)
option(LIBXML_HTML_ENABLED
       "Whether the HTML support is configured in" ON)
option(LIBXML_LEGACY_ENABLED
       "Whether the deprecated APIs are compiled in for compatibility" ON)
option(LIBXML_C14N_ENABLED
       "Whether the Canonicalization support is configured in" ON)
option(LIBXML_CATALOG_ENABLED
       "Whether the Catalog support is configured in" ON)
option(LIBXML_DOCB_ENABLED
       "Whether the SGML Docbook support is configured in" ON)
option(LIBXML_XPATH_ENABLED
       "Whether XPath is configured in" ON)
option(LIBXML_XPTR_ENABLED
       "Whether XPointer is configured in" ON)
option(LIBXML_XINCLUDE_ENABLED
       "Whether XInclude is configured in" ON)
option(LIBXML_ICONV_ENABLED
       "Whether iconv support is available" OFF)
option(LIBXML_ICU_ENABLED
       "Whether icu support is available" OFF)
option(LIBXML_ISO8859X_ENABLED
       "Whether ISO-8859-* support is made available in case iconv is not" OFF)
option(LIBXML_DEBUG_ENABLED
       "Whether Debugging module is configured in" ON)
option(DEBUG_MEMORY_LOCATION
       "Whether the memory debugging is configured in" OFF)
option(LIBXML_DEBUG_RUNTIME
       "Whether the runtime debugging is configured in" OFF)
option(LIBXML_UNICODE_ENABLED
       "Whether the Unicode related interfaces are compiled in" ON)
option(LIBXML_REGEXP_ENABLED
       "Whether the regular expressions interfaces are compiled in" ON)
option(LIBXML_AUTOMATA_ENABLED
       "Whether the automata interfaces are compiled in" ON)
option(LIBXML_EXPR_ENABLED
       "Whether the formal expressions interfaces are compiled in" ON)
option(LIBXML_SCHEMAS_ENABLED
       "Whether the Schemas validation interfaces are compiled in" ON)
option(LIBXML_SCHEMATRON_ENABLED
       "Whether the Schematron validation interfaces are compiled in" ON)
option(LIBXML_MODULES_ENABLED
       "Whether the module interfaces are compiled in" ON)
option(LIBXML_ZLIB_ENABLED
       "Whether the Zlib support is compiled in" OFF)
option(LIBXML_LZMA_ENABLED
       "Whether the Lzma support is compiled in" OFF)

configure_file("${CMAKE_SOURCE_DIR}/xmlversion.h.in"
               "${CMAKE_SOURCE_DIR}/libxml2/include/libxml/xmlversion.h")

#-------------------------------------------------------------------------------
# Library

# Sources
set(xml2_srcs
  libxml2/buf.c
  libxml2/c14n.c
  libxml2/catalog.c
  libxml2/chvalid.c
  libxml2/debugXML.c
  libxml2/dict.c
  libxml2/DOCBparser.c
  libxml2/encoding.c
  libxml2/entities.c
  libxml2/error.c
  libxml2/globals.c
  libxml2/hash.c
  libxml2/HTMLparser.c
  libxml2/HTMLtree.c
  libxml2/legacy.c
  libxml2/list.c
  libxml2/nanoftp.c
  libxml2/nanohttp.c
  libxml2/parser.c
  libxml2/parserInternals.c
  libxml2/pattern.c
  libxml2/relaxng.c
  libxml2/SAX.c
  libxml2/SAX2.c
  libxml2/schematron.c
  libxml2/threads.c
  libxml2/tree.c
  libxml2/uri.c
  libxml2/valid.c
  libxml2/xinclude.c
  libxml2/xlink.c
  libxml2/xmlcatalog.c
  libxml2/xmlIO.c
  libxml2/xmlmemory.c
  libxml2/xmlmodule.c
  libxml2/xmlreader.c
  libxml2/xmlregexp.c
  libxml2/xmlsave.c
  libxml2/xmlschemas.c
  libxml2/xmlschemastypes.c
  libxml2/xmlstring.c
  libxml2/xmlunicode.c
  libxml2/xmlwriter.c
  libxml2/xpath.c
  libxml2/xpointer.c
)

add_library(xml2 ${xml2_srcs})

# Options
if(WITH_PIC)
  set_target_properties(xml2 PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

# Includes
if(WIN32)
  file(
    COPY
      "${CMAKE_SOURCE_DIR}/libxml2/include/win32config.h"
    DESTINATION
      "${CMAKE_BINARY_DIR}/config.h"
  )
endif()

target_include_directories(
  xml2
  PRIVATE
    "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}"
    "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libxml2>"
    "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libxml2/include>"
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/libxml2/include>"
    "$<INSTALL_INTERFACE:include>"
)

# Dependencies
if(WIN32)
  if(LIBXML_FTP_ENABLED OR LIBXML_HTTP_ENABLED)
    target_link_libraries(xml2 PRIVATE ws2_32)
  endif()
elseif(UNIX)
  if(LIBXML_THREAD_ENABLED)
    # FIX: disable TRY_RUN for pthreads when cross-compiling
    if(CMAKE_CROSSCOMPILING)
      set(THREADS_PTHREAD_ARG OFF)
    endif()
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    target_link_libraries(xml2 PRIVATE ${CMAKE_THREAD_LIBS_INIT})
  endif()
endif()

#-------------------------------------------------------------------------------
# Installation

install(
  TARGETS xml2
  LIBRARY DESTINATION "lib" COMPONENT libraries
  ARCHIVE DESTINATION "lib" COMPONENT archive
  RUNTIME DESTINATION "bin" COMPONENT binaries
  INCLUDES DESTINATION "include"
)

install(
  DIRECTORY
    "${CMAKE_SOURCE_DIR}/libxml2/include/libxml"
  DESTINATION
    "include"
  COMPONENT headers
)