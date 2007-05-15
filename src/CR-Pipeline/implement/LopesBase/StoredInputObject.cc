/***************************************************************************
 *   Copyright (C) 2005                                                  *
 *   Andreas Horneffer (<mail>)                                                     *
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

/* $Id: StoredInputObject.cc,v 1.4 2007/05/02 08:14:15 bahren Exp $*/

#include <LopesBase/StoredInputObject.h>

using std::cerr;
using std::endl;

namespace CR {  // Namespace CR -- begin
  
  // --- Construction ------------------------------------------------------------
  
  template<class T> StoredInputObject<T>::StoredInputObject ()
    : StoredObject<T>() {
    this->className_p = "StoredInputObject";
    this->nParent_p = 0;
  }
  
  // --- Destruction -------------------------------------------------------------
  
  template<class T> StoredInputObject<T>::~StoredInputObject ()
  {;}
  
  // --- Parameters --------------------------------------------------------------
  
  template<class T> bool StoredInputObject<T>::put(Vector<T> *ref) {
    try {
      this->data_p.resize();
      this->data_p = *ref;
      this->invalidate();
      this->valid_p = true;
    } catch (AipsError x) {
      cerr << x.getMesg() << endl;
      return false;
    };
    return true;
  };
  
  template<class T> bool StoredInputObject<T>::put(Vector<T> *ref,
						   bool copy) {
    try {
      if (copy) {
	this->data_p.resize();
	this->data_p = *ref;
      } else {
	this->data_p.reference(*ref);
	this->invalidate();
	this->valid_p = true;
      };
    } catch (AipsError x) {
      cerr << x.getMesg() << endl;
      return false;
    };
    return true;
  };
  
  
  
  // ==============================================================================
  //
  // Template Instantiation
  //
  // ==============================================================================
  
  template class StoredInputObject<casa::Double>;
  template class StoredInputObject<casa::DComplex>;
  
}  // Namespace CR -- end
