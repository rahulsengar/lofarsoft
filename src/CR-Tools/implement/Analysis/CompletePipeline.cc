/*-------------------------------------------------------------------------*
 | $Id::                                                                 $ |
 *-------------------------------------------------------------------------*
 ***************************************************************************
 *   Copyright (C) 2008                                                    *
 *   Frank Schroeder (<mail>)                                              *
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

#include <Analysis/CompletePipeline.h>

namespace CR { // Namespace CR -- begin

  
  // ============================================================================
  //
  //  Construction
  //
  // ============================================================================
  
  // ----------------------------------------------------------- CompletePipeline
  
  CompletePipeline::CompletePipeline():
    plotStart_p(-2.05e-6),
    plotStop_p(-1.55e-6),
    ccWindowWidth_p(0.045e-6),
    plotlist(),
    lastUpsamplingExponent(-1),
    lastTimeUpsamplingExponent(-1),
    upFieldStrength(),
    upsampledAntennas(),
    upTimeValues(),
    Polarization("ANY")
  {;}
  
  // ----------------------------------------------------------- CompletePipeline
  
  CompletePipeline::CompletePipeline (CompletePipeline const &other):
    plotStart_p(-2.05e-6),
    plotStop_p(-1.55e-6),
    ccWindowWidth_p(0.045e-6),
    plotlist(),
    lastUpsamplingExponent(-1),
    lastTimeUpsamplingExponent(-1),
    upFieldStrength(),
    upsampledAntennas(),
    upTimeValues(),
    Polarization("ANY")
  {
    copy (other);
  }
  
  // ============================================================================
  //
  //  Destruction
  //
  // ============================================================================
  
  CompletePipeline::~CompletePipeline ()
  {
    destroy();
  }
  
  void CompletePipeline::destroy ()
  {;}
  
  // ============================================================================
  //
  //  Operators
  //
  // ============================================================================
  
  CompletePipeline& CompletePipeline::operator= (CompletePipeline const &other)
  {
    if (this != &other) {
      destroy ();
      copy (other);
    }
    return *this;
  }
  
  void CompletePipeline::copy (CompletePipeline const &other)
  {;}

  // ============================================================================
  //
  //  Parameters
  //
  // ============================================================================

  void CompletePipeline::summary (std::ostream &os)
  {;}


  // ============================================================================
  //
  //  Methods
  //
  // ============================================================================

  Vector<double> CompletePipeline::envelope(const Vector<double> &trace) const
  {
    Vector<double> envelope(trace.size(),0);	// create vector to return envelope

    try 
    {
      // get length of trace
      unsigned int tracelength = trace.size();

      // allocate memory for the trace
      float* envelopeTrace = new float[tracelength];

      // copy the trace into the array
      for (unsigned int i = 0; i < tracelength; i++) 
        envelopeTrace[i] = trace(i);

      // do envelope calculation (given trace will be overwritten!)
      RectifierHilbert(tracelength, envelopeTrace);

      // copy envelope from array to Vector
      for (unsigned int i = 0; i < tracelength; i++)
        envelope(i) = envelopeTrace[i];

      // delete arrays
      delete[] envelopeTrace;

    } catch (AipsError x) 
    {
        std::cerr << "CompletePipeline:envelope: " << x.getMesg() << std::endl;
    }

    return envelope;
  }


  void CompletePipeline::deselectectPolarization(DataReader *dr,
                                                 Vector<Bool> &antennaSelection)
  {
    try 
    {
      // Deselect antennas not matching the polarization (except if it is ANY)
      if (Polarization != "ANY")
      {
        // get the date of the current event
        unsigned int date;
        dr->header().get("Date",date);

        // get AntennaIDs from CalTables (should be initialised by FirstStagePipeline fist)
        Vector<Int> AntennaIDs;
        dr->header().get("AntennaIDs",AntennaIDs);

        String polarization_string;

        // loop through all AntennaIDs and deselect the ones with the wrong polarization
        for (unsigned int i=0; i<AntennaIDs.size(); i++)
        {
          CTRead->GetData(date, AntennaIDs(i), "Polarization", &polarization_string);
          if (polarization_string != static_cast<String>(Polarization)) {
            antennaSelection(i) = false;
            cout << "Deselected Antenna : " << i+1;
          }
        }
      } // if
    } catch (AipsError x) 
      {
        std::cerr << "CompletePipeline::deselectectPolarization " << x.getMesg() << std::endl;
      }
  }


  Matrix<Double> CompletePipeline::getUpsampledFieldstrength (DataReader *dr,
							      const int& upsampling_exp,
							      Vector<Bool> antennaSelection)
  {
    try 
    {
      // check if upsampling shoud be done at all (if not, return not upsampled data)
      if (upsampling_exp < 1) {
	return GetTimeSeries(dr, Vector<Bool>(), Polarization).copy(); 
      }

      // make antennaSelection unique, as casacore-Vectors are allways passed by reference
      antennaSelection.unique();

      // Get the antenna selection from the DataReader if no selction was chosen 	
      if (antennaSelection.nelements() == 0) {
	antennaSelection = GetAntennaMask(dr);
      }

      // check if upsampling is done for the first time
      if (lastUpsamplingExponent == -1)
      {
        //create vector for flags showing which antenna is allready upsampled
        upsampledAntennas.assign(GetAntennaMask(dr).nelements(),false);	// at the moment there are no upsampled antennas
      } 

      // consistency check: number of elements in antennaSelction and upsampledAntennas must be equal
      if ( upsampledAntennas.size() != antennaSelection.nelements() )
      {
        std::cerr << "CompletePipeline:getUpsampledFieldstrength: Number of elements in \"antennaSelection\" is inconsistent.\n" 
		<< std::endl;
        return upFieldStrength.copy();
      }

      // check if there are allready upsampled values and create todo-list
      vector<bool> upsamplingToDo;
      upsamplingToDo.assign(upsampledAntennas.size(),true);		// form Vector of correct size
      // maximum to do is the whole antenna selction
      for (unsigned int i=0; i < upsamplingToDo.size(); i++) {
	upsamplingToDo[i] = antennaSelection(i);
      }

      // if the last upsampling exponent is the same es the current, there might be somthing done allready
      // if not, clear the upsampled flags
      if (lastUpsamplingExponent != upsampling_exp) {
	upsampledAntennas.assign(upsampledAntennas.size(),false);
      } else for (unsigned int i=0; i < upsamplingToDo.size(); i++) {
	if (upsampledAntennas[i]) {
	  upsamplingToDo[i] = false;
	}
      }
      
      // Get the (not yet upsampled) fieldstrength of all antennas
      // (don't pass antennaSelection to get full number of columns in the Matrix)
      Matrix<Double> fieldstrength = GetTimeSeries(dr, Vector<Bool>(), Polarization);
      // create upsampling factor by upsampling exponent
      unsigned int upsampled = pow(2,upsampling_exp);

      // get length of trace
      unsigned int tracelength = fieldstrength.column(0).size();
      // allocate memory for original and upsampled traces
      double* originalTrace = new double[tracelength];
      double* upsampledTrace = new double[tracelength * upsampled];

      // resize Matrix for upsampled traces if the last upsampling exponent is different from the new one
      if (lastUpsamplingExponent != upsampling_exp)
      {
        upFieldStrength.resize(tracelength * upsampled, antennaSelection.nelements(), false);	// no need to copy old values
        // set all traces initially to zero (not neccessary but makes the detection of errors easier)
        upFieldStrength.set(0);	
      } 

      // do upsampling for each antenna in the todo-list
      std::cout << "Upsampling the calibrated data by a factor of " << upsampled << " ...\nAntenna:" << flush;
      for (unsigned int i = 0; i < antennaSelection.nelements(); i++) {
        if (upsamplingToDo[i]){
          std::cout << " " << i+1 << flush;
 	  // copy the trace into the array
	  for (unsigned int j = 0; j < tracelength; j++) {
            originalTrace[j] = fieldstrength.column(i)(j);
          }
	  
	  // do upsampling by factor #upsampled (--> NoZeros = upsampled -1)

          // calcutlate Offset:
          double before = originalTrace[0];

          ZeroPaddingFFT(tracelength, originalTrace, upsampled-1, upsampledTrace);
 
	  double offset = before - originalTrace[0];
	
  	  // copy upsampled trace into Matrix with antenna traces and subtract offset
          // remark: as there is no offset in the original data, this step should be avoided
          // as soon as upsampling without pedestal correction is available
	  for (unsigned int j = 0; j < tracelength*upsampled; j++) {
	    upFieldStrength.column(i)(j) = upsampledTrace[j] + offset;
	  }
	  
          /*
	    Remark: tried to fasten data transfer from the array to the Matrix
	    but did not work because arrays are of typ float but not double; as
	    double is supported now, it can be tried again.
	  */

          // set flag, that data for this antenna are upsampled
          upsampledAntennas[i] = true;
        } 
      } 
      std::cout << " ... done" << endl;

      // set last upsampling exponent to current value
      lastUpsamplingExponent = upsampling_exp;

      // delete arrays
      delete[] originalTrace;
      delete[] upsampledTrace;

      // Return upsampled traces 
      return upFieldStrength.copy();
    } catch (AipsError x) 
      {
        std::cerr << "CompletePipeline:getUpsampledFieldstrength: " << x.getMesg() << std::endl;
      }; 

    // return dummy to avoid warning (this command is reached only in case of an error).
    return Matrix<Double>();
  }

 
  Matrix<Double> CompletePipeline::getUpsampledFX (DataReader *dr,
						   const int& upsampling_exp,
						   Vector<Bool> antennaSelection,
						   const bool& offsetSubstraction)
  {
    try 
    {
      // Get the (not yet upsampled) raw data of all antennas
      Matrix<Double> rawData = dr->fx(); 

      // check if upsampling shoud be done at all (if not, return not upsampled data)
      if (upsampling_exp < 1) return rawData.copy(); 

      // make antennaSelection unique, as casacore-Vectors are allways passed by reference
      antennaSelection.unique();

      // Get the antenna selection from the DataReader if no selction was chosen 	
      if (antennaSelection.nelements() == 0) {
	antennaSelection = GetAntennaMask(dr);
      }

      // consistency check: number of elements in antennaSelction and rawData must be equal
      if ( rawData.ncolumn() != antennaSelection.nelements() )
      {
        std::cerr << "CompletePipeline:getUpsampledFX: Number of elements in \"antennaSelection\" is inconsistent.\n" 
		<< std::endl;
        return rawData.copy();
      }

      // create upsampling factor by upsampling exponent
      unsigned int upsampled = pow(2,upsampling_exp);

      // get length of trace
      unsigned int tracelength = rawData.column(0).size();

      // allocate memory for original and upsampled traces
      double* originalTrace = new double[tracelength];
      double* upsampledTrace = new double[tracelength * upsampled];

      // Create Matrix for usampled values
      Matrix<Double> upData(tracelength * upsampled, antennaSelection.nelements(), 0);

      // do upsampling for each antenna in the antennaSelection
      std::cout << "Upsampling the raw data by a factor of " << upsampled << " ...\nAntenna:" << flush;
      for (unsigned int i = 0; i < antennaSelection.nelements(); i++) if (antennaSelection(i))
      {
        std::cout << " " << i+1 << flush;
	// copy the trace into the array
	for (unsigned int j = 0; j < tracelength; j++) 
        {
          originalTrace[j] = rawData.column(i)(j);
        }

	// do upsampling by factor #upsampled (--> NoZeros = upsampled -1)

        // calcutlate Offset:
        double before = originalTrace[0];

        ZeroPaddingFFT(tracelength, originalTrace, upsampled-1, upsampledTrace);

	double offset = before - originalTrace[0];
	
	// copy upsampled trace into Matrix with antenna traces and subtract offset
        // if no offset correction is wanted
        // remember: ZeroPaddingFFT removes the offset automatically
        if (offsetSubstraction)
	  for (unsigned int j = 0; j < tracelength*upsampled; j++)
            upData.column(i)(j) = upsampledTrace[j];
        else
	  for (unsigned int j = 0; j < tracelength*upsampled; j++)
            upData.column(i)(j) = upsampledTrace[j] + offset;
      } 
      std::cout << " ... done" << endl;

      // delete arrays
      delete[] originalTrace;
      delete[] upsampledTrace;

      // Return upsampled traces 
      return upData.copy();
    } catch (AipsError x) 
      {
        std::cerr << "CompletePipeline:getUpsampledFX: " << x.getMesg() << std::endl;
      }; 

    // return dummy to avoid warning (this command is reached only in case of an error).
    return Matrix<Double>();
  }



  Vector<Double> CompletePipeline::getUpsampledTimeAxis (DataReader *dr,
							 const int& upsampling_exp)
  {
    try 
    {
      // Check if upsampling shoud be done at all (if not, return not upsampled time axis)
      if (upsampling_exp < 1) return static_cast< Vector<Double> >(dr->timeValues());

      // Check if upampling should be done be the same factor as last time
      // in this case, return the old values
      if (lastTimeUpsamplingExponent == upsampling_exp)
        return upTimeValues.copy();

      // Get the not interpolated time axis
      Vector<Double> timeaxis = static_cast< Vector<Double> >(dr->timeValues()); 

      // create upsampling factor by upsampling exponent
      unsigned int upsampled = pow(2,upsampling_exp);

      // Get time between samples
      double sampleTime = 1/(dr->sampleFrequency() * upsampled);

      // get the length of the x-axis 
      long int time_length = timeaxis.size();

      // Resize the time Vector for the new upsampled xaxis
      upTimeValues.resize(time_length*upsampled, false);

      // copy old values to the right place and fill space inbetween 
      for (long int i = time_length-1; i >= 0; i--)
      {
        // move existing time value to the right place
        upTimeValues(i*upsampled) = timeaxis(i);
        // create new values
        for (unsigned int j = 1; j < upsampled; j++) 
          upTimeValues(i*upsampled+j) = upTimeValues(i*upsampled) + j*sampleTime;
      }

      // set last upsampling exponent to current value
      lastTimeUpsamplingExponent = upsampling_exp;

      // Return upsampled xaxis
      return upTimeValues.copy();
    } catch (AipsError x) 
      {
        std::cerr << "CompletePipeline:getUpsampledTimeAxis: " << x.getMesg() << std::endl;
      }; 

    // return dummy to avoid warning (this command is reached only in case of an error).
    return Vector<Double>();
  }


  Slice CompletePipeline::calculatePlotRange (const Vector<Double>& xaxis) const
  {
    try
    {
      // check if plotStart is <= plotStop
      if (plotStop_p < plotStart_p)
      {
        std::cerr << "CompletePipeline:calculatePlotRange: Error: plotStop_p is greater than plotStart_p!" << std::endl;
        return Slice(0,0);
      }
      int startsample = ntrue(xaxis<plotStart_p);            //number of elements smaller then starting value of plot range
      int stopsample = ntrue(xaxis<plotStop_p);              //number of elements smaller then end of plot range
      Slice plotRange(startsample,(stopsample-startsample)); // create Slice with plotRange
      return plotRange;
    } catch (AipsError x) 
    {
        std::cerr << "CompletePipeline:calculatePlotRange: " << x.getMesg() << std::endl;
    }; 
 
    // return dummy to avoid warning (this command is reached only in case of an error).
    return Slice();
  }


  Slice CompletePipeline::calculateNoiseRange (const Vector<Double>& xaxis) const
  {
    try
    {
      // check if plotStart is <= plotStop
      if (plotStop_p < plotStart_p)
      {
        std::cerr << "CompletePipeline:calculateNoiseRange: Error: plotStop_p is greater than plotStart_p!" << std::endl;
        return Slice(0,0);
      }
      //number of elements smaller then starting value of plot range
      int startsample = ntrue(xaxis<plotStart_p);
      //number of elements smaller then end of plot range
      int stopsample = ntrue(xaxis<plotStop_p);  
      // create Slice with noiseRange
      Slice noiseRange((startsample+startsample-stopsample),(stopsample-startsample));

      return noiseRange;
    } catch (AipsError x) 
    {
        std::cerr << "CompletePipeline:calculateNoiseRange: " << x.getMesg() << std::endl;
    }; 

    // return dummy to avoid warning (this command is reached only in case of an error).
    return Slice();
  }


  Slice CompletePipeline::calculateCCRange (const Vector<Double>& xaxis, const double& ccBeamcenter) const
  {
    try
    {
      // check if ccBeam has converged
      if (ccBeamcenter == 0.0)
      {
        std::cerr << "CompletePipeline:calculateCCRange: Error: CC-beam did not converged !" << std::endl;
        return Slice(0,0);
      }
      //number of elements smaller then CC-center minus window size
      int startsample = ntrue(xaxis<(ccBeamcenter-ccWindowWidth_p)); 
      //number of elements smaller then end of plot range
      int stopsample = ntrue(xaxis<(ccBeamcenter+ccWindowWidth_p));
      // create Slice with plotRangeNoise
      Slice rangeCC(startsample,(stopsample-startsample));
      return rangeCC;
    } catch (AipsError x) 
    {
        std::cerr << "CompletePipeline:calculateCCRange: " << x.getMesg() << std::endl;
    }; 

    // return dummy to avoid warning (this command is reached only in case of an error).
    return Slice();
  }


  void CompletePipeline::plotCCbeam(const string& filename,
				    DataReader *dr,
				    Vector<Double> fittedCCbeam,
				    Vector<Bool> antennaSelection,
				    const int& filterStrength,
				    const double& remoteStart,
				    const double& remoteStop)
  {
    try 
    {
      SimplePlot plotter;    			// define plotter
      Vector<Double> xaxis, ccbeam, pbeam;	// values for plot
      double xmax,xmin,ymin,ymax;		// Plotrange
      uInt gtdate;
      dr->header().get("Date",gtdate);
      stringstream gtlabel;
      gtlabel << gtdate;
      string  plotfilename;

      // add the ".ps" to the filename
      plotfilename = filename + ".ps";

      // alternative filename gtdate".ps"
      //plotfilename = gtlabel.str() + ".ps";

      // make antennaSelection unique, as casacore-Vectors are allways passed by reference
      antennaSelection.unique();

      std::cout <<"Plotting the CC-beam and the Power-beam to file: " << plotfilename << std::endl;

      // Get the time axis
      xaxis = (Vector<Double>)(dr->timeValues()); 

      // Define plotrange
      Slice plotRange = calculatePlotRange(xaxis); 

      // Get the CC-beam and the power-beam
      ccbeam = GetCCBeam(dr, antennaSelection, Polarization).copy();
      pbeam  = GetPBeam(dr, antennaSelection, Polarization).copy();

      // smooth the data
      if (filterStrength > 0)
      {
        StatisticsFilter<Double> mf(filterStrength,FilterType::MEAN);
        ccbeam = mf.filter(ccbeam);
        pbeam = mf.filter(pbeam);
      }

      // calcutlate and substract offset (= noise, calculated as mean in remote region)
      // if no remote range was given, don't substract anything
      if (remoteStop != 0)
      {
        Slice remoteRegion(remoteStart,(remoteStop-remoteStart));
        double ccBeamOffset = mean(ccbeam(remoteRegion));
        double pBeamOffset  = mean(pbeam(remoteRegion));
        ccbeam -= ccBeamOffset;
        pbeam  -= pBeamOffset;
      }

      // conversion to micro
      xaxis *= 1e6;
      ccbeam *= 1e6;
      pbeam *= 1e6;
    
      // define Plotrange
      xmin = min(xaxis(plotRange));
      xmax = max(xaxis(plotRange));
      ymin = min(min(ccbeam(plotRange)), min(pbeam(plotRange)))*1.05;
      ymax = max(max(ccbeam(plotRange)), max(pbeam(plotRange)))*1.05;

      // Initialize the plots giving xmin, xmax, ymin and ymax
      plotter.InitPlot(plotfilename, xmin, xmax, ymin, ymax);

      // Create the Plot
      string label;
      label = "GT " + gtlabel.str() + " - CC-Beam and Power";
      plotter.AddLabels("Time t [#gmsec]", "CC-Beam [#gmV/m/MHz]",label);
    
      // Add CC-beam
      plotter.PlotLine(xaxis(plotRange),ccbeam(plotRange),9,1);
      // Add Power-beam
      plotter.PlotLine(xaxis(plotRange),pbeam(plotRange),8,1);

      // Add fitted beam, if supplied (must have correct length)
      if (fittedCCbeam.size() == xaxis.size())
      {
        fittedCCbeam.unique();
        fittedCCbeam *= 1e6;
        plotter.PlotLine(xaxis(plotRange),fittedCCbeam(plotRange),4,1);
      }

      // Add filename to list of created plots
      plotlist.push_back(plotfilename);

    } catch (AipsError x) 
      {
        std::cerr << "CompletePipeline:plotCCBeam: " << x.getMesg() << std::endl;
      }; 
  }


  void CompletePipeline::plotXbeam(const string& filename,
				   DataReader *dr,
				   Vector<Double> fittedXbeam,
				   Vector<Bool> antennaSelection,
				   const int& filterStrength,
				   const double& remoteStart,
				   const double& remoteStop)
  {
    try 
    {
      SimplePlot plotter;    			// define plotter
      Vector<Double> xaxis, xbeam, pbeam;	// values for plot
      double xmax,xmin,ymin,ymax;		// Plotrange
 
      uInt gtdate;
      dr->header().get("Date",gtdate);
      stringstream gtlabel;
      gtlabel << gtdate;
      string  plotfilename;

      // add the ".ps" to the filename
      plotfilename = filename + ".ps";

      // alternative filename gtdate".ps"
      //plotfilename = gtlabel.str() + ".ps";

      // make antennaSelection unique, as casacore-Vectors are allways passed by reference
      antennaSelection.unique();

      std::cout <<"Plotting the X-beam and the Power-beam to file: " << plotfilename << std::endl;

      // Get the time axis
      xaxis = (Vector<Double>)(dr->timeValues()); 

      // Define plotrange
      Slice plotRange = calculatePlotRange(xaxis); 

      // Get the X-beam and the power-beam
      xbeam = GetXBeam(dr, antennaSelection, Polarization).copy();
      pbeam = GetPBeam(dr, antennaSelection, Polarization).copy();

      // smooth the data
      if (filterStrength > 0)
      {
        StatisticsFilter<Double> mf(filterStrength,FilterType::MEAN);
        xbeam = mf.filter(xbeam);
        pbeam = mf.filter(pbeam);
      }

      // calcutlate and substract offset (= noise, calculated as mean in remote region)
      // if no remote range was given, don't substract anything
      if (remoteStop != 0)
      {
        Slice remoteRegion(remoteStart,(remoteStop-remoteStart));
        double xBeamOffset = mean(xbeam(remoteRegion));
        double pBeamOffset = mean(pbeam(remoteRegion));
        xbeam -= xBeamOffset;
        pbeam -= pBeamOffset;
      }

      // conversion to micro
      xaxis *= 1e6;
      xbeam *= 1e6;
      pbeam *= 1e6;
    
      // define Plotrange
      xmin = min(xaxis(plotRange));
      xmax = max(xaxis(plotRange));
      ymin = min(min(xbeam(plotRange)), min(pbeam(plotRange)))*1.05;
      ymax = max(max(xbeam(plotRange)), max(pbeam(plotRange)))*1.05;

      // Initialize the plots giving xmin, xmax, ymin and ymax
      plotter.InitPlot(plotfilename, xmin, xmax, ymin, ymax);

      // Create the Plot
      string label;
      label = "GT " + gtlabel.str() + " - X-Beam and Power";
      plotter.AddLabels("Time t [#gmsec]", "X-Beam [#gmV/m/MHz]",label);
    
      // Add X-beam
      plotter.PlotLine(xaxis(plotRange),xbeam(plotRange),9,1);
      // Add Power-beam
      plotter.PlotLine(xaxis(plotRange),pbeam(plotRange),8,1);

      // Add fitted beam, if supplied (must have correct length)
      if (fittedXbeam.size() == xaxis.size())
      {
        fittedXbeam.unique();
        fittedXbeam *= 1e6;
        plotter.PlotLine(xaxis(plotRange),fittedXbeam(plotRange),4,1);
      }

      // Add filename to list of created plots
      plotlist.push_back(plotfilename);

    } catch (AipsError x) 
      {
        std::cerr << "CompletePipeline:plotXBeam: " << x.getMesg() << std::endl;
      }; 
  }


  // Plot the timeshifted (= after beamforming) filedstrength of all antennas
  void CompletePipeline::plotAllAntennas(const string& filename,
					 DataReader *dr,
					 Vector<Bool> antennaSelection,
					 const bool& seperated,
					 const int& upsampling_exp,
					 const bool& rawData,
					 const bool& plotEnvelope)
  {
    try 
    {
      SimplePlot plotter;    			// define plotter
      Vector<Double> xaxis;			// xaxis
      double xmax,xmin,ymin=0,ymax=0;		// Plotrange
      Matrix<Double> yValues;			// y-values
      Matrix<Double> upYvalues;			// upsampled y-values
      int color = 3;				// starting color

      // make antennaSelection unique, as casacore-Vectors are allways passed by reference
      antennaSelection.unique();

      // Get the antenna selection from the DataReader if no selction was chosen
      if (antennaSelection.nelements() == 0) {
	antennaSelection = GetAntennaMask(dr);
      }

      // adopt the antennaSelection to the chosen polarization
      deselectectPolarization(dr,antennaSelection);

      // Get the (not upsampled) time axis
      xaxis = static_cast< Vector<Double> >(dr->timeValues());

      // Get the yValues of all antennas (raw data or fieldstrength)
      if (rawData)
      {
        // get not upsampled data
        yValues = dr->fx().copy();
        // Upsampled yValues (ADC offset will not be substracted)
        upYvalues = getUpsampledFX(dr,upsampling_exp, antennaSelection, false);
      }
      else
      {
        yValues = GetTimeSeries(dr).copy();  
        // don't use the antennaSelection to get the full number of columns in the Matrix

        // Upsampled yValues
        upYvalues = getUpsampledFieldstrength(dr,upsampling_exp, antennaSelection);
      }
    
      // Upsampled x-axis
      Vector<Double> upxaxis = getUpsampledTimeAxis(dr,upsampling_exp);

      // check length of time axis and yValues traces for consistency
      if (upxaxis.size() != upYvalues.column(0).size())
        std::cerr << "CompletePipeline:plotAllAntennas: WARNING: Length of time axis differs from length of the antenna traces!\n"
           << std::endl;


      // Define plotrange for not upsampled and upsampled data
      Slice plotRange = calculatePlotRange(xaxis);
      Slice upplotRange = calculatePlotRange(upxaxis);

      // conversion to micro (no conversion for height of raw data)
      xaxis *= 1e6;
      if (!rawData) yValues *= 1e6;
      upxaxis *= 1e6;
      if (!rawData) upYvalues *= 1e6;  

      // define Plotrange
      xmin = min(xaxis(plotRange));
      xmax = max(xaxis(plotRange));

      // find the minimal and maximal y values for the plot
      // do it with the upsampled data only as they are as least as heigh as the original ones
      for (unsigned int i = 0; i < antennaSelection.nelements(); i++)
        if (antennaSelection(i))		// consider only selected antennas
        {
          if ( ymin > min(upYvalues.column(i)(upplotRange)) ) {
	    ymin = min(upYvalues.column(i)(upplotRange));
	  }
          if ( ymax < max(upYvalues.column(i)(upplotRange)) ) {
	    ymax = max(upYvalues.column(i)(upplotRange));
	  }
        }

      // multiply ymin and ymax by 105% to have some space at the bottom and the top of the plot
      ymin *= 1.05;
      ymax *= 1.05;

      // set up label for plots and filename
      string plotfilename;
      string label;
      uInt gtdate;
      stringstream gtlabel, antennanumber;
      // Get the AntennaIDs for labeling
      dr->header().get("Date",gtdate);
      gtlabel << gtdate;


      // Create empty vector for not existing error bars 
      Vector<Double> empty;

      // Make the plots (either all antennas together or seperated)
      if (seperated)
      {
        stringstream antennaid;
        Vector<Int> AntennaIDs;
        dr->header().get("AntennaIDs",AntennaIDs);

        // Create the plots for each individual antenna looping through antennas
        if (rawData)
            std::cout <<"Plotting the raw data FX\nAntenna ... ";
        else
	    std::cout <<"Plotting the fieldstrength\n Antenna ..." ;


        for (unsigned int i = 0; i < antennaSelection.nelements(); i++){
          // consider only selected antennas
          if (antennaSelection(i)){
            // create filename and label
	    antennanumber.str("");
	    antennanumber.clear();
            antennanumber << (i+1);
            antennaid << AntennaIDs(i);

            //set the plotfilename to filename + "-" + antennanumber.str() + ".ps";
            if ( (i+1) < 10 ){
               plotfilename = filename + "-0" + antennanumber.str() + ".ps";
	    }else{
               plotfilename = filename + "-" + antennanumber.str() + ".ps";
	    }
            //set label "GT - Ant.Nr"
            label = "GT " + gtlabel.str() + " - Antenna " + antennanumber.str();

            //alternative plotfilename
            //plotfilename = gtlabel.str() + "-" + antennanumber.str() + ".ps";

            // Initialize the plot giving xmin, xmax, ymin and ymax
            plotter.InitPlot(plotfilename, xmin, xmax, ymin, ymax);

            // Add labels
            if (rawData)
              plotter.AddLabels("Time t [#gmsec]", "Counts",label);
            else
              plotter.AddLabels("Time t [#gmsec]", "Fieldstrength #ge#d0#u [#gmV/m/MHz]",label);
  
            // Plot (upsampled) trace
            plotter.PlotLine(upxaxis(upplotRange),upYvalues.column(i)(upplotRange),color,1);
            // Plot envelope
            if (plotEnvelope)
              plotter.PlotLine(upxaxis(upplotRange),envelope(upYvalues.column(i)(upplotRange)),color,1);
            // Plot original data points (if upsampling was done).
            if (upsampling_exp > 0)
              plotter.PlotSymbols(xaxis(plotRange),yValues.column(i)(plotRange),empty, empty, color, 2, 5);

            // Add filename to list of created plots
            plotlist.push_back(plotfilename);
  	    std::cout << " " << (i+1);
            color++;					// another color for the next antenna
            if (color >= 13) color = 3;			// there are only 16 colors available, 
							// use only ten as there are 3x10 antenna
          }
	}
        std::cout << std::endl;
      } else {  // if (seperated) => else
        // add the ".ps" to the filename
        plotfilename = filename + ".ps";

        //alternative plotfilename
        //plotfilename = gtlabel.str() + ".ps";

        if (rawData)
    	  std::cout <<"Plotting the raw data FX of all antennas to file: "
	            << plotfilename << std::endl;
        else
    	  std::cout <<"Plotting the fieldstrength of all antennas to file: "
	            << plotfilename << std::endl;

        // Initialize the plot giving xmin, xmax, ymin and ymax
        plotter.InitPlot(plotfilename, xmin, xmax, ymin, ymax);
        // Add labels 
        antennanumber << (antennaSelection.nelements()-1);
        label = "GT " + gtlabel.str() + " - " + antennanumber.str() + " Antennas";

        if (rawData)
          plotter.AddLabels("Time t [#gmsec]", "Counts",label);
        else
          plotter.AddLabels("Time t [#gmsec]", "Fieldstrength #ge#d0#u [#gmV/m/MHz]",label);

        // Create the plots looping through antennas
        for (unsigned int i = 0; i < antennaSelection.nelements(); i++)
        if (antennaSelection(i))			// consider only selected antennas
        {
          // Plot (upsampled) trace
          plotter.PlotLine(upxaxis(upplotRange),upYvalues.column(i)(upplotRange),color,1);

          color++;					// another color for the next antenna
          if (color >= 13) color = 3;			// there are only 16 colors available, 
							// use only ten as there are 3x10 antennas
        }

        // Add filename to list of created plots
        plotlist.push_back(plotfilename);
      } // else
    } catch (AipsError x) 
      {
        std::cerr << "CompletePipeline:plotAllAntennas: " << x.getMesg() << std::endl;
      }; 
  }

  
  void CompletePipeline::calculateMaxima(DataReader *dr,
					 Vector<Bool> antennaSelection,
					 const int& upsampling_exp,
					 const bool& rawData)
  {
    try 
    {
      Vector<Double> timeValues;		// time values
      Vector<Double> timeRange;			// time values
      Matrix<Double> yValues;			// y-values
      Vector<Double> trace;			// trace currently processed
      vector<double> maxima;			// Stores the calculated maxima
      vector<double> maxima_time;		// Stores the calculated time of the maxima
      vector<double> fwhm;			// width of the pulses
      vector<double> start_time;		// Stores the start time of the pulse (begin of FWHM)

      if (rawData)
        std::cout << "\nLooking for maxima in the envelope of the raw data FX: \n";
      else
        std::cout << "\nLooking for maxima in the envelope of the calibrated fieldstrength: \n";
  

      // make antennaSelection unique, as casacore-Vectors are allways passed by reference
      antennaSelection.unique();

      // Get the antenna selection from the DataReader if no selction was chosen 	
      if (antennaSelection.nelements() == 0) {
	antennaSelection = GetAntennaMask(dr);
      }

      // adopt the antennaSelection to the chosen polarization
      deselectectPolarization(dr,antennaSelection);
 
      // Get the (upsampled) time axis
      timeValues = getUpsampledTimeAxis(dr,upsampling_exp);

      // Get the yValues of all selected antennas (raw data or fieldstrength)
      if (rawData)
        yValues = getUpsampledFX(dr,upsampling_exp, antennaSelection, true);  // true means: offset will be substracted
      else
        yValues = getUpsampledFieldstrength(dr,upsampling_exp, antennaSelection);

      // check length of time axis and yValues traces for consistency
      if (timeValues.size() != yValues.column(0).size())
        std::cerr << "CompletePipeline:calculateMaxima: WARNING: Length of time axis differs from length of the antenna traces!\n"
           << std::endl;


      // Define the time range considered (the same as the plot range)
      Slice range = calculatePlotRange(timeValues);

      // cut time values
      timeRange = timeValues(range);

      // print header line of output
      std::cout <<  "Ant  pulse height  time of half height  time of max   FWHM\n"
                <<  "[#]   [uV/m/MHZ]          [us]             [us]       [ns]\n"
                <<  "-----------------------------------------------------------\n";

      // find the maximal y values for all selected antennas
      for (unsigned int i = 0; i < antennaSelection.nelements(); i++) if (antennaSelection(i))
      {
        // Start with height 0 and search for heigher and lower values
        double maximum = 0;
        int maxtimevalue = 0;

        // get current envelope of trace
        trace = envelope(yValues.column(i)(range));

        // loop through the values and search for the heighest one
        for(unsigned int j = 0; j < timeRange.nelements(); j++)
	{
          if ( maximum < trace(j)) 
          {
            maxtimevalue = j;
            maximum = trace(j);
          } 
	}

        // calculate FWHM
       double pulsestart = 0;
       double pulsestop = 0;
 
       // find begin of pulse (half height)
        for(unsigned int j = maxtimevalue; j >= 0; j--)
	{
          // find crossing of half height (between j and j+1)
          if ( trace(j) <= maximum/2.)
          {
            // interpolate linear
            // calculate differences to half height
            double totaldiff = trace(j+1) - trace(j);
            double diffdown = maximum/2. - trace(j);
            double timesample = timeRange(j+1) - timeRange(j);
            pulsestart = timeRange(j) + timesample * (diffdown/totaldiff);
            break; // exit loop
          } 
	}

       // find end of pulse (half height)
        for(unsigned int j = maxtimevalue; j < timeRange.nelements(); j++)
	{
          // find crossing of half height (between j-1 and j)
          if ( trace(j) <= maximum/2.)
          {
            // interpolate linear
            // calculate differences to half height
            double totaldiff = trace(j-1) - trace(j);
            double diffdown = maximum/2. - trace(j);
            double timesample = timeRange(j) - timeRange(j-1);
            pulsestop = timeRange(j) - timesample * (diffdown/totaldiff);
            break; // exit loop
          } 
	}

        // store the calculated values for later calculation of the mean
	// multiply by 1e6 for conversion to micro
        maxima.push_back(maximum*1e6);
        maxima_time.push_back(timeRange(maxtimevalue)*1e6);
        fwhm.push_back( (pulsestop-pulsestart)*1e9);
        start_time.push_back(pulsestart*1e6);


        // print the calculated values
        std::cout << std::setw(2) << i+1 << "     " 
                  << std::setw(8) << maximum*1e6 << "          " 
                  << std::setw(8) << pulsestart*1e6 << "       "
                  << std::setw(8) << timeRange(maxtimevalue)*1e6 << "   "
                  << std::setw(8) << (pulsestop-pulsestart)*1e9 << std::endl;
      } // for i

      // calculate the averages and the range if there is more than one value
      if (maxima.size() > 1)
      {
        std::cout << "Summary for the maxima:\n" 
                  << "Amplitude range [micro]:   " << min(static_cast< Vector<Double> >(maxima)) 
                  << " to " << max(static_cast< Vector<Double> >(maxima)) << "\n"
                  << "Amplitude average [micro]: " << mean(static_cast< Vector<Double> >(maxima)) << "\n"
                  << "Time range [micro s]:      " << min(static_cast< Vector<Double> >(maxima_time))
                  << " to " << max(static_cast< Vector<Double> >(maxima_time)) << "\n"
                  << "Time average (max) [us]:   " << mean(static_cast< Vector<Double> >(maxima_time)) << "\n"
                  << "Time average (start) [us]: " << mean(static_cast< Vector<Double> >(start_time)) << "\n"
                  << "FWHM average [ns]:         " << mean(static_cast< Vector<Double> >(fwhm)) << std::endl;
      }

    } catch (AipsError x) 
      {
        std::cerr << "CompletePipeline:caclulateMaxima: " << x.getMesg() << std::endl;
      }; 
  }



  void CompletePipeline::listCalcMaxima(DataReader *dr,
					 Vector<Bool> antennaSelection,
					 const int& upsampling_exp,
					 const double& cc_center)
  {
    try 
    {
      Vector<Double> timeValues;		// time values
      Vector<Double> timeRange;			// time values
      Vector<Double> timeRangeNoise;		// time values
      Matrix<Double> yValues;			// y-values
      Vector<Double> trace;			// trace currently processed
      Vector<Double> traceNoise;		// trace currently processed
      vector<double> extrema;			// Stores the calculated extrema
      vector<double> extrema_time;		// Stores the calculated time of the extrema


      // make antennaSelection unique, as casacore-Vectors are allways passed by reference
      antennaSelection.unique();

      // Get the antenna selection from the DataReader if no selction was chosen 	
      if (antennaSelection.nelements() == 0) {
	antennaSelection = GetAntennaMask(dr);
      }

      // adopt the antennaSelection to the chosen polarization
      deselectectPolarization(dr,antennaSelection);
 
      // Get the (upsampled) time axis
      timeValues = getUpsampledTimeAxis(dr,upsampling_exp);

      // Get the yValues of all selected antennas (raw data or fieldstrength)
      yValues = getUpsampledFieldstrength(dr,upsampling_exp, antennaSelection);


      // check length of time axis and yValues traces for consistency
      if (timeValues.size() != yValues.column(0).size())
      {
        std::cerr << "CompletePipeline:listCalcMaxima: Error: Length of time axis differs from length of the antenna traces!\n"
                  << "CompletePipeline:listCalcMaxima: exiting function!" << std::endl;
        return;
      }

      // check if cc_center is at a valid position (time must be within the time-axis)
      if ( (cc_center < timeValues(0)) || (cc_center > timeValues(timeValues.endPosition())) )
      {
        std::cerr << "CompletePipeline:listCalcMaxima: Error: Center of CC-Beam is at an invalid time: " << cc_center << " s\n" 
                  << "CompletePipeline:listCalcMaxima: exiting function!" << std::endl;
        return;
      }


      // Define the time range considered for peak search 
      Slice range = calculateCCRange(timeValues,cc_center);
      // Define the time range considered (the same length as the plot range, 
      // but before actual plot range)
      Slice rangeNoise = calculateNoiseRange(timeValues);


      // Start with height 0 and search for heigher and lower values
      double maximum = 0;
      double minimum = 0;
      double extremum =0;
      double noise=0;
      int extrematimevalue = 0;


      // cut time values
      timeRange = timeValues(range);
      // cut time values form before plotrange
      timeRangeNoise = timeValues(rangeNoise);

      // find the maximal y  values  for all selected antennas
      std::cout <<  "An field    noise    time\n";
      for (unsigned int i = 0; i < antennaSelection.nelements(); i++) 

      if (antennaSelection(i)){
        // Start with height 0 and search for heigher and lower values
        maximum = 0;
	minimum = 0;
        extremum =0;
        noise =0;
        extrematimevalue = 0;

        // get current trace
        trace = yValues.column(i)(range);

        // loop through the values and search for the heighest and lowest one
        for(unsigned int j = 0; j < timeRange.nelements(); j++)
	{
          if ( maximum < trace(j)) 
            maximum = trace(j);

          if ( minimum > trace(j)) 
            minimum = trace(j);

          if ( extremum < maximum ) {
             extremum = maximum;
             extrematimevalue = j;
	  }
          if (extremum < abs(minimum)){
             extremum = abs(minimum);
             extrematimevalue = j;
	  }
	}  

        extrema.push_back(extremum*1e6);
        extrema_time.push_back(timeRange(extrematimevalue)*1e6);

        // get current trace
        traceNoise = yValues.column(i)(rangeNoise);
        // loop through the values and search for the heighest and lowest one
        for(unsigned int j = 0; j < timeRangeNoise.nelements(); j++)
	{
	  noise += abs(traceNoise(j));
	}
	noise /=timeRangeNoise.nelements();

        // print the calculated values
        std::cout << std::setw(2) << i+1 << " " <<std::setw(7) <<extremum*1e6 << " ";
	std::cout << std::setw(8) << noise*1e+6  << " " << std::setw(8)<< timeRange(extrematimevalue)*1e6 ;
	std::cout << std::endl;
      }else{
        // antenna has bad data
        // get current trace
        traceNoise = yValues.column(i)(rangeNoise);
        // loop through the values and search for the heighest and lowest one
        for(unsigned int j = 0; j < timeRangeNoise.nelements(); j++)
	{
	  noise += abs(traceNoise(j));
	}
	noise /=timeRangeNoise.nelements();
        std::cout << std::setw(2) << i+1 << " 0.0      "<< std::setw(8) << noise*1e+6  << " 0.0" <<std::endl;
      }

    } catch (AipsError x) 
      {
        std::cerr << "CompletePipeline:listCalcMaxima: " << x.getMesg() << std::endl;
      }; 
  }



} // Namespace CR -- end
