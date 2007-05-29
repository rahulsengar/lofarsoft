/***************************************************************************
 *   Copyright (C) 2007                                                    *
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

/* $Id: template-tclass.cc,v 1.7 2007/03/06 14:53:26 bahren Exp $*/

#include <Math/RotationMatrix.h>

using CR::RotationMatrix;

/*!
  \file tRotationMatrix.cc

  \ingroup Math

  \brief A collection of test routines for RotationMatrix
 
  \author Lars Baehren
 
  \date 2007/05/29
*/

// -----------------------------------------------------------------------------

/*!
  \brief Test constructors for a new RotationMatrix object

  \return nofFailedTests -- The number of failed tests.
*/
int test_RotationMatrix ()
{
  int nofFailedTests (0);
  
  std::cout << "\n[test_RotationMatrix]\n" << std::endl;

  std::cout << "[1] Testing default constructor ..." << std::endl;
  try {
    RotationMatrix newObject;
    newObject.summary();
  } catch (std::string message) {
    std::cerr << message << std::endl;
    nofFailedTests++;
  }
  
  std::cout << "[2] Testing argumented constructor ..." << std::endl;
  try {
    RotationMatrix rotation2d (2);
    RotationMatrix rotation3d (3);

    std::cout << "--> Rotation in 2 dimensions" << std::endl;
    rotation2d.summary();
    std::cout << "--> Rotation in 3 dimensions" << std::endl;
    rotation3d.summary();
  } catch (std::string message) {
    std::cerr << message << std::endl;
    nofFailedTests++;
  }
  
  std::cout << "[2] Testing argumented constructor ..." << std::endl;
  try {
    unsigned int rank (2);
    vector<double> angles (1,45.0);
    RotationMatrix rotation2d (rank,angles);
    rotation2d.summary();
  } catch (std::string message) {
    std::cerr << message << std::endl;
    nofFailedTests++;
  }
  
  return nofFailedTests;
}

// -----------------------------------------------------------------------------

int main ()
{
  int nofFailedTests (0);

  // Test for the constructor(s)
  {
    nofFailedTests += test_RotationMatrix ();
  }

  return nofFailedTests;
}
