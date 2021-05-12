# This file is part of HemeLB and is Copyright (C)
# the HemeLB team and/or their institutions, as detailed in the
# file AUTHORS. This software is provided under the terms of the
# license in the file LICENSE.

include_guard()

# - Find CTEMPLATE
# Find the native CTEMPLATE includes and library
#
#   CTEMPLATE_FOUND       - True if CTEMPLATE found.
#   CTEMPLATE_INCLUDE_DIR - where to find CTEMPLATE.h, etc.
#   CTEMPLATE_LIBRARIES   - List of libraries when using CTEMPLATE.
#

option(CTEMPLATE_USE_STATIC "Prefer Static CTemplate library" OFF)
IF( CTEMPLATE_INCLUDE_DIR )
  # Already in cache, be silent
  SET( CTEMPLATE_FIND_QUIETLY TRUE )
ENDIF( CTEMPLATE_INCLUDE_DIR )

FIND_PATH( CTEMPLATE_INCLUDE_DIR "ctemplate/template.h")
if(CTEMPLATE_USE_STATIC)
  set(__old_cmake_find_lib_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()

FIND_LIBRARY( CTEMPLATE_LIBRARIES
  NAMES "ctemplate"
  PATH_SUFFIXES "ctemplate" )

if(CTEMPLATE_USE_STATIC)
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${__old_cmake_find_lib_suffixes})
endif()

# handle the QUIETLY and REQUIRED arguments and set CTEMPLATE_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( "FindPackageHandleStandardArgs" )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( "CTemplate" DEFAULT_MSG CTEMPLATE_INCLUDE_DIR CTEMPLATE_LIBRARIES )

MARK_AS_ADVANCED( CTEMPLATE_INCLUDE_DIR CTEMPLATE_LIBRARIES )

if (CTEMPLATE_FOUND)
  add_library(CTemplate::CTemplate INTERFACE IMPORTED)
  set_target_properties(CTemplate::CTemplate PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CTEMPLATE_INCLUDE_DIR}"
    INTERFACE_LINK_LIBRARIES "${CTEMPLATE_LIBRARIES}"
  )
endif()
