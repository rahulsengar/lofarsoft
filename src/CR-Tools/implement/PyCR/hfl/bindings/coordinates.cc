/**************************************************************************
 *  This file is part of the Heino Falcke Library.                        *
 *  Copyright (C) 2010 Pim Schellart <P.Schellart@astro.ru.nl>            *
 *                                                                        *
 *  This library is free software: you can redistribute it and/or modify  *
 *  it under the terms of the GNU General Public License as published by  *
 *  the Free Software Foundation, either version 3 of the License, or     *
 *  (at your option) any later version.                                   *
 *                                                                        * 
 *  This library is distributed in the hope that it will be useful,       *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *  GNU General Public License for more details.                          *
 *                                                                        *
 *  You should have received a copy of the GNU General Public License     *
 *  along with this library. If not, see <http://www.gnu.org/licenses/>.  *
 **************************************************************************/

// SYSTEM INCLUDES
//

// PROJECT INCLUDES
#include <hfl/coordinates.h>

// LOCAL INCLUDES
#define PY_ARRAY_UNIQUE_SYMBOL PyArrayHandle
#include "num_util.h"
#include "numpy_ptr.h"

// FORWARD REFERENCES
//

namespace hfl
{
    namespace bindings
    {

    bool pixel2World(boost::python::numeric::array world, boost::python::numeric::array pixel, const std::string refcode, const std::string projection, const double refLong, const double refLat, const double incLong, const double incLat, const double refX, const double refY, const double lonPole, const double latPole)
    {
      return hfl::coordinates::pixel2World(hfl::numpyBeginPtr<double>(world), hfl::numpyEndPtr<double>(world), hfl::numpyBeginPtr<double>(pixel), hfl::numpyEndPtr<double>(pixel), refcode, projection, refLong, refLat, incLong, incLat, refX, refY, lonPole, latPole);
    }

    bool world2Pixel(boost::python::numeric::array pixel, boost::python::numeric::array world, const std::string refcode, const std::string projection, const double refLong, const double refLat, const double incLong, const double incLat, const double refX, const double refY)
    {
      return hfl::coordinates::world2Pixel(hfl::numpyBeginPtr<double>(pixel), hfl::numpyEndPtr<double>(pixel), hfl::numpyBeginPtr<double>(world), hfl::numpyEndPtr<double>(world), refcode, projection, refLong, refLat, incLong, incLat, refX, refY);
    }

    void azElRadius2Cartesian(boost::python::numeric::array out, boost::python::numeric::array in, bool anglesInDegrees)
    {
      hfl::coordinates::azElRadius2Cartesian(hfl::numpyBeginPtr<double>(out), hfl::numpyEndPtr<double>(out), hfl::numpyBeginPtr<double>(in), hfl::numpyEndPtr<double>(in), anglesInDegrees);
    }

    void equatorial2Horizontal(boost::python::numeric::array hc, boost::python::numeric::array ec, const double utc, const double ut1_utc, const double L, const double phi)
    {
      hfl::coordinates::equatorial2Horizontal(hfl::numpyBeginPtr<double>(hc), hfl::numpyEndPtr<double>(hc), hfl::numpyBeginPtr<double>(ec), hfl::numpyEndPtr<double>(ec), utc, ut1_utc, L, phi);
    }

    } // End bindings
} // End hfl

BOOST_PYTHON_MODULE(coordinates)
{
  import_array();
  boost::python::numeric::array::set_module_and_type("numpy", "ndarray");

  using namespace boost::python;

  boost::python::register_exception_translator<hfl::TypeError>(hfl::exceptionTranslator);

  def("pixel2World", hfl::bindings::pixel2World, "Get world coordinates for given vector of pixel coordinates\n\n*world* begin iterator for world coordinate array\n*world_end* end iterator for world coordinate array\n*pixel* begin iterator for pixel coordinate array\n*pixel_end* end iterator for pixel coordinate array\n*refcode* reference code for coordinate system e.g. AZEL,J2000,...\n*projection* the projection used e.g. SIN,STG,...\n*refLong* reference value for longtitude (CRVAL)\n*refLat* reference value for latitude (CRVAL)\n*incLong* \n*incLat* increment value for latitude (CDELT)\n*refX* reference x pixel (CRPIX)\n*refY* reference y pixel (CRPIX)\n*lonPole* \n*latPole* \n");
  def("world2Pixel", hfl::bindings::world2Pixel, "Get pixel coordinates for given vector of world coordinates\n\n*pixel* begin iterator for pixel coordinate array\n*pixel_end* end iterator for pixel coordinate array\n*world* begin iterator for world coordinate array\n*world_end* end iterator for world coordinate array\n*refcode* reference code for coordinate system e.g. AZEL,J2000,...\n*projection* the projection used e.g. SIN,STG,...\n*refLong* reference value for longtitude (CRVAL)\n*refLat* reference value for latitude (CRVAL)\n*incLong* \n*incLat* increment value for latitude (CDELT)\n*refX* reference x pixel (CRPIX)\n*refY* reference y pixel (CRPIX)\n");
  def("azElRadius2Cartesian", hfl::bindings::azElRadius2Cartesian, "Given an array of (az, el, r, az, el, r, ...) coordinates\n      return an array of (x, y, z, x, y, z, ...) coordinates.\n\n*out* \n*out_end* \n*in* \n*in_end* \n*anglesInDegrees* \n");
  def("equatorial2Horizontal", hfl::bindings::equatorial2Horizontal, "Convert array of equatorial J2000 coordinates to horizontal,\n      AZEL coordinates.\n\n             if 0 a maximum error of 0.9 seconds is made.\n\n*hc* array with horizontal coordiates (alt, az, alt, az, ...)\n*hc_end* \n*ec* array with equatorial coordinates (ra, dec, ra, dec, ...)\n*ec_end* \n*utc* UTC as Julian Day\n*ut1_utc* difference UT1-UTC (as obtained from IERS bullitin A)\n*L* longitude of telescope\n*phi* latitude of telescope\n");
}

