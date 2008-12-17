/*-------------------------------------------------------------------------*
 | $Id:: NewClass.cc 1964 2008-09-06 17:52:38Z baehren                   $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2008                                                    *
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

#include <Imaging/GeomDelay.h>

namespace CR { // Namespace CR -- begin
  
  // ============================================================================
  //
  //  Construction / Destruction
  //
  // ============================================================================

  //_____________________________________________________________________________
  //                                                                    GeomDelay
  
  GeomDelay::GeomDelay ()
  {
    init ();
  }
  
  //_____________________________________________________________________________
  //                                                                    GeomDelay
  
  GeomDelay::GeomDelay (Matrix<double> const &antPositions,
			Matrix<double> const &skyPositions,
			bool const &farField,
			bool const &bufferDelays)
  {
    init (antPositions,
	  skyPositions,
	  farField,
	  bufferDelays);
  }
  
  //_____________________________________________________________________________
  //                                                                    GeomDelay
  
  GeomDelay::GeomDelay (GeomDelay const &other)
  {
    copy (other);
  }
  
  //_____________________________________________________________________________
  //                                                                   ~GeomDelay
  
  GeomDelay::~GeomDelay ()
  {
    destroy();
  }
  
  //_____________________________________________________________________________
  //                                                                      destroy
  
  void GeomDelay::destroy ()
  {;}
  
  // ============================================================================
  //
  //  Operators
  //
  // ============================================================================
  
  //_____________________________________________________________________________
  //                                                                    operator=

  GeomDelay& GeomDelay::operator= (GeomDelay const &other)
  {
    if (this != &other) {
      destroy ();
      copy (other);
    }
    return *this;
  }
  
  //_____________________________________________________________________________
  //                                                                         copy

  void GeomDelay::copy (GeomDelay const &other)
  {
    farField_p     = other.farField_p;
    bufferDelays_p = other.bufferDelays_p;

    antPositions_p.resize(other.antPositions_p.shape());
    antPositions_p = other.antPositions_p;

    skyPositions_p.resize(other.skyPositions_p.shape());
    skyPositions_p = other.skyPositions_p;
    
    if (bufferDelays_p) {
      delays_p.resize(other.delays_p.shape());
      delays_p = other.delays_p;
    }
  }
  
  // ============================================================================
  //
  //  Parameters
  //
  // ============================================================================

  //_____________________________________________________________________________
  //                                                                         init

  void GeomDelay::init ()
  {
    Matrix<double> antPos (1,3,0.0);
    Matrix<double> skyPos (1,3,0.0);

    /* Default sky position: [0,0,1] */
    skyPos(0,2) = 1.0;
    
    init (antPos,
	  skyPos,
	  false,
	  false);
  }
  
  //_____________________________________________________________________________
  //                                                                         init
  
  void GeomDelay::init (Matrix<double> const &antPositions,
			Matrix<double> const &skyPositions,
			bool const &farField,
			bool const &bufferDelays)
  {
    farField_p     = farField;
    bufferDelays_p = bufferDelays;

    antPositions_p.resize(antPositions.shape());
    antPositions_p = antPositions;

    skyPositions_p.resize(skyPositions.shape());
    skyPositions_p = skyPositions;
  }

  //_____________________________________________________________________________
  //                                                                      summary
  
  void GeomDelay::summary (std::ostream &os)
  {
    os << "[GeomDelay] Summary of internal parameters." << std::endl;
    os << "-- Far-field delay        = " << farField_p            << std::endl;
    os << "-- Buffer delay values    = " << bufferDelays_p        << std::endl;
    os << "-- nof. antenna positions = " << nofAntennaPositions() << std::endl;
    os << "-- nof. sky positions     = " << nofSkyPositions()     << std::endl;
  }
  
  
  // ============================================================================
  //
  //  Methods
  //
  // ============================================================================

  //_____________________________________________________________________________
  //                                                                       delays
  
  Matrix<double> GeomDelay::delays ()
  {
    if (bufferDelays_p) {
      return delays_p;
    } else {
      return delay (antPositions_p,
		    skyPositions_p,
		    farField_p);
    }
  }
  
  //_____________________________________________________________________________
  //                                                                        delay
  
  double GeomDelay::delay (std::vector<double> const &antPosition,
			   std::vector<double> const &skyPosition,
			   bool const &farField)
  {
    double delay = 0;
    
    if (farField) {
      std::vector<double> direction (3);
      CR::normalize (direction,skyPosition);
      delay = -(direction[0]*antPosition[0]
		+direction[1]*antPosition[1]
		+direction[2]*antPosition[2])/lightspeed;
    }
    else {
      std::vector<double> diff (3);
      diff[0] = skyPosition[0]-antPosition[0];
      diff[1] = skyPosition[1]-antPosition[1];
      diff[2] = skyPosition[2]-antPosition[2];
      delay = (CR::L2Norm(diff)-CR::L2Norm(skyPosition))/lightspeed;
    }
    
    return delay;
  }

  //_____________________________________________________________________________
  //                                                                        delay
  
  double GeomDelay::delay (casa::Vector<double> const &antPosition,
			   casa::Vector<double> const &skyPosition,
			   bool const &farField)  
  {
    double delay = 0;
    
    if (farField) {
      casa::Vector<double> direction (3);
      CR::normalize (direction,skyPosition);
      delay = -(direction(0)*antPosition(0)
		+direction(1)*antPosition(1)
		+direction(2)*antPosition(2))/lightspeed;
    }
    else {
      casa::Vector<double> diff = skyPosition-antPosition;
      delay = (CR::L2Norm(diff)-CR::L2Norm(skyPosition))/lightspeed;
    }
    
    return delay;
  }

  //_____________________________________________________________________________
  //                                                                        delay
  
  Matrix<double> GeomDelay::delay (casa::Matrix<double> const &antPositions,
				   casa::Matrix<double> const &skyPositions,
				   bool const &farField)
  {
    unsigned int nofAnt = antPositions.nrow();
    unsigned int nofSky = skyPositions.nrow();
    Matrix<double> delays (nofAnt,nofSky);
    
    for (unsigned int ant(0); ant<nofAnt; ant++) {
      for (unsigned int sky(0); sky<nofSky; sky++) {
	delays(ant,sky) = delay (antPositions.row(ant),
				 skyPositions.row(sky),
				 farField);
      }
    }

    return delays;
  }

  
} // Namespace CR -- end
