#-------------------------------------------------------------------------------
# Coding Style check with checkpatch.pl (Linux Kernel)
#
# Adding style check (only once):
#   include(StyleCheck)
#   if(STYLE_CHECK_AVAILABLE)
#     add_style_check("src/*.c" "src/*.h")
#   endif()
#
# Author/s:
#   Gerard Marull-Paretas <gmarull@ingeniamc.com>


if(NOT DEFINED STYLE_CHECK_AVAILABLE)
  find_package(Perl)
  
  if(PERL_FOUND)
    set(STYLE_CHECK_AVAILABLE ON CACHE BOOL "Style check available")
    if(NOT CMAKE_REQUIRED_QUIET)
      message(STATUS "Style check available (style_check)")
    endif()
  else()
    set(STYLE_CHECK_AVAILABLE OFF CACHE BOOL "Style check not available")
    if(NOT CMAKE_REQUIRED_QUIET)
      message(WARNING "Style check available not available. Install Perl.")
    endif()
  endif()
endif()

function(add_style_check)
  if(DEFINED HAS_STYLE_CHECK)
    return()
  else()
    set(
      HAS_STYLE_CHECK "" CACHE INTERNAL "Has style check"
    )
  endif()

  set(CHECKPATCH
    ${CMAKE_SOURCE_DIR}/scripts/checkpatch.pl
  )

  set(CHECKPATCH_ARGS
    --no-tree --terse --strict --subjective --ignore NEW_TYPEDEFS
    --ignore PREFER_KERNEL_TYPES --ignore CONST_STRUCT --file
  )

  file(
    GLOB_RECURSE
    CHECKPATCH_FILES
    ${ARGN}
  )

  add_custom_target(
    style_check
    ${PERL_EXECUTABLE} ${CHECKPATCH} ${CHECKPATCH_ARGS} ${CHECKPATCH_FILES}
    COMMENT "Checking coding style..." VERBATIM
  )
endfunction()
