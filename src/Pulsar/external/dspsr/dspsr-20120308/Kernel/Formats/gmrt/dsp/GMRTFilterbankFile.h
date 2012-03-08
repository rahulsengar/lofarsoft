//-*-C++-*-
/***************************************************************************
 *
 *   Copyright (C) 2002 by Willem van Straten
 *   Licensed under the Academic Free License version 2.1
 *
 ***************************************************************************/

/* $Source: /cvsroot/dspsr/dspsr/Kernel/Formats/gmrt/dsp/GMRTFilterbankFile.h,v $
   $Revision: 1.1 $
   $Date: 2009/03/02 17:27:35 $
   $Author: straten $ */


#ifndef __GMRTFilterbankFile_h
#define __GMRTFilterbankFile_h

#include "dsp/File.h"

namespace dsp {

  //! Loads BitSeries data from a GMRT filterbank data file.
  class GMRTFilterbankFile : public File 
  {
  public:
   
    //! Construct and open file
    GMRTFilterbankFile (const char* filename = 0);

    //! Returns true if filename appears to name a valid GMRTFilterbank file
    bool is_valid (const char* filename) const;

  protected:

    //! Open the file
    void open_file (const char* filename);

  };

}

#endif // !defined(__GMRTFilterbankFile_h)
  
