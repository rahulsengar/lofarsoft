
## -----------------------------------------------------------------------------
## - Check for the AIPS++/CASA library
##
## Variables assigned:
##  HAVE_CASA
##  HAVE_AIPS_H       -- aips.h header file located
##  HAVE_GLISH_H      -- glish.h header file located
##  CASA_INCLUDE_DIR  -- Path to the CASA header files
##

## -----------------------------------------------------------------------------
## Check for the header files first, as from this we can derive a number of 
## fundamental CASA variables (such as e.g. the root directory of the
## installation). The main point to keep in mind here is, that parts of the
## final installation will reside in architecture-dependent directories - the
## setup of the code directory however is generic, so once we have located it
## we can go from there.

## Find "aips.h", which is included by all other files

find_path (CASA_INCLUDE_DIR aips.h
  PATHS
  /casa
  /opt/casa
  /sw/share/casa
  PATH_SUFFIXES
  code/include/casa
  current/code/include/casa
  weekly/code/include/casa
  stable/code/include/casa
  )

if (CASA_INCLUDE_DIR)
  set (HAVE_AIPS_H true)
  string (REGEX REPLACE include/casa include CASA_INCLUDE_DIR ${CASA_INCLUDE_DIR})
  string (REGEX REPLACE /code/include "" AIPSROOT ${CASA_INCLUDE_DIR})
endif (CASA_INCLUDE_DIR)

## Find "glish.h", which is pointing towards the location of the Glish source code

if (HAVE_AIPS_H)
  ## locate the Glish header file
  find_path (GLISH_INCLUDE_DIR glish.h
    ${CASA_INCLUDE_DIR}
    PATH_SUFFIXES
    Glish
    )
  ## adjust the include path for the Glish header files
  if (GLISH_INCLUDE_DIR)
    set (HAVE_GLISH_H true)
    STRING (REGEX REPLACE include/Glish include GLISH_INCLUDE_DIR ${GLISH_INCLUDE_DIR})
  endif (GLISH_INCLUDE_DIR)
endif (HAVE_AIPS_H)

## -----------------------------------------------------------------------------
## Preparation: get additional information about the system

if (UNIX)
  set (AIPS_ARCH "LINUX")
  set (AIPS_ENDIAN "LITTLE")
  IF (APPLE)
    ## definition passed to the compiler
    SET (AIPS_DARWIN 1)
    ## AIPS architecture -- as used for the compiler
    SET (AIPS_ARCH "DARWIN")
    ## AIPS architecture -- AIPSARCH system variable
    SET (AIPSARCH "darwin")
    ## check the processor type
    IF (${CMAKE_OSX_ARCHITECTURES} MATCHES "ppc")
      SET (AIPS_ENDIAN "BIG")
    ENDIF (${CMAKE_OSX_ARCHITECTURES} MATCHES "ppc")
  endif (APPLE)
endif (UNIX)

## -----------------------------------------------------------------------------
## Check for the library

set (CASA_MODULES
  aips
  alma
  atnf
  bima
  calibration
  casa
  components
  contrib
  coordinates
  demo
  dish
  display
  fits
  flagging
  graphics
  hia
  images
  ionosphere
  jive
  lattices
  )

FIND_PATH (AIPSLIBD version.o
  PATHS ${AIPSROOT}
  PATH_SUFFIXES darwin/lib linux/lib linux_gnu/lib
  )

STRING (REGEX REPLACE /lib "" AIPSARCH ${AIPSLIBD})

FIND_LIBRARY (CALIBRATION_LIBRARY calibration ${AIPSLIBD})
FIND_LIBRARY (CASA_LIBRARY casa ${AIPSLIBD})
FIND_LIBRARY (COMPONENTS_LIBRARY components ${AIPSLIBD})
FIND_LIBRARY (COORDINATES_LIBRARY coordinates ${AIPSLIBD})
FIND_LIBRARY (FITS_LIBRARY fits ${AIPSLIBD})
FIND_LIBRARY (GRAPHICS_LIBRARY graphics ${AIPSLIBD})
FIND_LIBRARY (IMAGES_LIBRARY images ${AIPSLIBD})
FIND_LIBRARY (LATTICES_LIBRARY lattices ${AIPSLIBD})
FIND_LIBRARY (MEASURES_LIBRARY measures ${AIPSLIBD})
FIND_LIBRARY (MS_LIBRARY ms ${AIPSLIBD})
FIND_LIBRARY (MSFITS_LIBRARY msfits ${AIPSLIBD})
FIND_LIBRARY (MSVIS_LIBRARY msvis ${AIPSLIBD})
FIND_LIBRARY (SCIMATH_LIBRARY scimath ${AIPSLIBD})
FIND_LIBRARY (SCIMATH_F_LIBRARY scimath_f ${AIPSLIBD})
FIND_LIBRARY (TABLES_LIBRARY tables ${AIPSLIBD})
FIND_LIBRARY (TASKING_LIBRARY tasking ${AIPSLIBD})

## -----------------------------------------------------------------------------
## Due to fact, that the individual modules of the CASA libraries are
## interdependent, we need to take this into account when setting up the linker
## instructions. The reference for the dependences listed here is the
## corresponding table listed in the 'makedefs'
##
##  LINKcasa        := casa
##  LINKscimath     := scimath casa
##  LINKtables      := tables casa
##  LINKmeasures    := measures tables scimath casa
##  LINKfits        := fits measures tables scimath casa
##  LINKlattices    := lattices tables scimath casa
##  LINKcoordinates := coordinates fits measures tables scimath casa
##  LINKcomponents  := components coordinates fits measures tables scimath casa
##  LINKimages      := images components coordinates lattices fits measures
##                      tables scimath casa
##  LINKms          := ms measures tables scimath casa
##  LINKmsfits      := msfits ms fits measures tables scimath casa
##  LINKmsvis       := msvis ms measures tables scimath casa
##  LINKcalibration := calibration msvis ms measures tables scimath casa
##  LINKionosphere  := ionosphere measures tables scimath casa
##  LINKflagging    := flagging msvis ms lattices measures tables scimath casa
##  LINKdish        := dish ms fits coordinates lattices measures tables scimath
##                      casa
##  LINKsimulators  := simulators ms measures tables scimath casa
##  LINKsynthesis   := synthesis calibration msvis ms images components
##                      coordinates lattices fits measures tables scimath casa
##  LINKgraphics    := graphics casa
##  LINKtasking     := tasking graphics msvis ms images components coordinates
##                      lattices fits measures tables scimath casa
##
##  Starting off with the original table by subsequent substitution we get
##
##   LINKcasa        := casa
##   LINKscimath     := scimath LINKcasa
##   LINKtables      := tables LINKcasa
##   LINKmeasures    := measures tables LINKscimath
##   LINKfits        := fits LINKmeasures
##   LINKlattices    := lattices tables LINKscimath
##   LINKcoordinates := coordinates LINKfits
##   LINKcomponents  := components LINKcoordinates
##   TMPcomponents   := components coordinates lattices LINKfits
##   LINKimages      := images TMPcomponents
##   LINKms          := ms LINKmeasures
##   LINKmsfits      := msfits ms LINKfits
##   LINKmsvis       := msvis LINKms
##   LINKcalibration := calibration LINKmsvis
##   LINKionosphere  := ionosphere LINKmeasures
##   LINKflagging    := flagging msvis ms lattices LINKmeasures
##   LINKdish        := dish ms fits coordinates lattices LINKmeasures
##   LINKsimulators  := simulators LINKms
##   LINKsynthesis   := synthesis calibration msvis ms images TMPcomponents
##   LINKgraphics    := graphics LINKcasa
##   LINKtasking     := tasking graphics msvis ms images TMPcomponents
##

set (LINKcasa "-lcasa" CACHE STRING "LINKcasa")
set (LINKscimath "-lscimath -lscimath_f ${LINKcasa}" CACHE STRING "LINKscimath")
set (LINKtables "-ltables ${LINKcasa}" CACHE STRING "LINKtables")
set (LINKmeasures "-lmeasures -ltables ${LINKscimath}" CACHE STRING "LINKmeasures")
set (LINKfits "-lfits ${LINKmeasures}" CACHE STRING "LINKfits")
set (LINKlattices "-llattices -ltables ${LINKscimath}" CACHE STRING "LINKlattices")
set (LINKcoordinates "-lcoordinates ${LINKfits}" CACHE STRING "LINKcoordinates")
set (LINKcomponents "-lcomponents ${LINKcoordinates}" CACHE STRING "LINKcomponents")
set (TMPcomponents "-lcomponents -lcoordinates -llattices ${LINKfits}" CACHE STRING "TMPcomponents")
set (LINKimages "-limages ${TMPcomponents}" CACHE STRING "LINKimages")
set (LINKms "-lms ${LINKmeasures}" CACHE STRING "LINKms")
set (LINKmsfits "-lmsfits -lms ${LINKfits}" CACHE STRING "LINKmsfits")
set (LINKmsvis "-lmsvis ${LINKms}" CACHE STRING "LINKmsvis")
set (LINKcalibration "-lcalibration ${LINKmsvis}" CACHE STRING "LINKcalibration")
set (LINKionosphere "-lionosphere ${LINKmeasures}" CACHE STRING "LINKionosphere")
set (LINKflagging "-lflagging -lmsvis -lms -llattices ${LINKmeasures}" CACHE STRING "LINKflagging")
set (LINKdish "-ldish -lms -lfits -lcoordinates -llattices ${LINKmeasures}" CACHE STRING "LINKdish")
set (LINKsimulators "-lsimulators ${LINKms}" CACHE STRING "LINKsimulators")
set (LINKsynthesis "-lsynthesis -lcalibration -lmsvis -lms -limages ${TMPcomponents}" CACHE STRING "LINKsynthesis")
set (LINKgraphics "-lgraphics ${LINKcasa}" CACHE STRING "LINKgraphics")
set (LINKtasking "-ltasking -lgraphics -lmsvis -lms -limages ${TMPcomponents}" CACHE STRING "LINKtasking")

IF (CASA_LIBRARY)
  SET (CASA_LIBRARIES ${LINKcasa})
  IF (SCIMATH_LIBRARY)
    SET (CASA_LIBRARIES ${LINKscimath})
  ENDIF (SCIMATH_LIBRARY)
  IF (TABLES_LIBRARY)
    SET (CASA_LIBRARIES ${LINKtables})
  ENDIF (TABLES_LIBRARY)
  IF (GRAPHICS_LIBRARY)
    SET (CASA_LIBRARIES ${LINKgraphics})
  ENDIF (GRAPHICS_LIBRARY)
  IF (SCIMATH_LIBRARY AND TABLES_LIBRARY)
    if (MEASURES_LIBRARY)
      set (CASA_LIBRARIES ${LINKmeasures})
      if (MS_LIBRARY)
	set (CASA_LIBRARIES ${LINKms})
      endif (MS_LIBRARY)
      if (FITS_LIBRARY)
	set (CASA_LIBRARIES ${LINKfits})
	if (COORDINATES_LIBRARY)
	  set (CASA_LIBRARIES ${LINKcoordinates})
	  if (COMPONENTS_LIBRARY)
	    set (CASA_LIBRARIES ${LINKcomponents})
	    if (IMAGES_LIBRARY)
	      set (CASA_LIBRARIES ${LINKimages})
	    endif (IMAGES_LIBRARY)
	  endif (COMPONENTS_LIBRARY)
	endif (COORDINATES_LIBRARY)
      endif (FITS_LIBRARY)
    endif (MEASURES_LIBRARY)
  endif (SCIMATH_LIBRARY AND TABLES_LIBRARY)
endif (CASA_LIBRARY)

## -----------------------------------------------------------------------------
## If detection successful, register package as found

IF (CASA_INCLUDE_DIR AND CASA_LIBRARIES)
  SET(HAVE_CASA TRUE)
  STRING (REGEX REPLACE lib/libcasa.a lib CASA_LIBRARIES_DIR ${CASA_LIBRARY})
ELSE (CASA_INCLUDE_DIR AND CASA_LIBRARIES)
  IF (NOT CASA_FIND_QUIETLY)
    IF (NOT CASA_INCLUDE_DIR)
      MESSAGE (STATUS "Unable to find CASA header files!")
    ENDIF (NOT CASA_INCLUDE_DIR)
    IF (NOT CASA_LIBRARIES)
      MESSAGE (STATUS "Unable to find CASA library files!")
    ENDIF (NOT CASA_LIBRARIES)
  ENDIF (NOT CASA_FIND_QUIETLY)
ENDIF (CASA_INCLUDE_DIR AND CASA_LIBRARIES)

## ------------------------------------------------------------------------------
## Final assembly of the provided variables and flags; once this is done, we
## provide some extended feedback.

IF (HAVE_CASA)
  add_definitions (
    -DAIPS_${AIPS_ARCH}
    -DAIPS_${AIPS_ENDIAN}_ENDIAN
    -DNATIVE_EXCP
    -DAIPS_STDLIB
    -DAIPS_AUTO_STL
    -DAIPS_NO_LEA_MALLOC
    -D_GLIBCPP_DEPRECATED
    -DSIGNBIT 
    -DAIPS_NO_TEMPLATE_SRC
    -DAIPS_NO_TEMPLATE_SRC
    -I${CASA_INCLUDE_DIR}
    -DAIPS_${AIPS_ARCH})
  set (CASA_CXX_FLAGS "-fPIC -pipe -Wall -Wno-non-template-friend -Woverloaded-virtual -Wno-comment -fexceptions -Wcast-align")
  SET (CASA_CXX_LFLAGS "-L${CASA_LIBRARIES_DIR} ${CASA_LIBRARIES_DIR}/version.o ${CASA_LIBRARIES}")
  IF (NOT CASA_FIND_QUIETLY)
    message (STATUS "Found components for CASA.")
    message (STATUS "CASA architecture .. : ${AIPS_ARCH}")
    message (STATUS "CASA root dir ...... : ${AIPSROOT}")
    message (STATUS "CASA library dir ... : ${CASA_LIBRARIES_DIR}")
    message (STATUS "CASA library files . : ${CASA_LIBRARIES}")
    message (STATUS "CASA header files .. : ${CASA_INCLUDE_DIR}")
    message (STATUS "CASA compile command : ${CASA_CXX_FLAGS}")
    message (STATUS "CASA linker command  : ${CASA_CXX_LFLAGS}")
#    message (STATUS "LINKcasa        = ${LINKcasa}")
#    message (STATUS "LINKscimath     = ${LINKscimath}")
#    message (STATUS "LINKtables      = ${LINKtables}")
#    message (STATUS "LINKmeasures    = ${LINKmeasures}")
#    message (STATUS "LINKfits        = ${LINKfits}")
#    message (STATUS "LINKlattices    = ${LINKlattices}")
#    message (STATUS "LINKcoordinates = ${LINKcoordinates}")
#    message (STATUS "LINKcomponents  = ${LINKcomponents}")
#    message (STATUS "LINKimages      = ${LINKimages}")
#    message (STATUS "LINKms          = ${LINKms}")
#    message (STATUS "LINKmsfits      = ${LINKmsfits}")
#    message (STATUS "LINKmsvis       = ${LINKmsvis}")
  ENDIF (NOT CASA_FIND_QUIETLY)
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CASA_CXX_FLAGS}")
ELSE (HAVE_CASA)
  IF (CASA_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find CASA")
  ENDIF (CASA_FIND_REQUIRED)
ENDIF (HAVE_CASA)

## ------------------------------------------------------------------------------
## Mark as advanced ...

MARK_AS_ADVANCED (
  HAVE_CASA
  CASA_INCLUDE_DIR
  )