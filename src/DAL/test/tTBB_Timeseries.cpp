/*-------------------------------------------------------------------------*
 | $Id::                                                                 $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2008                                                    *
 *   Lars B"ahren (lbaehren@gmail.com)                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <casa/Arrays/ArrayIO.h>
#include <casa/HDF5/HDF5File.h>
#include <casa/HDF5/HDF5Record.h>

#include <TBB_Timeseries.h>

using std::cerr;
using std::cout;
using std::endl;
using DAL::TBB_Timeseries;  // Namespace usage

/*!
  \file tTBB_Timeseries.cpp

  \ingroup DAL

  \brief A collection of test routines for the TBB_Timeseries class
 
  \author Lars B&auml;hren
 
  \date 2008/02/06
*/

// -----------------------------------------------------------------------------

/*!
  \brief Test constructors for a new TBB_Timeseries object

  \param filename -- Name of the HDF5 file used for testing

  \return nofFailedTests -- The number of failed tests.
*/
int test_construction (std::string const &filename)
{
  cout << "\n[test_construction]\n" << endl;

  int nofFailedTests (0);

  //__________________________________________________________________
  // Test the default constructor
  
  cout << "[1] Testing default constructor ..." << endl;
  try {
    TBB_Timeseries newTBB_Timeseries;
    //
    newTBB_Timeseries.summary(); 
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  //__________________________________________________________________
  // Test the argumented constructor taking filename as input
  
  cout << "[2] Testing argumented constructor ..." << endl;
  try {
    TBB_Timeseries newTBB_Timeseries (filename);
    //
    newTBB_Timeseries.summary(); 
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }

  //__________________________________________________________________
  // Test the copy constructor
  
  cout << "[3] Testing copy constructor ..." << endl;
  try {
    cout << "--> creating original object ..." << endl;
    TBB_Timeseries timeseries (filename);
    timeseries.summary(); 
    //
    cout << "--> creating new object by copy ..." << endl;
    TBB_Timeseries timeseriesCopy (timeseries);
    timeseriesCopy.summary(); 
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }
  
  return nofFailedTests;
}

// -----------------------------------------------------------------------------

/*!
  \brief Test the various methods of the class

  \param filename -- Name of the HDF5 file used for testing

  \return nofFailedTests -- The number of failed tests.
*/
int test_methods (std::string const &filename)
{
  cout << "\n[test_methods]\n" << endl;

  int nofFailedTests = 0;
  TBB_Timeseries ts (filename);

  cout << "[1] Retrieve attributes attached to the root group ..." << endl;
  try {
    std::string telescope;
    std::string observer;
    std::string project;
    std::string observation_id;
    std::string observation_mode;
    //
    telescope        = ts.telescope();
    observer         = ts.observer();
    project          = ts.project();
    observation_id   = ts.observation_id();
    observation_mode = ts.observation_mode();
    //
    cout << "-- TELESCOPE ...... = " << telescope        << endl;
    cout << "-- OBSERVER ....... = " << observer         << endl;
    cout << "-- PROJECT ........ = " << project          << endl;
    cout << "-- OBSERVATION_ID   = " << observation_id   << endl;
    cout << "-- OBSERVATION_MODE = " << observation_mode << endl;
  } catch (std::string message) {
    cerr << message << endl;
    nofFailedTests++;
  }
  
  cout << "[2] Retrieve attributes attached to the station groups ..." << endl;
  try {
    std::string group_name;
#ifdef HAVE_CASA
    casa::Vector<casa::String> trigger_type;
    casa::Vector <double> trigger_offset;
    casa::Vector<double> station_position_value;
    casa::Vector<casa::String> station_position_unit;
    std::string station_position_frame;
    casa::Vector<double> beam_direction_value;
    casa::Vector<casa::String> beam_direction_unit;
    std::string beam_direction_frame;
#else
    std::vector<std::string> trigger_type;
    std::vector<double> trigger_offset;
    std::vector<double> station_position_value;
    std::vector<std::string> station_position_unit;
    std::string station_position_frame;
    std::vector<double> beam_direction_value;
    std::vector<std::string> beam_direction_unit;
    std::string beam_direction_frame;
#endif
    //
    trigger_type   = ts.trigger_type();
    trigger_offset = ts.trigger_offset();
    //
    cout << "-- TRIGGER_TYPE ......... = " << trigger_type           << endl;
    cout << "-- TRIGGER_OFFSET ....... = " << trigger_offset         << endl;
    cout << "-- STATION_POSITION_VALUE = " << station_position_value << endl;
    cout << "-- STATION_POSITION_UNIT  = " << station_position_unit  << endl;
    cout << "-- STATION_POSITION_FRAME = " << station_position_frame << endl;
    cout << "-- BEAM_DIRECTION_VALUE   = " << beam_direction_value   << endl;
    cout << "-- BEAM_DIRECTION_UNIT    = " << beam_direction_unit    << endl;
    cout << "-- BEAM_DIRECTION_FRAME   = " << beam_direction_frame   << endl;
  } catch (std::string message) {
    cerr << message << endl;
    nofFailedTests++;
  }
  
  return nofFailedTests;
}

// -----------------------------------------------------------------------------

/*!
  \brief Test export of the attribute values to a casa::Record object
  
  \param filename -- Name of the HDF5 file, within which the dataset is located

  \return nofFailedTests -- The number of failed tests.
*/
int test_attributes2record (std::string const &filename)
{
  cout << "\n[test_attributes2record]\n" << endl;

  int nofFailedTests (0);
  TBB_Timeseries ts (filename);

  cout << "[1] Retreiving attributes of group into record ..." << endl;
  try {
    // retrieve attributes into record
    casa::Record rec = ts.attributes2record ();
    // display the contents of the record
    std::cout << rec << endl;
  } catch (std::string message) {
    cerr << message << endl;
    nofFailedTests++;
  }

  cout << "[2] Retreiving attributes of group into record (recursive) ..."
	    << endl;
  try {
    // retrieve attributes into record
    casa::Record rec = ts.attributes2record (true);
    // display the contents of the record
    std::cout << rec << endl;
  } catch (std::string message) {
    cerr << message << endl;
    nofFailedTests++;
  }

  return nofFailedTests;

  cout << "[3] Retrieve attributes into a header record for the DR ..." << endl;
  try {
    casa::Record rec;
    bool recursive (false);
    bool headerRecord (true);
    // retrieve the attrbutes into the record
    ts.attributes2record (rec,recursive,headerRecord);
    // display the contents of the record
    std::cout << rec << endl;
  } catch (std::string message) {
    cerr << message << endl;
    nofFailedTests++;
  }

  cout << "[4] Test operations on parameters " << endl;
  try {
    uint min_times      = min(ts.time());
    uint min_dataLength = min(ts.data_length());
    //
    cout << "-- min(times)       = " << min_times      << endl;
    cout << "-- min(data_length) = " << min_dataLength << endl;
  } catch (std::string message) {
    cerr << message << endl;
    nofFailedTests++;
  }

  return nofFailedTests;
}

// -----------------------------------------------------------------------------

/*!
  \brief Test retrieval of TBB data

  \param filename -- Name of the HDF5 file used for testing

  \return nofFailedTests -- The number of failed tests.
*/
int test_data (std::string const &filename)
{
  cout << "\n[test_data]\n" << endl;

  int nofFailedTests = 0;
  int start          = 0;
  int nofSamples     = 1024;
  TBB_Timeseries timeseries (filename);

  cout << "[1] Retrieve time-series data without channel selection"
	    << endl;
  try {
    casa::Matrix<double> data = timeseries.fx (start,
					       nofSamples);
    // feedback 
    cout << "-- Data start     = " << start        << endl;
    cout << "-- Data blocksize = " << nofSamples   << endl;
    cout << "-- Data array     = " << data.shape() << endl;
    cout << "-- Data [0,]      = " << data.row(0)  << endl;
    cout << "-- Data [1,]      = " << data.row(1)  << endl;
  } catch (std::string message) {
    std::cerr << message << endl;
    nofFailedTests++;
  }
  
  return nofFailedTests;
}

// -----------------------------------------------------------------------------

int main (int argc,
	  char *argv[])
{
  int nofFailedTests (0);

  //__________________________________________________________________
  // Check the input parameters provided form the command line
  
  if (argc < 2) {
    std::cerr << "[tTBB_StationGroup] Too few parameters!" << endl;
    std::cerr << "" << endl;
    std::cerr << "  tTBB_StationGroup <filename>" << endl;
    std::cerr << "" << endl;
    return -1;
  }

  std::string filename = argv[1];
  std::string name_station = "Station001";
  std::string name_dataset = "001000001";
  
  if (argc == 3) {
    name_station = argv[2];
  }
  
  if (argc == 4) {
    name_dataset = argv[3];
  }
  
  //__________________________________________________________________
  // Run the tests
  
  // Test for the constructor(s)
  nofFailedTests += test_construction (filename);

  if (nofFailedTests == 0) {
    nofFailedTests += test_methods (filename);
    nofFailedTests += test_attributes2record (filename);
    nofFailedTests += test_data (filename);
  } else {
    std::cerr << "[tTBB_StationGroup]"
	      << " Skipping tests after testing constructors returned errors!"
	      << endl;
  }
  
  return nofFailedTests;
}
