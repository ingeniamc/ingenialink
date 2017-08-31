#-------------------------------------------------------------------------------
# Coding Style check with checkpatch.pl (Linux Kernel)
#
# Notes:
#   The following warnings/errors are ignored:
#     - NEW_TYPEDEFS: We are not in Kernel, so it's fine to have them.
#     - PREFER_KERNEL_TYPES: Again, we are not in Kernel.
#     - CONST_STRUCT: Not necessary.
#     - CAMELCASE: Some API names used do not follow this policy, and we can't
#       fix them anyway.
#     - MACRO_ARG_REUSE: Should be allowd in some constructs (e.g. stuff like
#       MIN(a,b))
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
  set(CHECKPATCH
    ${CMAKE_SOURCE_DIR}/scripts/checkpatch.pl
  )

  set(CHECKPATCH_ARGS
    --no-tree --terse --strict --subjective
    --ignore NEW_TYPEDEFS,PREFER_KERNEL_TYPES,CONST_STRUCT,CAMELCASE
    --ignore MACRO_ARG_REUSE --file
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
