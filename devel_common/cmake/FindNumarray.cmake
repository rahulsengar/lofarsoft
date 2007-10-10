##------------------------------------------------------------------------
## $Id:: template_FindXX.cmake 815 2007-09-21 09:18:08Z baehren          $
##------------------------------------------------------------------------

# - Check for the presence of Numarray
#
# The following variables are set when Numarray is found:
#  HAVE_NUMARRAY       = Set to true, if all components of Numarray
#                          have been found.
#  NUMARRAY_INCLUDES   = Include path for the header files of Numarray
#  NUMARRAY_LIBRARIES  = Link these to use Numarray
#  NUMARRAY_LFGLAS     = Linker flags (optional)

## -----------------------------------------------------------------------------
## Standard locations where to look for required components

set (bin_locations
  ./../release/bin
  ./../../release/bin
  /usr/local/bin
  /usr/bin
  /usr/X11R6/bin
  /opt/bin
  /opt/local/bin
  /sw/bin
  )

set (lib_locations
  ./../release/lib
  ./../../release/lib
  /usr/local/lib
  /usr/lib
  /usr/X11R6/lib
  /opt/lib
  /opt/local/lib
  /sw/lib
  )

set (include_locations
  ./../release/include
  ./../../release/include
  /usr/include
  /usr/local/include
  /usr/X11R6/include
  /opt/include
  /opt/local/include
  /sw/include
  )

## -----------------------------------------------------------------------------
## Check for the header files

find_path (NUMARRAY_INCLUDES <header file(s)>
  PATHS ${include_locations}
  PATH_SUFFIXES <optional path extension>
  NO_DEFAULT_PATH
  )

## -----------------------------------------------------------------------------
## Check for the library

find_library (NUMARRAY_LIBRARIES <package name>
  PATHS ${lib_locations}
  NO_DEFAULT_PATH
  )

## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (NUMARRAY_INCLUDES AND NUMARRAY_LIBRARIES)
  set (HAVE_NUMARRAY TRUE)
else (NUMARRAY_INCLUDES AND NUMARRAY_LIBRARIES)
  set (HAVE_NUMARRAY FALSE)
  if (NOT NUMARRAY_FIND_QUIETLY)
    if (NOT NUMARRAY_INCLUDES)
      message (STATUS "Unable to find NUMARRAY header files!")
    endif (NOT NUMARRAY_INCLUDES)
    if (NOT NUMARRAY_LIBRARIES)
      message (STATUS "Unable to find NUMARRAY library files!")
    endif (NOT NUMARRAY_LIBRARIES)
  endif (NOT NUMARRAY_FIND_QUIETLY)
endif (NUMARRAY_INCLUDES AND NUMARRAY_LIBRARIES)

if (HAVE_NUMARRAY)
  if (NOT NUMARRAY_FIND_QUIETLY)
    message (STATUS "Found components for NUMARRAY")
    message (STATUS "NUMARRAY_INCLUDES  = ${NUMARRAY_INCLUDES}")
    message (STATUS "NUMARRAY_LIBRARIES = ${NUMARRAY_LIBRARIES}")
  endif (NOT NUMARRAY_FIND_QUIETLY)
else (HAVE_NUMARRAY)
  if (NUMARRAY_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find NUMARRAY!")
  endif (NUMARRAY_FIND_REQUIRED)
endif (HAVE_NUMARRAY)

## -----------------------------------------------------------------------------
## Mark advanced variables

mark_as_advanced (
  NUMARRAY_INCLUDES
  NUMARRAY_LIBRARIES
  )
