#!/bin/sh

##-------------------------------------------------------------------------------
## $Id:: CMakeLists.txt 724 2007-09-10 10:17:21Z baehren                        $
##-------------------------------------------------------------------------------
##
## A simple shell script to configure and install as many as possible of
## the packages in the LOFAR USG distribution. Simple run
## 
##   ./build.sh
##
## from within the build directory - and the script should take care of
## the rest.
##

basedir=`pwd`

## -----------------------------------------------------------------------------
## Helper functions

# \param buidrDir  -- directory in which to perform the build; this is a 
#                     sub-directory created below "build"
# \param sourceDir -- directory in which the source code of the package can be
#                     found; the path is given relativ to $LOFARSOFT
build_package ()
{
    buildDir=$1
    sourceDir=$2
    configureOption=$3
    # check if the build directory exists
    cd $basedir
    if test -d $buildDir ; then
	{
	# change into the build directory
	cd $buildDir
	# clean up the directory before we do anything else
	rm -rf *
	# run cmake on the source directory
	if test -z $configureOption ; then
	    cmake $basedir/../$sourceDir
	else 
	    cmake $basedir/../$sourceDir $$configureOption
	fi
	# build the package
	make install;
	}
    else 
	{
	echo "No build directory $buildDir - creating it now."
	mkdir $buildDir;
	# recursive call
	build_package $buildDir $sourceDir
	}
    fi
}

## -----------------------------------------------------------------------------
## Build individual/multiple packages

case $1 in 
    bison)
	echo "[build] Selected package Bison"
	build_package bison external/bison
    ;;
    casacore)
	echo "[build] Selected package CASACORE"
	build_package casacore external/casacore
    ;;
    cfitsio)
	echo "[build] Selected package CFITSIO"
	build_package cfitsio external/cfitsio
    ;;
    flex)
	echo "[build] Selected package Flex"
	build_package flex external/flex
    ;;
    hdf5)
	echo "[build] Selected package Hdf5"
	build_package hdf5 external/hdf5
    ;;
    wcslib)
	echo "[build] Selected package WCSLIB"
	build_package wcslib external/wcslib
    ;;
    dal)
	echo "[build] Selected package DAL"
	## external packages
	./build.sh wcslib
	./build.sh cfitsio
	./build.sh casacore
	build_package hdf5 external/hdf5 -DHDF5_FORCE_BUILD:BOOL=1
	build_package boost external/boost -DBOOST_FORCE_BUILD:BOOL=1
	## USG packages
	build_package dal src/DAL
    ;;
    cr)
	echo "[build] Selected package CR-Tools"
	./build.sh dal
	build_package cr src/CR-Tools
    ;;
    *)
	## external packages
	build_package bison external/bison
	build_package flex external/flex
	build_package pgplot external/pgplot
	build_package wcslib external/wcslib
	build_package cfitsio external/cfitsio
	build_package casacore external/casacore
	build_package hdf5 external/hdf5
	build_package boost external/boost
    ;;
esac
