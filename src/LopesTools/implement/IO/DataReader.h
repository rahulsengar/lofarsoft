/***************************************************************************
 *   Copyright (C) 2006                                                    *
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

/* $Id: DataReader.h,v 1.22 2006/08/17 15:33:33 bahren Exp $*/

#ifndef DATAREADER_H
#define DATAREADER_H

#include <iostream>
#include <fstream>

#include <casa/aips.h>
#include <casa/iostream.h>
#include <casa/fstream.h>
#include <casa/string.h>
#include <casa/Arrays.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayIO.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Containers/Record.h>
#include <casa/Quanta.h>
#include <scimath/Mathematics.h>
#include <scimath/Mathematics/FFTServer.h>

#include <lopes/IO/DataIterator.h>
#include <lopes/Functionals/HanningFilter.h>

#include <casa/namespace.h>

/*!
  \class DataReader

  \ingroup IO

  \brief Brief description for class DataReader

  \author Lars B&auml;hren

  \date 2006/05/04

  \test tDataReader.cc

  <h3>Prerequisite</h3>

  <ul type="square">
    <li>[AIPS++] <a href="http://aips2.nrao.edu/docs/scimath/implement/Mathematics/FFTServer.html">FFTServer</a>
    <li>[LOPES-Tools] DataIterator
  </ul>

  For a set of higher level construction/helper routines see DataReaderTools.

  <h3>Motivation</h3>

  Reading in data, processing them within the LOPES-Tools kernel (data-kernel.g)
  and passing them to a Glish client has proven to be the main bottleneck of
  large computations; typically the time spent on this part is much larger than
  e.g. the beamforming for the computation of a Skymap. In order to overcome
  this problem, (a) as little as possible computation should be done in Glish
  and (b) as little as possible data should be passed over the Glish bus
  connecting the (present) LOPES-Tools data kernel and the client program.

  The original motivation for designing this class was to have a facility for
  fast input of raw data (ADC values after signal conversion); recently the code
  has been expanded to also handle LOPES event files and -- at a later point --
  data formats as generated by the Aviary plugin infrastructure.
  
  <h3>Synopsis</h3>

  The functionality and logic for dealing with a certain data set is distributed
  over the base class (DataReader) and the derived classes (e.g. ITSCapture,
  LopesEvent).

  <table border="0" cellpadding="3">
    <tr bgcolor="silver">
      <td width="15%">Function</td>
      <td>Description</td>
    </tr>
    <tr valign="top">
      <td bgcolor="orange">DataReader::setStreams()</td>
      <td>
      This function serves as the central point where the input streams are
      set up to read in the data (from disk); a function prototype is
      provided in the base class as a virtual function, but then needs to
      be reimplemented in the derived classes to actually set the streams
      \code
      Bool LopesEvent::setStreams ()
      {
        bool status (true);
      
	uint nofSamples (blocksize_p);
	Vector<uint> antennas (length_);
	Vector<Float> adc2voltage (DataReader::adc2voltage());
	Matrix<Complex> fft2calfft (DataReader::fft2calfft());
	Vector<String> filenames (length_);
	DataIterator *iterator;

	// set and check the variables
	
	// initialize the internals of the underlying DataReader object
	DataReader::init (nofSamples,
	                  antennas,
			  adc2voltage,
			  fft2calfft,
			  filenames,
			  iterator);
	
	return status;
      }
      \endcode
      </td>
    </tr>
    <tr valign="top">
      <td bgcolor="red">DataReader::init()</td>
      <td>
      This is the core function by which the derived classes initialize the
      underlying DataReader object (see example above) -- it <b>must</b> be 
      called to ensure the internal structure of framework provided by the 
      DataReader is intact.
      </td>
    </tr>
    <tr valign="top">
      <td bgcolor="green">DataReader::setHanningFilter()</td>
      <td>
      A HanningFilter can optionally be inserted into the signal chain; by
      default it is switched off, so you will need to activate it first (see
      tDataReader.cc for further examples):
      \code
      {
        Float alpha (0.5);
	
	DataReader dr  (blocksize, adc2voltage, fft2calfft);
	dr.setHanningFilter (alpha);
	
	Matrix<Complex> fft_filtered (dr.fft());
      }
      \endcode
      </td>
    </tr>
  </table>

  <h3>Example(s)</h3>

  Using the DataReader with a client program as of this data requires two
  segments of code to be present:
  <ul>
    <li><b>Glish:</b> Your Glish script needs to set up a records containing
    all necessary information required for the successful creation of a
    DataReader object; this is done via
    \code
    RecordForDataReader (rec,param);
    \endcode
    where <tt>rec</tt> is (a reference to) the data record and <tt>param</tt>
    is the record passed along with the Glish bus event.
    <li><b>C/C++:</b> While the DataReader class contains the general framework
    for the processing of the data up to cross-correlation spectra, the data
    type specific details are implemented in a set of separate classes (e.g.
    ITSCapture, LopesEvent, ...). Parsing of the record passed along with the
    Glish event and proper construction of a DataReader object is wrapped by
    DataReaderTools::assignFromRecord -- therefor all you client needs is
    \code
    DataReader *dr;
    dr = DataReaderTools::assignFromRecord (glishRec,true);
    \endcode
    where of course <tt>dr</tt> is a pointer to a DataReader object,
    <tt>glishRec</tt> is the record passed along with the Glish event and the
    optional parameter <tt>verbose=true</tt> enables the verbose mode of 
    DataReaderTools::assignFromRecord. After this access to the data is as simple
    as:
    \code
    Matrix<Float> fx (dr->fx());
    Matrix<Float> voltage (dr->voltage());
    Matrix<Complex> fft (dr->fft());
    Matrix<Complex> calfft (dr->calfft());
    \endcode
*/

class DataReader {

  //! Record structure to store the meta data.
  Record header_p;

  //! Conversion from ADC values to voltages
  Vector<Float> adc2voltage_p;

  //! Conversion from raw to calibrated FFT
  Matrix<Complex> fft2calfft_p;

  //! Value of the ADC sampling rate, [Hz]
  Float samplingRate_p;

  //! Nyquist zone, in which the data have been sampled
  uint nyquistZone_p;

  //! Selection of the antennas
  Vector<uint> antennas_p;
  
  //! Enable/Disable frequency channel selection
  Bool selectChannels_p;

  //! Hanning filter to be applied to the time-domain data (optional)
  HanningFilter<Float> hanningFilter_p;

  //! Apply the Hanning filter to the data?
  Bool applyHanning_p;

  //! At which block of the data volume do we start reading data
  uint startBlock_p;

 protected:
  
  // ----------------------------------------------------------- array dimensions

  //! Size of a block of data, [samples]
  uint blocksize_p;

  //! Output length of the FFT, [Samples]
  uint fftLength_p;

  // --------------------------------------------------------------- data streams

  //! Number of filestream, from which data are read
  uint nofStreams_p;

  //! File streams connecting to the stored data.
  std::fstream *fileStream_p;
  
  //! Array of DataIterator objects for handling progressing through data volume
  DataIterator *iterator_p;
  
  /*!
    \brief Set up the streams for reading in the data

    \return status -- 
  */
  virtual Bool setStreams () {
    std::cout << "[DataReader::setStreams]" << std::endl;
    return True;
  };

  /*!
    \brief Initialize the internal structure of the DataReader

    \param blocksize   -- Size of a block of data, [samples]
    \param antennas    -- Antennas included in the experiment/observation and
                          containing valid data files
    \param adc2voltage -- Multiplication factors for conversion from ADC values
                          to voltages
    \param fft2calfft  -- Multiplication factors for conversion from raw to
                          calibrated FFT
    \param filenames   -- Names of the files from which to read in the data
    \param iterators   -- Set of DataIterator objects used for navigation within
                          the file streams.

    This is the primary method for the derived classes to initialize the
    underlying DataReader object handling conversion of data.
   */
  void init (uint const &blocksize,
	     Vector<uint> const &antennas,
	     Vector<Float> const &adc2voltage,
	     Matrix<Complex> const &fft2calfft,
	     Vector<String> const &filenames,
	     DataIterator const *iterators);
  
  // ----------------------------------------------------- (selected of) antennas

  //! Selection of the antennas
  Vector<bool> antennaSelection_p;
  Vector<uint> selectedAntennas_p;

  // ----------------------------------------------------- (selected of) channels
  
  //! Selection of the frequency channels
  Vector<uint> selectedChannels_p;

 public:

  // --------------------------------------------------------------- Construction

  /*!
    \brief Default constructor

    This is the default constructor for a new DataReader object; this will not
    enable the actual reading of data, but otherwise represents a fully
    operational object.

    \verbatim
    - blocksize   = 1
    - FFT length  = 1
    - nof. files  = 1
    - adc2voltage = [1]
    - fft2calfft  = [(1,0)]
    \endverbatim
  */
  DataReader ();

  /*!
    \brief Argumented constructor

    \param blocksize   -- Size of a block of data, [samples]
  */
  DataReader (uint const &blocksize);

  /*!
    \brief Argumented constructor

    \param blocksize   -- Size of a block of data, [samples]
  */
  DataReader (uint const &blocksize,
	      uint const &nyquistZone,
	      Float const &samplerate);

  /*!
    \brief Argumented constructor

    \param blocksize   -- Size of a block of data, [samples]
    \param adc2voltage -- Multiplication factors for conversion from ADC values
                          to voltages
    \param fft2calfft  -- Multiplication factors for conversion from raw to
                          calibrated FFT
  */
  DataReader (uint const &blocksize,
	      Vector<Float> const &adc2voltage,
	      Matrix<Complex> const &fft2calfft);

  /*!
    \brief Argumented constructor

    \todo implentation incomplete

    \param filename    -- Name of the file from which to read in the data
    \param blocksize   -- Size of a block of data, [samples]
    \param var         -- Variable type as which the data are stored in the file
    \param adc2voltage -- Multiplication factors for conversion from ADC values
                          to voltages
    \param fft2calfft  -- Multiplication factors for conversion from raw to
                          calibrated FFT
  */
  template <class T> 
    DataReader (String const &filename,
		uint const &blocksize,
		T const &var,
		Vector<Float> const &adc2voltage,
		Matrix<Complex> const &fft2calfft);
  
  /*!
    \brief Copy constructor

    \todo Based on (a) the problem of creating a copy of a stream - what does
    this actually mean - and (b) the triggered discussion with Marcel and Ger
    there a good argumentes to question while we should have a copy
    constructor/function for this class; as long as we are just read from disk
    files things can be done conceptually, but as soon as we make the
    generalization to input streams, copy no longer makes sense.

    \param other -- Another DataReader object from which to create this new
                    one.
  */
  DataReader (DataReader const &other);

  // ---------------------------------------------------------------- Destruction

  /*!
    \brief Destructor
  */
  virtual ~DataReader ();

  // ------------------------------------------------------------------ Operators

  /*!
    \brief Overloading of the copy operator

    \param other -- Another DataReader object from which to make a copy.
  */
  DataReader& operator= (DataReader const &other); 

  // ----------------------------------------------------------------- Parameters

  /*!
    \brief Get the number of files to which the data streams are connected

    \return nofStreams -- Number of streams used for reading in the data.
  */
  uint nofStreams () const {
    return nofStreams_p;
  }

  /*!
    \brief Get the size of a block of data per antenna

    \return blocksize -- Size of a block of data per antenna, [samples].
  */
  uint blocksize () const {
    return blocksize_p;
  }

  /*!
    \brief Set the size of a data block (in samples).
    
    \param blocksize -- The size of a block of data [samples].
  */    
  void setBlocksize (uint const &blocksize);

  /*!
    \brief Get the output length of the FFT
    
    \return fftLength -- Output length of the FFT, [Samples]
  */
  uint fftLength () const {
    return fftLength_p;
  }

  /*!
    \brief Get the ADC sampling rate

    \return samplingRate -- The value of the ADC sampling rate, [Hz]
  */
  Float samplingRate () const {
    return samplingRate_p;
  }
  
  /*!
    \brief Get the ADC sampling rate

    \param unit -- The frequency unit in which to return the sampling rate
    
    \return samplingRate -- The value of the ADC sampling rate, in units of
                            <i>unit</i>
  */
  Float samplingRate (String const &unit);
  
  /*!
    \brief Set the ADC sampling rate

    \param samplingRate -- The value of the ADC sampling rate, [Hz]
  */
  void setSamplingRate (Float const &samplingRate);

  /*!
    \brief Set the ADC sampling rate
    
    \param samplingRate -- The value of the ADC sampling rate
  */
  void setSamplingRate (Quantity const &samplingRate);

  /*!
    \brief Get the Nyquist zone, in which the data were sampled

    \return nyquistZone -- The Nyquist zone, in which the data were sampled
   */
  uint nyquistZone () const {
    return nyquistZone_p;
  }

  /*!
    \brief Set the Nyquist zone, in which the data were sampled
    
    \param nyquistZone -- The Nyquist zone, in which the data were sampled
  */
  void setNyquistZone (uint const &nyquistZone) {
    nyquistZone_p = nyquistZone;
  }
  
  /*!
    \brief Weights for conversion from raw ADC samples to voltages

    \return fx2voltage -- Weights to convert raw ADC samples to voltages
  */
  Vector<Float> adc2voltage () const {
    return adc2voltage_p;
  }

  /*!
    \brief Set the weights for conversion from raw ADC samples to voltages

    Kepp in mind, that there two possible shapes in which the data will be 
    accepted:
    <ol>
      <li>nelem == nofSelectedAntennas() : only conversion values for the 
      selected antennas are provided.
      <li>nelem == nofAntennas() : conversion values for all antennas in the 
      dataset are provided; this is the more secure method, as this will allow
      for later re-selection of antennas without affecting the proper conversion
      from ADC values to voltages.
    </ol>

    \param fx2voltage -- Weights to convert raw ADC samples to voltages
  */
  void setADC2voltage (Vector<Float> const &adc2voltage);

  /*!
    \brief Weights for conversion from raw to calibrated FFT

    \return fft2calfft -- Weights to convert the output of the Fourier
                          transform on the voltages to calibrated spectra,
			  accounting for the slope of the bandpass filter.
  */
  Matrix<Complex> fft2calfft () const {
    return fft2calfft_p;
  }

  /*!
    \brief Set the weights for conversion from raw to calibrated FFT

    \param fft2calfft -- Weights to convert the output of the Fourier
                         transform on the voltages to calibrated spectra,
			 accounting for the slope of the bandpass filter.
  */
  void setFFT2calFFT (Matrix<Complex> const &fft2calfft);

  /*!
    \brief Get the record with the header information

    \return header -- Record containing the header information
  */
  Record header () const {
    return header_p;
  };

  /*!
    \brief Set the record with the header information

    \todo We need to define, which standard fields should be present/expected
    in a header record.

    \param header -- Record containing the header information
  */
  void setHeader (Record const &header);

  // ----------------------------------------- Navigation through the data volume

  /*!
    \brief Get the number of the block at which we start reading the data

    \param startBlock -- The number of the block at which we start reading the
                         data
  */
  void setStartBlock (uint const &startBlock);

  /*!
    \brief Set a specific data block to read next
  */
  void setBlock (uint const &block);

  /*!
    \brief Set the stride between two subsequent blocks of data 

    \param stride -- Stride between two subsequent blocks of data 
  */
  void setStride (uint const & stride);

  /*!
    \brief Set the shift between the start of the data segment and the first block.

    \param shift -- \f$ N_{\rm Shift} \f$, a shift w.r.t. \f$ N_{\rm Start} \f$
  */
  void setShift (uint const & shift);

  /*!
   \brief Forward position pointer to the next block of data
   */
  void nextBlock ();

  /*!
    \brief Go back to the block at which we start reading the data
  */
  void toStartBlock () {
    setBlock (startBlock_p);
  }

  // -------------------------------------------------------------------- Methods

  /*!
    \brief Get the raw time series after ADC conversion
    
    \return fx -- Raw ADC time series, [Counts]
  */
  virtual Matrix<Float> fx ();

  /*!
    \brief Get the voltage time series
    
    \return voltage -- Voltage time series, [Volt]
  */
  virtual Matrix<Float> voltage ();

  /*!
    \brief Raw FFT of the voltage time series
    
    \return fft -- Raw FFT of the voltage time series
  */
  virtual Matrix<Complex> fft ();

  /*!
    \brief Get the calibrated FFT
    
    \return calfft -- Calibrated FFT, i.e. spectra after correction for the antenna
                      gain-curves.
  */
  virtual Matrix<Complex> calfft ();

  /*!
    \brief Get the cross-correlation spectra

    \param fromCalFFT -- Cross-correlation spectra from the calibrated FFT data? If
                         set to <i>False</i>, the cross-correlation is carried out on
			 the raw FFT.

    \return ccSpectra -- Data cube with the cross-correlation spectra,
                         [nfreq,nant,nant]
  */
  virtual Cube<Complex> ccSpectra (Bool const &fromCalFFT=True);

  /*!
    \brief Get the visibilities

    \param fromCalFFT -- Cross-correlation spectra from the calibrated FFT data? If
                         set to <i>False</i>, the cross-correlation is carried out
			 on the raw FFT.

    \return vis -- The visibilities

    \todo The antenna combinations for the baselines must be provided as well;
    either this information is stored internally, when computing the visibilities
    or is computed on the fly when requested.
    \todo We need an additional switch, to include/exclude the auto-correlation
    products (this feature is required when passing data to the MSSimulator).
  */
  virtual Matrix<Complex> visibilities (Bool const &fromCalFFT=True);

  // ------------------------------------------------------ Selection of antennas

  /*!
    \brief Number of antennas available in the data set.

    \return nofAntennas -- Number of antennas available in the data set.
  */
  uint nofAntennas () const {
    return antennas_p.nelements();
  }

  /*!
    \brief Get the IDs of the available antennas

    \return antennas -- IDs of the antennas available in the data set.
  */
  Vector<uint> antennas () const {
    return antennas_p;
  }

  /*!
    \brief Define the set of available antennas in the data set.

    \param antennas -- Vector with the numbers of the available antennas

    \return status -- Status of the operation; returns <i>true</i> if everything
                      went fine.
  */
  bool setAntennas (Vector<uint> const &antennas);

  /*!
    \brief Define the set of available antennas in the data set.

    \param antennas         -- 
    \param selectedAntennas -- 

    \return status -- Status of the operation; returns <i>true</i> if everything
                      went fine.
  */
  bool setAntennas (Vector<uint> const &antennas,
		    Vector<uint> const &selectedAntennas);

  /*!
    \brief Define the set of available antennas in the data set.

    \param antennas         -- Vector with the numbers of the available antennas
    \param antennaSelection -- 

    \return status -- Status of the operation; returns <i>true</i> if everything
                      went fine.
  */
  bool setAntennas (Vector<uint> const &antennas,
		    Vector<bool> const &antennaSelection);

  /*!
    \brief Return array with the antenna selection flags

    \return antennaSelection -- Antenna selection flags
  */
  Vector<bool> antennaSelection () const {
    return antennaSelection_p;
  }

  /*!
    \brief 

    \return 
  */
  uint nofSelectedAntennas () const {
    return selectedAntennas_p.nelements();
  }

  /*!
    \brief Get the number of baselines

    \param allAntennas 

    \return nofBaselines
  */
  uint nofBaselines (bool const &allAntennas=false) const;

  /*!
   \brief Selection of the antennas in the dataset
   
   \return antennaSelection -- Selection of the antennas in the dataset
  */
  Vector<uint> selectedAntennas () const {
    return selectedAntennas_p;
  }

  /*!
   \brief Selection of the antennas in the dataset
   
   \param antennaSelection -- Selection of the antennas in the dataset
  */
  Bool setSelectedAntennas (Vector<uint> const &antennaSelection,
			    bool const &absolute=true);

  /*!
   \brief Selection of the antennas in the dataset
   
   \param antennaSelection -- Selection of the antennas in the dataset
  */
  Bool setSelectedAntennas (Vector<Bool> const &antennaSelection);

  // -------------------------------------------- Selection of frequency channels

  Vector<uint> selectedChannels () const {
    return selectedChannels_p;
  }

  /*!
    \brief Set the numbers of the frequency channels selected for processing

    \param channelSelection -- 

    \return status -- 
  */
  Bool setSelectedChannels (Vector<uint> const &channelSelection);

  /*!
    \brief Set the numbers of the frequency channels selected for processing

    \param channelSelection -- 

    \return status -- 
  */
  Bool setSelectedChannels (Vector<Bool> const &channelSelection);

  /*!
    \brief Get the number of selected frequency channels

    \return nofSelectedChannels -- The number of selected frequency channels
  */
  uint nofSelectedChannels () const {
    return selectedChannels_p.nelements();
  };

  /*!
    \brief Get the frequency values corresponding to the channels in the FFT

    \param onlySelected -- Return the frequency values only for the selected 
                           frequency channels; otherwise the complete frequency
			   band in the FFT is covered

    \return frequency -- Frequency values corresponding to the channels in the
                         FFT, [Hz]
   */
  Vector<Double> frequencyValues (Bool const &onlySelected=True);

  /*!
    \brief Enable/Disable the Hanning filter

    \param alpha -- Slope parameter of the HanningFilter.

    Insert a Hanning filter before the the Fourier transform; the slope
    parameter \f$ \alpha \f$ can be used to obtain a Hanning- or a
    Hammingfilter. To disable the HanningFilter call this function with
    \f$ \alpha = 0 \f$.
   */
  void setHanningFilter (const Float& alpha);
  /*!
    \brief Enable/Disable the Hanning filter

    \param alpha -- Slope parameter of the HanningFilter.
    \param beta  -- Width of the plateau with w[n]=1.
  */
  void setHanningFilter (const Float &alpha,
			 const uint &beta);
  
 private:

  /*!
    \brief Unconditional copying
  */
  void copy (DataReader const &other);

  /*!
    \brief Unconditional deletion 
  */
  void destroy(void);

  /*!
    \brief Initialization to default parameters
    
    \param blocksize   -- Size of a block of data, [samples]
  */
  void init (uint const &blocksize);

  /*!
    \brief Initialization
    
    \param blocksize   -- Size of a block of data, [samples]
  */
  void init (uint const &blocksize,
	     uint const &nyquistZone,
	     Float const &samplerate);
  
  /*!
    \brief Initialization of internal parameters

    \param blocksize   -- Size of a block of data, [samples]
    \param adc2voltage -- 
    \param fft2calfft  -- 
  */
  void init (uint const &blocksize,
	     Vector<Float> const &adc2voltage,
	     Matrix<Complex> const &fft2calfft);
    
  /*!
    \brief Store the output length of the FFT

    This in done directly using an instance of the FFT server; this way we ensure
    that the stored number indeed agrees with the shape returned by the FFT
    server.
   */
  void setFFTLength ();

  /*!
    \brief Select the frequency channels
  */
  Matrix<Complex> selectChannels (Matrix<Complex> const &data);
};

#endif /* DATAREADERBASE_H */
