/***************************************************************************
 *   Copyright (C) 2004,2005                                               *
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

/* $Id: Masking.cc,v 1.8 2007/03/19 17:40:33 bahren Exp $ */

#include <Utilities/Masking.h>

namespace CR {
  
  // ----------------------------------------------------------------- mergeMasks
  
  Array<bool> mergeMasks (const Array<bool>& mask1,
			  const Array<bool>& mask2,
			  const String logic)
  {
    IPosition shape1 = mask1.shape();
    IPosition shape2 = mask2.shape();
    IPosition shapeFallback (1);
    Array<bool> mask;
    
    // Check if the shape of the two arrays is consistent
    if (shape1 != shape2) {
      cerr << "[mergeMasks] Shape of the two are is inconsistent!" << endl;
      cerr << "\tArray 1 : " << shape1 << endl;
      cerr << "\tArray 2 : " << shape2 << endl;
      mask.resize(shapeFallback);
      mask = false;
    }
    else {
      if (logic == "AND") {
	cout << "[mergeMasks] AND" << endl;
	mask = (mask1 && mask2);
      }
      else if (logic == "NAND") {
	cout << "[mergeMasks] NAND" << endl;
	mask = !(mask1 && mask2);
      }
      else if (logic == "OR") {
	cout << "[mergeMasks] OR" << endl;
	mask = (mask1 || mask2);
      }
      else if (logic == "NOR") {
	cout << "[mergeMasks] NOR" << endl;
	mask = !(mask1 || mask2);
      }
      else {
	cerr << "[mergeMasks] Unsupported merging option "
	   << logic << endl;
	mask.resize(shapeFallback);
	mask = false;
      }
    }
    return mask;
  }

// ------------------------------------------------------------------- setSubmask
  
  void setSubmask (Vector<bool>& outMask, 
		   Vector<bool> const &inMask,
		   Vector<int> const &inIndex)
  {
    int nOutMask (outMask.nelements());
    int nInIndex (inIndex.nelements());
    
    if (nInIndex == nOutMask) {
      for (int i=0; i<nOutMask; i++) {
	if (inMask(inIndex(i))) {
	  outMask(i) = true;
	} else {
	  outMask(i) = false;
	}
      }
    } else {
      outMask.resize(nOutMask);
      outMask = true;
    }
    
  }
  
  // ---------------------------------------------------------------- maskFromRange
  
  template <class T>
  Vector<bool> maskFromRange (Vector<T> const &range,
			      Vector<T> const &values)
  {
    IPosition shapeRange = range.shape();
    unsigned int nofValues = values.nelements();
    Vector<bool> mask(nofValues,false);
    
    // Step through all alement of the data vector and check if they lie within
    // the specified range.
    if (range.nelements() == 2) {
      T rangeMin = min(range);
      T rangeMax = max(range);
      for (unsigned int k=0; k<nofValues; k++) {
	if (values(k) >= rangeMin && values(k) <= rangeMax) {
	  mask(k) = true;
	}
      }
    } else {
      cerr << "[maskFromRange] Invalid number of element in range vector"
	   << endl;
    }
    
    return mask;
  }
  
  // -------------------------------------------------------------- validElements
  
  unsigned int validElements (const Vector<bool>& mask) 
  {
    unsigned int nofElements (mask.nelements());
    unsigned int validElements (0);
    
    for (unsigned int i=0; i<nofElements; i++) {
      validElements += (unsigned int)(mask(i));
    }
    
    return validElements;
  }
  
  // ------------------------------------------------------------ invalidElements
  
  unsigned int invalidElements (const Vector<bool>& mask) 
  {
    unsigned int nofElements = mask.nelements();
    unsigned int invalidElements (nofElements);
    
    for (unsigned int i=0; i<nofElements; i++) {
      invalidElements -= (unsigned int)(mask(i));
    }
    
    return invalidElements;
  }
  
  // ----------------------------------------------------- Template instantiation
  
  template Vector<bool> maskFromRange (const Vector<int>& range,
				       const Vector<int>& values);
  template Vector<bool> maskFromRange (const Vector<float>& range,
				       const Vector<float>& values);
  template Vector<bool> maskFromRange (const Vector<double>& range,
				       const Vector<double>& values);
  
}
