## ============================================================================== 
##
##  Register project in CMake
##
## ============================================================================== 

## Name of the project
project (StarTools)

## Minimum required CMake version
cmake_minimum_required (VERSION 2.6)

## User defined build/install options

option (StarTools_VERBOSE_CONFIGURE  "Verbose output during configuration?"       0)
option (StarTools_FORCE_BUILD        "Force rebuild and local installation"       0)
option (StarTools_BUILD_APPLICATIONS "Build all the applications in the package?" 1)

## System inspection

find_package (StarTools)

## ============================================================================== 
##
##  Organization of the project
##
## ============================================================================== 

#  .
#  |-- calib
#  |   |-- Amp_meas
#  |   |-- AntPosition_meas
#  |   |-- Lab_meas
#  |   |   |-- FreqResp
#  |   |   |-- HF_Bandpass_rev02
#  |   |   |-- HF_Board_17
#  |   |   |-- LNA
#  |   |   `-- RefCoef
#  |   |-- Rectifier_meas
#  |   `-- TimeDelay_meas
#  |-- doc
#  |-- include
#  |-- lib
#  |-- obj
#  |-- scripts
#  `-- src

## ==============================================================================
## 
##  Search for external packages 
##
## ==============================================================================

if (NOT StarTools_VERBOSE_CONFIGURE)
  set (FFTW3_FIND_QUIETLY      YES )
  set (POSTGRESQL_FIND_QUIETLY YES )
  set (ROOT_FIND_QUIETLY       YES )
  set (STARTOOLS_FIND_QUIETLY  YES )
endif (NOT StarTools_VERBOSE_CONFIGURE)

##__________________________________________________________
##  System header files

if (HAVE_LIBGEN_H)
  add_definitions (-DHAVE_LIBGEN_H)
endif (HAVE_LIBGEN_H)

if (HAVE_TIME_H)
  add_definitions (-DHAVE_TIME_H)
endif (HAVE_TIME_H)

if (HAVE_MATH_H)
  add_definitions (-DHAVE_MATH_H)
endif (HAVE_MATH_H)

##__________________________________________________________
##                                                     FFTW3

if (NOT HAVE_FFTW3)
  include (FindFFTW3)
endif (HAVE_FFTW3)

if (HAVE_FFTW3)
  include_directories (${FFTW3_INCLUDES})
  add_definitions (-DHAVE_FFTW3)
else (HAVE_FFTW3)
  message (SEND_ERROR "[Star-Tools] Missing or incomplete FFTW3 installation!")
  message (STATUS "FFTW3_INCLUDES  = ${FFTW3_INCLUDES}")
  message (STATUS "FFTW3_LIBRARIES = ${FFTW3_LIBRARIES}")
endif (HAVE_FFTW3)

##__________________________________________________________
##                                                      ROOT

if (NOT ROOT_FOUND)
  find_package (ROOT)
endif (NOT ROOT_FOUND)

if (ROOT_FOUND)
  include_directories (${ROOT_INCLUDES})
  add_definitions (-DHAVE_ROOT)
else (ROOT_FOUND)
  message (SEND_ERROR "[Star-Tools] Missing or incomplete ROOT installation!")
  message (STATUS "ROOT_INCLUDES  = ${ROOT_INCLUDES}")
endif (ROOT_FOUND)

##__________________________________________________________
##                                                PostgreSQL

if (NOT PostgreSQL_FOUND)
  include (FindPostgreSQL)
endif (NOT PostgreSQL_FOUND)

if (POSTGRESQL_INCLUDES AND POSTGRESQL_LIBRARIES)
  set (HAVE_PostgreSQL TRUE)
  include_directories (${POSTGRESQL_INCLUDES})
  add_definitions (-DHAVE_POSTGRESQL)
endif (POSTGRESQL_INCLUDES AND POSTGRESQL_LIBRARIES)

## ==============================================================================
##
##  Installation settings
##
## ==============================================================================

## No relinking and full RPATH for the install tree

set (CMAKE_SKIP_BUILD_RPATH TRUE)
set (CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


## ==============================================================================
##
##  Subdirectories of the project  
##
## ==============================================================================

include_directories (
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/include
  )

add_subdirectory (src)

## ==============================================================================
##
##  Summary of configuration
##
## ==============================================================================

if (StarTools_VERBOSE_CONFIGURE)
  message (STATUS " ")
  message (STATUS "============================================================")
  message (STATUS " ")
  message (STATUS "[Star-Tools] Summary of configuration settings")
  message (STATUS " ")
  message (STATUS "FFTW3 library       = ${FFTW3_LIBRARIES}")
  message (STATUS "FFTW3 includes      = ${FFTW3_INCLUDES}")
  message (STATUS "ROOT library        = ${ROOT_LIBRARIES}")
  message (STATUS "ROOT includes       = ${ROOT_INCLUDES}")
  message (STATUS "PostgreSQL library  = ${POSTGRESQL_LIBRARIES}")
  message (STATUS "PostgreSQL includes = ${POSTGRESQL_INCLUDES}")
  message (STATUS " ")
  message (STATUS "Installation prefix = ${CMAKE_INSTALL_PREFIX}")
  message (STATUS " ")
  message (STATUS "============================================================")
  message (STATUS " ")
endif (StarTools_VERBOSE_CONFIGURE)

