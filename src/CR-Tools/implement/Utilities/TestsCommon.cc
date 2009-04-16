/*-------------------------------------------------------------------------*
 | $Id::                                                                 $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2009                                                    *
 *   Lars B"ahren (bahren@astron.nl)                                       *
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

#include <Utilities/TestsCommon.h>

namespace CR { // Namespace CR -- begin

  //_____________________________________________________________________________
  //                                                                      runtime

  double runtime (clock_t const &start,
		  clock_t const &end)
  {
    return ((double)( end - start ) / (double)CLOCKS_PER_SEC );
  }

  //_____________________________________________________________________________
  //                                                              number_sequence

  std::vector<int> number_sequence () {
    // vector returning the result
    std::vector<int> nelem;
    // generate sequence
    nelem.push_back(1);
    nelem.push_back(2);
    nelem.push_back(5);
    nelem.push_back(10);
    nelem.push_back(20);
    nelem.push_back(50);
    nelem.push_back(100);
    nelem.push_back(200);
    nelem.push_back(500);
    nelem.push_back(1000);
    nelem.push_back(2000);
    nelem.push_back(5000);
    nelem.push_back(10000);
    // return sequence of numbers
    return nelem;
  }
  
  // ============================================================================
  //
  //  Module Coordinates
  //
  // ============================================================================
  
  CR::TimeFreqCoordinate getTimeFreq (uint const &blocksize,
				      casa::Quantity const &sampleFreq,
				      uint const &nyquistZone,
				      uint const &blocksPerFrame,
				      uint const &nofFrames)
  {
    CR::TimeFreqCoordinate timeFreq (blocksize,
				     sampleFreq,
				     nyquistZone,
				     blocksPerFrame,
				     nofFrames);
    return timeFreq;
  }

  // ============================================================================
  //
  //  Module Imaging
  //
  // ============================================================================
  
  //_____________________________________________________________________________
  //                                                             export_GeomDelay
  
  void export_GeomDelay (CR::GeomDelay &delay,
			 std::string const &prefix)
  {
    int ant;
    int sky;
    std::ofstream outfile;

    std::cout << "-- Exporting the values of the antenna positions ..." << std::endl;
    {
      std::string filename        = prefix+"-antPositions.dat";
      Matrix<double> antPositions = delay.antPositions();
      IPosition shape             = antPositions.shape();
      int nCoord;
      
      outfile.open(filename.c_str(),std::ios::out);
      
      for (ant=0; ant<shape(0); ant++) {
	outfile << ant;
	for (nCoord=0; nCoord<shape(1); nCoord++) {
	  outfile << "\t" << antPositions(ant,nCoord);
	}
	outfile << std::endl;
      }
      
      outfile.close();
    }
  }

  //_____________________________________________________________________________
  //                                                            export_Beamformer
  
  void export_Beamformer (CR::Beamformer &bf,
			  std::string const &prefix)
  {
    std::cout << "[TestsCommon::export_Beamformer]" << std::endl;

    int ant;
    int sky;
    std::ofstream outfile;
    
    std::cout << "-- Exporting the values of the antenna positions ..." << std::endl;
    {
      std::string filename        = prefix+"-antPositions.dat";
      Matrix<double> antPositions = bf.antPositions();
      IPosition shape             = antPositions.shape();
      int nCoord;
      
      outfile.open(filename.c_str(),std::ios::out);
      
      for (ant=0; ant<shape(0); ant++) {
	outfile << ant;
	for (nCoord=0; nCoord<shape(1); nCoord++) {
	  outfile << "\t" << antPositions(ant,nCoord);
	}
	outfile << std::endl;
      }
      
      outfile.close();
    }
    
    std::cout << "-- Exporting the values of the sky positions ..." << std::endl;
    {
      std::string filename        = prefix+"-skyPositions.dat";
      Matrix<double> skyPositions = bf.skyPositions();
      IPosition shape             = skyPositions.shape();
      int nSky;
      int nCoord;
      
      outfile.open(filename.c_str(),std::ios::out);
      
      for (nSky=0; nSky<shape(0); nSky++) {
	outfile << nSky;
	for (nCoord=0; nCoord<shape(1); nCoord++) {
	  outfile << "\t" << skyPositions(nSky,nCoord);
	}
	outfile << std::endl;
      }
      
      outfile.close();
    }
    
    std::cout << "-- Exporting the values of the geometrical delays ..." << std::endl;
    {
      std::string filename  = prefix+"-delays.dat";
      Matrix<double> delays = bf.delays();
      IPosition shape       = delays.shape(); /* [ant,sky] */
      
      outfile.open(filename.c_str(),std::ios::out);
      
      for (ant=0; ant<shape(0); ant++) {
	for (sky=0; sky<shape(1); sky++) {
	  outfile << ant << "\t" << sky << "\t" << delays(ant,sky) << std::endl;
	}
	outfile << std::endl;
      }
      
      outfile.close();
    }
    
    /* Export the values of the geometrical phases */
  }  
  
} // Namespace CR -- end
