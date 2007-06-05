/***************************************************************************
 *   Copyright (C) 2005                                                    *
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

/* $Id: Algebra.cc,v 1.2 2006/10/31 18:24:08 bahren Exp $*/

#include <cmath>
#include <Math/VectorNorms.h>

namespace CR { // Namespace CR -- begin
  
  // ============================================================================
  //
  //  Vector norms
  //
  // ============================================================================

  // --------------------------------------------------------------------- L1Norm
  
  template <class T>
  T L1Norm (T const *vec,
	    unsigned int const &nelem)
  {
    T sum (0.0);
    
    for (unsigned int n(0); n<nelem; n++) {
      sum += fabs(vec[n]);
    }
    
    return sum;
  }
  
  // --------------------------------------------------------------------- L2Norm
  
  template <class T>
  T L2Norm (T const *vec,
	    unsigned int const &nelem)
  {
    T sum2 (0.0);
    
    for (unsigned int n(0); n<nelem; n++) {
      sum2 += vec[n]*vec[n];
    }
    
    return sqrt (sum2);
  }
  
  // --------------------------------------------------------------------- L1Norm
  
  template <class T>
  T L1Norm (std::vector<T> const &vec)
  {
    unsigned int nelem(vec.size());
    T sum (0.0);
    
    for (unsigned int n(0); n<nelem; n++) {
      sum += fabs(vec[n]);
    }
    
    return sum;
  }
  
#ifdef HAVE_BLITZ

  template <class T>
  T L1Norm (blitz::Array<T,1> const &vec)
  {
    return sum(fabs(vec));
  }
  
  template <class T>
  double L2Norm (blitz::Array<T,1> const &vec)
  {
    return sqrt(sum(pow2(vec)));
  }

#endif

#ifdef HAVE_CASA

  template <class T>
  T L1Norm (const casa::Vector<T> &vec)
  {
    double norm (0.0);
    norm = sqrt(sum(fabs(vec)));
    return norm;
  }

  template <class T>
  double L2Norm (const casa::Vector<T> &vec)
  {
    double norm (0.0);
    norm = sqrt(sum(pow(vec,2)));
    return norm;
  }
  
#endif

  // ============================================================================
  //
  //  Sign (of vector elements)
  //
  // ============================================================================
  
  // ----------------------------------------------------------------------- sign

  template <class T>
  T sign (T const &x)
  {
    if (x<0) return static_cast<T>(-1);
    else if (x>0) return static_cast<T>(+1);
    else return static_cast<T>(0);
  }
  
#ifdef HAVE_CASA
  
  // ----------------------------------------------------------------------- sign
  
  template <class T>
  casa::Vector<T> sign (casa::Vector<T> const &x)
  {
    int nelem (x.nelements());
    casa::Vector<T> s(nelem);

    for (int n=0; n<nelem; ++n) {
      s(n) = sign (x(n));
    }

    return s;
  }

  // ---------------------------------------------------------------- invertOrder

  template <class T>
  casa::Vector<T> invertOrder (casa::Vector<T> const &vec)
  {
    int nelements (vec.nelements());
    casa::Vector<T> inv (nelements);
    int n (0);
    int m (nelements-1);

    for (n=0; n<nelements; n++, m--) {
      inv(m) = vec(n);
    }
    
    return inv;
  }
 
#endif

  // ============================================================================
  //
  //  Template instantiation
  //
  // ============================================================================

  template float L1Norm (float const *vec, unsigned int const &nelem);
  template double L1Norm (double const *vec, unsigned int const &nelem);

  template float L2Norm (float const *vec, unsigned int const &nelem);
  template double L2Norm (double const *vec, unsigned int const &nelem);

  template float L1Norm (std::vector<float> const &vec);
  template double L1Norm (std::vector<double> const &vec);

  template int sign (int const &x);
  template float sign (float const &x);
  template double sign (double const &x);

#ifdef HAVE_BLITZ

  template float L1Norm (const blitz::Array<float,1> &vec);
  template double L1Norm (const blitz::Array<double,1> &vec);

  template float L2Norm (const blitz::Array<float,1> &vec);
  template double L2Norm (const blitz::Array<double,1> &vec);

#endif

#ifdef HAVE_CASA

  template casa::Vector<int> sign (casa::Vector<int> const &x);
  template casa::Vector<float> sign (casa::Vector<float> const &x);
  template casa::Vector<double> sign (casa::Vector<double> const &x);
  
  template casa::Vector<String> invertOrder (casa::Vector<String> const &vec);
  template casa::Vector<uInt> invertOrder (casa::Vector<uInt> const &vec);
  template casa::Vector<int> invertOrder (casa::Vector<int> const &vec);
  template casa::Vector<float> invertOrder (casa::Vector<float> const &vec);
  template casa::Vector<double> invertOrder (casa::Vector<double> const &vec);
  template casa::Vector<Complex> invertOrder (casa::Vector<Complex> const &vec);
  template casa::Vector<DComplex> invertOrder (casa::Vector<DComplex> const &vec);

  template float L1Norm (const casa::Vector<float> &vec);
  template double L1Norm (const casa::Vector<double> &vec);

  template float L2Norm (const casa::Vector<float> &vec);
  template double L2Norm (const casa::Vector<double> &vec);

#endif

} // Namespace CR -- end
