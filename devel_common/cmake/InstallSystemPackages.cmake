
## ==============================================================================
##
##  Some generic system inspection
##
## ==============================================================================

## Mute the output of the find scripts
set (FFTW3_FIND_QUIETLY     YES )
set (GFORTRAN_FIND_QUIETLY  YES )
set (GSL_FIND_QUIETLY       YES )
set (HDF5_FIND_QUIETLY      YES )

## Load CMake modules
include (FindFFTW3)
include (FindGFortran)
include (FindGSL)
include (FindHDF5)

##____________________________________________________________________
## Executables

find_program (fink_executable   fink    PATHS ${bin_locations})
find_program (aptget_executable apt-get PATHS ${bin_locations})
find_program (yast_executable   yast    PATHS ${bin_locations})

##____________________________________________________________________
## Determine platform details



## ==============================================================================
##
##  Package lists
##
## ==============================================================================

if (UNIX)
  
  if (APPLE)
    
    set (fink_packages
      atlas
      bison
      flex
      fftw3
      fftw3-shlibs
      gfortran-shlibs
      numpy-py26
      qt4-mac
      sip-py26
      pyqt4-py26
      )
    
    if (NOT doxygen_executable)
      list (APPEND fink_packages doxygen)
    endif (NOT doxygen_executable)
    
    if (NOT HAVE_GSL)
      list (APPEND fink_packages gsl)
      list (APPEND fink_packages gsl-shlibs)
    endif (NOT HAVE_GSL)
    
    if (NOT SWIG_FOUND)
      list (APPEND fink_packages swig)
    endif (NOT SWIG_FOUND)
    
  else (APPLE)
    
  endif (APPLE)

endif (UNIX)
  
## ==============================================================================
##
##  Wrapper for system-dependent package managers
##
## ==============================================================================

if (UNIX)
  if (APPLE)
    if (fink_executable)
    else (fink_executable)
      message (STATUS "Unable to install Fink packages - missing executable!")
    endif (fink_executable)
  else (APPLE)
  endif (APPLE)
else (UNIX)
  if (WIN32)
    message (STATUS "Installation of system packages not supported on Windows!")
  else (WIN32)
endif (UNIX)
