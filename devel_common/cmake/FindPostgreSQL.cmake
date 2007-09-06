##------------------------------------------------------------------------
## $Id:: template_FindXX.cmake 654 2007-08-29 15:57:17Z baehren          $
##------------------------------------------------------------------------

# - Check for the presence of PostgreSQL
#
# The following variables are set when PostgreSQL is found:
#  HAVE_PostgreSQL       = Set to true, if all components of PostgreSQL
#                          have been found.
#  PostgreSQL_INCLUDES   = Include path for the header files of PostgreSQL
#  PostgreSQL_LIBRARIES  = Link these to use PostgreSQL
#  PostgreSQL_LFGLAS     = Linker flags (optional)

## -----------------------------------------------------------------------------
## Standard locations where to look for required components

set (lib_locations
  /usr/local/lib
  /usr/lib
  /usr/X11R6/lib
  /sw/lib
  )

set (include_locations
  /usr/include
  /usr/local/include
  /usr/X11R6/include
  /sw/include
  )

## -----------------------------------------------------------------------------
## Check for the header files

find_path (PostgreSQL_INCLUDES postgres_ext.h pgtypes_numeric.h libpq-fe.h
  PATHS ${include_locations}
  PATH_SUFFIXES postgresql
  NO_DEFAULT_PATH
  )

## -----------------------------------------------------------------------------
## Check for the library components

set (PostgreSQL_LIBRARIES "")

## [1] libpq

find_library (libpq pq
  PATHS ${lib_locations}
  PATH_SUFFIXES postgresql
  NO_DEFAULT_PATH
  )

if (libpq)
  list (APPEND PostgreSQL_LIBRARIES ${libpq})
endif (libpq)

## [2] libpgtypes

find_library (libpgtypes pgtypes 
  PATHS ${lib_locations}
  PATH_SUFFIXES postgresql
  NO_DEFAULT_PATH
  )

if (libpgtypes)
  list (APPEND PostgreSQL_LIBRARIES ${libpgtypes})
endif (libpgtypes)

## [3] libecpg

find_library (libecpg ecpg
  PATHS ${lib_locations}
  PATH_SUFFIXES postgresql
  NO_DEFAULT_PATH
  )

if (libecpg)
  list (APPEND PostgreSQL_LIBRARIES ${libecpg})
endif (libecpg)

## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (PostgreSQL_INCLUDES AND PostgreSQL_LIBRARIES)
  set (HAVE_PostgreSQL TRUE)
else (PostgreSQL_INCLUDES AND PostgreSQL_LIBRARIES)
  set (HAVE_PostgreSQL FALSE)
  if (NOT PostgreSQL_FIND_QUIETLY)
    if (NOT PostgreSQL_INCLUDES)
      message (STATUS "Unable to find PostgreSQL header files!")
    endif (NOT PostgreSQL_INCLUDES)
    if (NOT PostgreSQL_LIBRARIES)
      message (STATUS "Unable to find PostgreSQL library files!")
    endif (NOT PostgreSQL_LIBRARIES)
  endif (NOT PostgreSQL_FIND_QUIETLY)
endif (PostgreSQL_INCLUDES AND PostgreSQL_LIBRARIES)

if (HAVE_PostgreSQL)
  if (NOT PostgreSQL_FIND_QUIETLY)
    message (STATUS "Found components for PostgreSQL")
    message (STATUS "PostgreSQL_INCLUDES  = ${PostgreSQL_INCLUDES}")
    message (STATUS "PostgreSQL_LIBRARIES = ${PostgreSQL_LIBRARIES}")
  endif (NOT PostgreSQL_FIND_QUIETLY)
else (HAVE_PostgreSQL)
  if (PostgreSQL_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find PostgreSQL!")
  endif (PostgreSQL_FIND_REQUIRED)
endif (HAVE_PostgreSQL)

## -----------------------------------------------------------------------------
## Mark advanced variables

mark_as_advanced (
  PostgreSQL_INCLUDES
  PostgreSQL_LIBRARIES
  )
