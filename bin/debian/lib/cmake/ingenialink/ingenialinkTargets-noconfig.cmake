#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ingenialink" for configuration ""
set_property(TARGET ingenialink APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(ingenialink PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libingenialink.so"
  IMPORTED_SONAME_NOCONFIG "libingenialink.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS ingenialink )
list(APPEND _IMPORT_CHECK_FILES_FOR_ingenialink "${_IMPORT_PREFIX}/lib/libingenialink.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
