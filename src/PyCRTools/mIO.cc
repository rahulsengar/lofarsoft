/**************************************************************************
 *  IO module for the CR Pipeline Python bindings.                        *
 *                                                                        *
 *  Copyright (c) 2010                                                    *
 *                                                                        *
 *  Martin van den Akker <martinva@astro.ru.nl>                           *
 *  Heino Falcke <h.falcke@astro.ru.nl>                                   *
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


// ========================================================================
//
//  Wrapper Preprocessor Definitions
//
// ========================================================================

//-----------------------------------------------------------------------
//Some definitions needed for the wrapper preprosessor:
//-----------------------------------------------------------------------
//$FILENAME: HFILE=mIO.def.h
//-----------------------------------------------------------------------

// ========================================================================
//
//  Header files
//
// ========================================================================

#include "core.h"
#include "casa.h"
#include "mVector.h"
#include "mIO.h"
#include "mMath.h"

#ifdef PYCRTOOLS_WITH_AERA
#include "aera/Datareader.h"
#include "aera/Data/Header.h"
#include "aera/Data/Event.h"
#include "aera/Data/LocalStation.h"
#endif /* PYCRTOOLS_WITH_AERA */

// ========================================================================
//
//  Implementation
//
// ========================================================================

using namespace std;

#undef HFPP_FILETYPE
//--------------------
#define HFPP_FILETYPE CC
//--------------------

// ========================================================================
//$SECTION: IO functions
// ========================================================================

/*!
\brief Determine the filetype based on the extension of the filename
 */
HString hgetFiletype(HString filename){
  HString ext=hgetFileExtension(filename);
  HString typ="";
  if (ext=="event") typ="LOPESEvent";
  else if (ext=="h5") typ="LOFAR_TBB";
  return typ;
}

/*!
\brief Get the extension of the filename, i.e. the text after the last dot.
 */
HString hgetFileExtension(HString filename){
  HInteger size=filename.size();
  HInteger pos=filename.rfind('.',size);
  pos++;
  if (pos>=size) {return "";}
  else {return filename.substr(pos,size-pos);};
}


//$DOCSTRING: Print a brief summary of the file contents and current settings.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hFileSummary
//------------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF  (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (CRDataReader)(dr)()("DataReader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING
*/

void HFPP_FUNC_NAME(CRDataReader & dr) {
  dr.summary();
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Function to open a file based on a filename and returning a datareader object.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hFileOpen
//------------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF  (CRDataReader)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_0 (HString)(Filename)()("Filename of file to open including full directory name")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING
*/

CRDataReader & HFPP_FUNC_NAME(const HString Filename) {

  bool opened = false;

  //Create the a pointer to the DataReader object and store the pointer
  CR::DataReader* drp = NULL;

  HString Filetype = hgetFiletype(Filename);

  if (Filetype=="LOPESEvent") {
    CR::LopesEventIn* lep = new CR::LopesEventIn;
    opened=lep->attachFile(Filename);
    if (opened == false) {
      throw PyCR::IOError("Unable to open file.");
    } else {
      drp=lep;
    }
    cout << "Opening LOPES File="<<Filename<<endl; drp->summary();
  } else if (Filetype=="LOFAR_TBB") {
    drp = new CR::LOFAR_TBB(Filename,1024);
    if (drp == NULL) {
      opened = false;
      throw PyCR::IOError("Unable to open file.");
    } else {
      opened = true;
    }
    cout << "Opening LOFAR File="<<Filename<<endl; drp->summary();
  } else {
    ERROR(BOOST_PP_STRINGIZE(HFPP_FUNC_NAME) << ": Unknown Filetype = " << Filetype  << ", Filename=" << Filename);
    opened=false;
  }
  if (!opened){
    ERROR(BOOST_PP_STRINGIZE(HFPP_FUNC_NAME) << ": Opening file " << Filename << " failed.");
    CR::LopesEventIn* lep = new CR::LopesEventIn; //Make a dummy data reader ....
    drp=lep;
  };
  return *drp;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"

//------------------------------------------------------------------------
//$DOCSTRING: Return information from a data file as a Python object.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hFileGetParameter
//-----------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF  (HPyObject)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (CRDataReader)(dr)()("Datareader object openen, e.g. with hFileOpen or crfile.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HString)(keyword)()("Keyword to be read out from the file metadata")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING
*/
HPyObject HFPP_FUNC_NAME(CRDataReader &dr, const HString key)
{
  HString key1(key);
  HString key2(key);
  key2[0]=(unsigned char)toupper((int)key2[0]);
  key1[0]=(unsigned char)tolower((int)key1[0]);
  DataReader *drp=&dr;
#define HFPP_REPEAT(TYPE,TYPE2,KEY)  if ((key1== #KEY) || (key2== #KEY)) {_H_NL_ TYPE result(drp->KEY ()); _H_NL_ HPyObject pyob((TYPE2)result); _H_NL_ return pyob;} else
    HFPP_REPEAT(uint,uint,nofAntennas)
    HFPP_REPEAT(uint,uint,nofSelectedChannels)
    HFPP_REPEAT(uint,uint,nofSelectedAntennas)
    HFPP_REPEAT(uint,uint,blocksize)
    HFPP_REPEAT(uint,uint,fftLength)
    HFPP_REPEAT(uint,uint,block)
    HFPP_REPEAT(uint,uint,stride)
    HFPP_REPEAT(uint,uint,nyquistZone)
    HFPP_REPEAT(double,double,sampleInterval)
    HFPP_REPEAT(double,double,referenceTime)
    HFPP_REPEAT(double,double,sampleFrequency)
    HFPP_REPEAT(uint,uint,nofBaselines)
#undef HFPP_REPEAT
#define HFPP_REPEAT(TYPE,TYPE2,KEY) if ((key1== #KEY) || (key2== #KEY)) {_H_NL_ CasaVector<TYPE> casavec(drp->KEY ()); _H_NL_ std::vector<TYPE2> result; _H_NL_ aipsvec2stlvec(casavec,result); _H_NL_ HPyObject pyob(result); _H_NL_ return pyob;} else
    HFPP_REPEAT(uint,HInteger,antennas)
    HFPP_REPEAT(uint,HInteger,selectedAntennas)
    HFPP_REPEAT(uint,HInteger,selectedChannels)
    HFPP_REPEAT(uint,HInteger,positions)
    HFPP_REPEAT(double,HNumber,increment)
    HFPP_REPEAT(double,HNumber,frequencyValues)
    HFPP_REPEAT(double,HNumber,frequencyRange)
    HFPP_REPEAT(int, HInteger, shift)
#undef HFPP_REPEAT

// --- Reading data from the headerrecord in a scalar---
#define HFPP_REPEAT(TYPE,TYPE2,KEY)  \
      if ((key1== #KEY) || (key2== #KEY)) {_H_NL_\
	if (drp->headerRecord().isDefined(#KEY)) {	_H_NL_	\
	TYPE result;  _H_NL_\
	drp->headerRecord().get(#KEY,result); _H_NL_\
        HPyObject pyob((TYPE2)result);return pyob;} _H_NL_	\
	else {HPyObject pyob("NONE");return pyob;}; _H_NL_ \
       	} else

    HFPP_REPEAT(uint,uint,Date)
    HFPP_REPEAT(casa::String,HString,Observatory)
    HFPP_REPEAT(int,int,Filesize)
    //	HFPP_REPEAT(double,double,dDate)
    HFPP_REPEAT(int,int,presync)
    HFPP_REPEAT(int,int,TL)
    HFPP_REPEAT(int,int,LTL)
    HFPP_REPEAT(int,int,EventClass)
    HFPP_REPEAT(casa::uChar,uint,SampleFreq)
    HFPP_REPEAT(uint,uint,StartSample)
#undef HFPP_REPEAT

// --- Reading data from the headerrecord in a vector ---
#define HFPP_REPEAT(TYPE,TYPE2,KEY)  if ((key1== #KEY) || (key2== #KEY)) {_H_NL_	\
      if (drp->headerRecord().isDefined(#KEY)) {  _H_NL_		\
      CasaVector<TYPE> casavec;  _H_NL_\
      drp->headerRecord().get(#KEY,casavec); _H_NL_\
      std::vector<TYPE2> result; _H_NL_\
      aipsvec2stlvec(casavec,result); _H_NL_ \
      HPyObject pyob(result);return pyob;} _H_NL_ \
      else {HPyObject pyob("NONE");return pyob;}; _H_NL_ \
      } else

      HFPP_REPEAT(int,HInteger,AntennaIDs)
      HFPP_REPEAT(int,HInteger,SAMPLE_OFFSET)
      HFPP_REPEAT(uint,HInteger,SAMPLE_NUMBER)
      HFPP_REPEAT(uint,HInteger,TIME)
    { HString result; result = result
#undef HFPP_REPEAT
#define HFPP_REPEAT(TYPE,TYPE2,KEY)  + #KEY + ", "
    HFPP_REPEAT(uint,uint,nofAntennas)
    HFPP_REPEAT(uint,uint,nofSelectedChannels)
    HFPP_REPEAT(uint,uint,nofSelectedAntennas)
    HFPP_REPEAT(uint,uint,nofBaselines)
    HFPP_REPEAT(uint,uint,block)
    HFPP_REPEAT(uint,uint,blocksize)
    HFPP_REPEAT(uint,uint,stride)
    HFPP_REPEAT(uint,uint,fftLength)
    HFPP_REPEAT(uint,uint,nyquistZone)
    HFPP_REPEAT(double,double,sampleInterval)
    HFPP_REPEAT(double,double,referenceTime)
    HFPP_REPEAT(double,double,sampleFrequency)
    HFPP_REPEAT(uint,HInteger,antennas)
    HFPP_REPEAT(uint,HInteger,selectedAntennas)
    HFPP_REPEAT(uint,HInteger,selectedChannels)
    HFPP_REPEAT(uint,HInteger,positions)
    HFPP_REPEAT(int,HInteger,shift)
    HFPP_REPEAT(double,HNumber,increment)
    HFPP_REPEAT(double,HNumber,frequencyValues)
    HFPP_REPEAT(double,HNumber,frequencyRange)
    HFPP_REPEAT(uint,uint,Date)
    HFPP_REPEAT(casa::String,HString,Observatory)
    HFPP_REPEAT(int,int,Filesize)
    // HFPP_REPEAT(double,double,dDate)
    HFPP_REPEAT(int,int,presync)
    HFPP_REPEAT(int,int,TL)
    HFPP_REPEAT(int,int,LTL)
    HFPP_REPEAT(int,int,EventClass)
    HFPP_REPEAT(casa::uChar,uint,SampleFreq)
    HFPP_REPEAT(uint,uint,StartSample)
    HFPP_REPEAT(int,HInteger,AntennaIDs)
			HFPP_REPEAT(int,HInteger,SAMPLE_OFFSET)
			HFPP_REPEAT(uint,HInteger,SAMPLE_NUMBER)
			HFPP_REPEAT(uint,HInteger,TIME)
//------------------------------------------------------------------------
#undef HFPP_REPEAT
		     + "keywords, help";
      if ((key1!="help") && (key1!="keywords")) cout << "Unknown keyword " << key1 <<"!"<<endl;
      if (key1!="keywords") cout  << BOOST_PP_STRINGIZE(HFPP_FUNC_NAME) << " - available keywords: "<< result <<endl;
      HPyObject pyob(result);
      return pyob;
    };
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Set parameters in a data file with a Python object as input.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hFileSetParameter
//-----------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF  (HBool)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (CRDataReader)(dr)()("Datareader object openen, e.g. with hFileOpen or crfile.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HString)(keyword)()("Keyword to be set in the file")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_2 (HPyObjectPtr)(pyob)()("Input parameter")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
////$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING
*/
HBool HFPP_FUNC_NAME(CRDataReader &dr, const HString key, const HPyObjectPtr pyob)
{
  DataReader *drp=&dr;
  HString key1(key);
  HString key2(key);
  key2[0]=(unsigned char)toupper((int)key2[0]);
  key1[0]=(unsigned char)tolower((int)key1[0]);
#define HFPP_REPEAT(TYPE,TYPE2,KEY) if ((key1== #KEY) || (key2==#KEY)) {TYPE input(TYPE2 (pyob)); drp->set##KEY (input);} else
  HFPP_REPEAT(uint,PyInt_AsLong,Block)
    HFPP_REPEAT(uint,PyInt_AsLong,Blocksize)
    HFPP_REPEAT(uint,PyInt_AsLong,StartBlock)
    HFPP_REPEAT(uint,PyInt_AsLong,Stride)
    HFPP_REPEAT(uint,PyInt_AsLong,SampleOffset)
    HFPP_REPEAT(uint,PyInt_AsLong,NyquistZone)
    HFPP_REPEAT(double,PyFloat_AsDouble,ReferenceTime)
    HFPP_REPEAT(double,PyFloat_AsDouble,SampleFrequency)
    HFPP_REPEAT(int,PyInt_AsLong,Shift)
    if ((key1=="shift") || (key2=="Shift")) {
      vector<int> stlvec(PyList2STLInt32Vec(pyob));
      drp->setShift(stlvec);
    } else  if ((key1=="selectedAntennas") || (key2=="SelectedAntennas")) {
      vector<uint> stlvec(PyList2STLuIntVec(pyob));
      uint * storage = &(stlvec[0]);
      casa::IPosition shape(1,stlvec.size()); //tell casa the size of the vector
      CasaVector<uint> casavec(shape,storage,casa::SHARE);
      drp->setSelectedAntennas(casavec,false);
    } else if ((key1=="selectedAntennasID") || (key2=="SelectedAntennasID")) {
      vector<uint> stlvec(PyList2STLuIntVec(pyob));
      uint * storage = &(stlvec[0]);
      casa::IPosition shape(1,stlvec.size()); //tell casa the size of the vector
      CasaVector<uint> casavec(shape,storage,casa::SHARE);
      drp->setSelectedAntennas(casavec);
    } else if ((key1=="shiftVector") || (key2=="ShiftVector")) {
      vector<int> stlvec(PyList2STLInt32Vec(pyob));
      drp->setShift(stlvec);
    } else
#undef HFPP_REPEAT
      // -- For documentation purposes --
#define HFPP_REPEAT(TYPE,TYPE2,KEY)  + #KEY + ", "
    { HString txt; txt = txt
    HFPP_REPEAT(uint,PyInt_AsLong,Blocksize)
    HFPP_REPEAT(uint,PyInt_AsLong,StartBlock)
    HFPP_REPEAT(uint,PyInt_AsLong,Block)
    HFPP_REPEAT(uint,PyInt_AsLong,Stride)
    HFPP_REPEAT(uint,PyInt_AsLong,SampleOffset)
    HFPP_REPEAT(uint,PyInt_AsLong,NyquistZone)
    HFPP_REPEAT(double,PyFloat_AsDouble,ReferenceTime)
    HFPP_REPEAT(double,PyFloat_AsDouble,SampleFrequency)
    HFPP_REPEAT(int,PyInt_AsLong,Shift)
    HFPP_REPEAT(int,XX,ShiftVector)
    HFPP_REPEAT(uint,XX,SelectedAntennas)
    HFPP_REPEAT(uint,XX,SelectedAntennasID)
#undef HFPP_REPEAT
		     + "help";
      if (key1!="help") {
	cout << "Unknown keyword " << key1 <<"!"<<endl;
	return false;
      };
      cout  << BOOST_PP_STRINGIZE(HFPP_FUNC_NAME) << " - available keywords: "<< txt <<endl;
    };
  return true;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//Copy of the hCopy function - duplicated here, since I am too lazy to turn the templated functions into an .h file that could be included here and be used with, e.g. CASA pointers

template <class Iter, class Iterin>
void uglycopy(const Iter vecout, const Iter vecout_end, const Iterin vecin, const Iterin vecin_end)
{
  // Declaration of variables
  typedef IterValueType T;
  Iter itout(vecout);
  Iterin itin(vecin);

  // Sanity check - but how should this error happen in a Python code ...?
  if ((vecout > vecout_end) || (vecin > vecin_end)) {
    throw PyCR::ValueError("Negative size of vector.");
    return;
  }

  // Copying of data
  while (itout != vecout_end) {
    *itout=hfcast<T>(*itin);
    ++itin; ++itout;
    if (itin==vecin_end) itin=vecin;
  };
}


//$DOCSTRING: Read a specified block of data from a Datareader object into a vector, where the size should be pre-allocated.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hFileRead
//-----------------------------------------------------------------------
#define HFPP_FUNC_MASTER_ARRAY_PARAMETER 2 // Use the third parameter as the master array for looping and history informations
#define HFPP_FUNCDEF  (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (CRDataReader)(dr)()("Datareader object, opened e.g. with hFileOpen or crfile.")(HFPP_PAR_IS_UNMUTABLE_SCALAR)()(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HString)(Datatype)()("Name of the data column to be retrieved (e.g., FFT, Fx,Time, Frequency...)")(HFPP_PAR_IS_UNMUTABLE_SCALAR)()(HFPP_PASS_AS_VALUE) //UNMUTABLE_SCALAR: will not be converted to a vector when used in looping mode.
#define HFPP_PARDEF_2 (HFPP_TEMPLATED_TYPE)(vec)()("Data (output) vector")(HFPP_PAR_IS_VECTOR)(STDIT)(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_3 (HInteger)(block)()("block number to read - read current block if negative")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_4 (HInteger)(antenna)()("antenna number to read - read all if negative")(HFPP_PAR_IS_UNMUTABLE_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Example:
  >>> filename=LOFARSOFT+\"/data/lofar/rw_20080701_162002_0109.h5\"
  >>> datafile=crfile(filename)
  >>> datafile[\"blocksize\"]=10
  >>> f=hArray(float,[10])
  >>> f.read(datafile,\"Fx\")
  >>> f.pprint(-1)
  [29,29,35,27,9,-8,-45,-74,-92,-109]

  >>> datafile[\"blocksize\"]=2
  >>> x=hArray(float,[5,2])
  >>> hFileRead(datafile,\"Fx\",x[...],Vector(range(2)))
  >>> x.mprint()
  [29,29]   # blck 0
  [35,27]   # blck 1
  [29,29]   # blck 0
  [35,27]   # blck 1
  [29,29]   # blck 0

  Note, that here block 0,1 and one are read in 2-3 times, since the
  array was longer then the block list specified! This is the default
  behaviour of looping arrays.

  You may only read the data into a section of your array:

  >>> x[1].read(datafile,\"Fx\",3)
  >>> x.mprint()
  [29,29]
  [-45,-74]
  [29,29]
  [35,27]
  [29,29]

  Now, we sneaked the 4th block (block 3) into the second row (index 1).

  Finally, you can also read the data into an array of different type
  (but that is slightly slower as scratch arrays are created for every
  row that is being read)

  >>> c=hArray(complex,[5,2])
  >>> c[...].read(datafile,\"Fx\",Vector([0,1,2,3]))
  >>> c.mprint()
  [(29,0),(29,0)]
  [(35,0),(27,0)]
  [(9,0),(-8,0)]
  [(-45,0),(-74,0)]
  [(29,0),(29,0)]   # here we have block 0 again ....
*/

template <class Iter>
void HFPP_FUNC_NAME(
		    CRDataReader &dr,
		    const HString Datatype,
		    const Iter vec, const Iter vec_end,
		    const HInteger block,
		    const HInteger antenna
		    )
{

  //Create a DataReader Pointer from an integer variable
  DataReader *drp=&dr;
  typedef IterValueType T;

  //Check whether it is non-NULL.
  if (drp==Null_p){
    ERROR(BOOST_PP_STRINGIZE(HFPP_FUNC_NAME) << ": pointer to FileObject is NULL, DataReader not found.");
    return;
  };
  if (block>=0) {drp->setBlock((uint)block);};
  if (antenna>=0) { //Select a single antenna - in a bit complicated way
    vector<uint> stlvec; stlvec.push_back((uint)antenna);
    uint * storage = &(stlvec[0]);
    casa::IPosition shape(1,stlvec.size()); //tell casa the size of the vector
    CasaVector<uint> casavec(shape,storage,casa::SHARE);
    drp->setSelectedAntennas(casavec,false);
  };

  //------TIME------------------------------
  if (Datatype=="Time") {
    casa::IPosition shape(1,vec_end-vec); //tell casa the size of the vector
    if (typeid(*vec) == typeid(double)) {
      double * storage = reinterpret_cast<double*>(&(*vec));
      CasaVector<double> casavec(shape,storage,casa::SHARE);
      drp->timeValues(casavec);
    }  else {
      CasaVector<double> casavec(shape);
      drp->timeValues(casavec);
      aipsvec2stdit(casavec,vec,vec_end);
    };
    //------FREQUENCY------------------------------
  } else if (Datatype=="Frequency") {
    CasaVector<double> val = drp->frequencyValues();
    aipsvec2stdit(val,vec,vec_end);
  }
//..........................................................................................

#define HFPP_REPEAT(TYPESTL,TYPECASA,FIELD,SIZE) CasaMatrix<TYPECASA> val=drp->FIELD(); _H_NL_ uglycopy(vec,vec_end,val.cbegin(),val.cend())

  //------Fx----------------------------TYPESTL,TYPECASA,FIELD,SIZE
  else if (Datatype=="Fx") {HFPP_REPEAT(HNumber,double,fx,blocksize);}
  //------VOLTAGE------------------------------
  else if (Datatype=="Voltage") {HFPP_REPEAT(HNumber,double,voltage,blocksize);}
  //------FFT------------------------------
  else if (Datatype=="FFT") {HFPP_REPEAT(HComplex,CasaComplex,fft,fftLength);}
  //------CALFFT------------------------------
  else if (Datatype=="CalFFT") {HFPP_REPEAT(HComplex,CasaComplex,calfft,fftLength);}
  else {
    ERROR(BOOST_PP_STRINGIZE(HFPP_FUNC_NAME) << ": Datatype=" << Datatype << " is unknown.");
  };
  return;
}
#undef HFPP_REPEAT
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"

//$DOCSTRING: Read data from a Datareader object (pointer in ``iptr``) into a vector, where the size should be pre-allocated.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hFileRead
//-----------------------------------------------------------------------
//#define HFPP_WRAPPER_CLASSES HFPP_CLASS_STL HFPP_CLASS_hARRAY  //HFPP_CLASS_hARRAYALL is missing due to a deficiency of hfppnew to deal with pass as reference
//#define HFPP_PYTHON_WRAPPER_CLASSES HFPP_CLASS_STL HFPP_CLASS_hARRAY
//#define HFPP_FUNC_KEEP_RETURN_TYPE_FIXED HFPP_TRUE //return a single DataReader object and not a vector thereof for array operations
#define HFPP_FUNC_MASTER_ARRAY_PARAMETER 2 // Use the third parameter as the master array for looping and history informations
//#define HFPP_FUNCDEF  (CRDataReader)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_UNMUTABLE_SCALAR)()(HFPP_PASS_AS_REFERENCE)
#define HFPP_FUNCDEF  (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (CRDataReader)(dr)()("Datareader object, opened e.g. with hFileOpen or crfile.")(HFPP_PAR_IS_UNMUTABLE_SCALAR)()(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HString)(Datatype)()("Name of the data column to be retrieved (e.g., FFT, Fx,Time, Frequency...)")(HFPP_PAR_IS_UNMUTABLE_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_2 (HFPP_TEMPLATED_TYPE)(vec)()("Data (output) vector")(HFPP_PAR_IS_VECTOR)(STDIT)(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING
*/

template <class Iter>
void HFPP_FUNC_NAME(
		    CRDataReader &dr,
		    const HString Datatype,
		    const Iter vec, const Iter vec_end
		    )
{
  hFileRead(dr,Datatype,vec,vec_end,-1,-1);
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"

//$DOCSTRING: Read data from a Datareader object into a vector, where the size should be pre-allocated.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hFileRead
//-----------------------------------------------------------------------
//#define HFPP_WRAPPER_CLASSES HFPP_CLASS_STL HFPP_CLASS_hARRAY  //HFPP_CLASS_hARRAYALL is missing due to a deficiency of hfppnew to deal with pass as reference
//#define HFPP_PYTHON_WRAPPER_CLASSES HFPP_CLASS_STL HFPP_CLASS_hARRAY
//#define HFPP_FUNC_KEEP_RETURN_TYPE_FIXED HFPP_TRUE //return a single DataReader object and not a vector thereof for array operations
//#define HFPP_FUNCDEF  (CRDataReader)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_UNMUTABLE_SCALAR)()(HFPP_PASS_AS_REFERENCE)
#define HFPP_FUNC_MASTER_ARRAY_PARAMETER 2 // Use the third parameter as the master array for looping and history informations
#define HFPP_FUNCDEF  (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (CRDataReader)(dr)()("Datareader object, opened e.g. with hFileOpen or crfile.")(HFPP_PAR_IS_UNMUTABLE_SCALAR)()(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HString)(Datatype)()("Name of the data column to be retrieved (e.g., FFT, Fx,Time, Frequency...)")(HFPP_PAR_IS_UNMUTABLE_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_2 (HFPP_TEMPLATED_TYPE)(vec)()("Data (output) vector")(HFPP_PAR_IS_VECTOR)(STDIT)(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_3 (HInteger)(block)()("block number to read - read current block if negative")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING
*/

template <class Iter>
void HFPP_FUNC_NAME(
		    CRDataReader &dr,
		    const HString Datatype,
		    const Iter vec, const Iter vec_end,
		    const HInteger block
		    )
{
  hFileRead(dr,Datatype,vec,vec_end,block,-1);
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//------------------------------------------------------------------------
//$DOCSTRING: Return a list of antenna positions from the CalTables - this is a test.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hCalTable
//-----------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF  (HPyObjectPtr)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (HString)(filename)()("Filename of the caltable")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_1 (HString)(keyword)()("Keyword to be read out from the file metadata (currenly only Position is implemented)")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_2 (HInteger)(date)()("Date for which the information is requested")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_3 (HPyObjectPtr)(pyob)()("(Python) List  with antenna IDs")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Example:
  >>> antennaIDs=hFileGetParameter(file,\"AntennaIDs\")
  >>> x=hCalTable(\"~/LOFAR/usg/data/lopes/LOPES-CalTable\",\"Positions\",obsdate,list(antennaIDs))
*/
HPyObjectPtr HFPP_FUNC_NAME(const HString filename, const HString keyword, const HInteger date, const HPyObjectPtr pyob) {
  CR::CalTableReader* CTRead = new CR::CalTableReader(filename);
  HInteger i,ant,size;
  casa::Vector<Double> tmpvec;
  Double tmpval=0.;
  HPyObjectPtr list=PyList_New(0),tuple;
  if (CTRead != NULL && PyList_Check(pyob)) {  //Check if CalTable was opened ... and Python object is a list
    size=PyList_Size(pyob);
    for (i=0;i<size;++i){  //loop over all antennas
      ant=PyInt_AsLong(PyList_GetItem(pyob,i));  //Get the ith element of the list, i.e. the antenna ID
      CTRead->GetData((uint)date, ant, keyword, &tmpvec);
      if (keyword=="Position") {
	tuple=PyTuple_Pack(3,PyFloat_FromDouble(tmpvec[0]),PyFloat_FromDouble(tmpvec[1]),PyFloat_FromDouble(tmpvec[2]));
      } else if (keyword=="Delay") {
	tuple=PyTuple_Pack(1,PyFloat_FromDouble(tmpval));
      };
      PyList_Append(list,tuple);
    };
  };
  delete CTRead;
  return list;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//------------------------------------------------------------------------
//$DOCSTRING: Return a list of antenna calibration values from the CalTables as a float vector.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hCalTableVector
//-----------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF  (HNumber)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_VECTOR)(STL)(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (HString)(filename)()("Filename of the caltable")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_1 (HString)(keyword)()("Keyword to be read out from the file metadata (currenly only Position is implemented)")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_2 (HInteger)(date)()("Date for which the information is requested (I guess Julian days? As output from DataReader)")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_3 (HPyObjectPtr)(pyob)()("(Python) List  with antenna IDs")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Description:

  Possible keywords: ``Positions``, ``Delay``

  Example:
  >>> antennaIDs=hFileGetParameter(file,\"AntennaIDs\")
  >>> vec=hCalTablePositions(\"~/LOFAR/usg/data/lopes/LOPES-CalTable\",\"Positions\",obsdate,list(antennaIDs))
*/
vector<HNumber> HFPP_FUNC_NAME(const HString filename, const HString keyword, const HInteger date, const HPyObjectPtr pyob) {
  CR::CalTableReader* CTRead = new CR::CalTableReader(filename);
  /*  cout << "summary:" << CTRead.summary() << endl;
  uint t = time();
  cout << "current time:" << t << endl;
  */
  vector<HNumber> outvec;
  HInteger i,ant,size;
  casa::Vector<Double> tmpvec;
  Double tmpval;
  if (CTRead != NULL && PyList_Check(pyob)) {  //Check if CalTable was opened ... and Python object is a list
    size=PyList_Size(pyob);
    for (i=0;i<size;++i){  //loop over all antennas
      ant=PyInt_AsLong(PyList_GetItem(pyob,i));  //Get the ith element of the list, i.e. the antenna ID
      if (keyword=="Position") {
	CTRead->GetData((uint)date, ant, keyword, &tmpvec);
	outvec.push_back(tmpvec[1]); outvec.push_back(tmpvec[0]); outvec.push_back(tmpvec[2]);
      } else if (keyword=="Delay") {
	CTRead->GetData((uint)date, ant, keyword, &tmpval);
	outvec.push_back(tmpval);
      };
    };
  };
  delete CTRead;
  return outvec;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//-----------------------------------------------------------------------
//$DOCSTRING: Dump a single vector to a file in binary format (machine dependent).
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hWriteFileBinary
//-----------------------------------------------------------------------
#define HFPP_WRAPPER_TYPES HFPP_ALL_PYTHONTYPES
#define HFPP_FUNCDEF (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (HFPP_TEMPLATED_TYPE)(vec)()("Output data vector.")(HFPP_PAR_IS_VECTOR)(STDIT)(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HString)(filename)()("Filename (including path if necessary) to write file to.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Usage:
  vec.writefilebinary(filename) -> dumps vector to file

  See also:
  hReadFileBinary, hWriteFileBinary, hWriteFileBinaryAppend

  Example:
  >>> v=Vector(float,10,fill=3.0)
  >>> hWriteFileBinary(v,\"test.dat\")
  >>> x=Vector(float,10,fill=1.0)
  >>> hReadFileBinary(x,\"test.dat\")
  >>> x
  Vector(float, 10, [3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0, 3.0])
*/
template <class Iter>
void HFPP_FUNC_NAME(const Iter vec, const Iter vec_end, HString filename) {
  if (vec_end<=vec) return;
  char * v1 = reinterpret_cast<char*>(&(*vec));
  char * v2 = reinterpret_cast<char*>(&(*vec_end));
  ofstream outfile(filename.c_str(), ios::out | ios::binary | ios::trunc);
  if (outfile){
    outfile.write(v1, (HInteger)(v2-v1));
    outfile.close();
  } else throw PyCR::IOError("hWriteFileBinary: Unable to open file "+filename+".");
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//-----------------------------------------------------------------------
//$DOCSTRING: Dump a single vector and append it to the end of a file in binary format (machine dependent).
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hWriteFileBinaryAppend
//-----------------------------------------------------------------------
#define HFPP_WRAPPER_TYPES HFPP_ALL_PYTHONTYPES
#define HFPP_FUNCDEF (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (HFPP_TEMPLATED_TYPE)(vec)()("Output data vector.")(HFPP_PAR_IS_VECTOR)(STDIT)(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HString)(filename)()("Filename (including path if necessary) to write file to.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Usage:
  vec.writefilebinaryappend(filename,true) -> appends binary vector data to end of file

  See also:
  hReadFileBinary, hWriteFileBinary, hWriteFileBinaryAppend

  Example:
  >>> v=Vector(float,10,fill=3.0)
  >>> hWriteFileBinary(v,\"test.dat\")
  >>> v.fill(2.0)
  >>> hWriteFileBinaryAppend(v,\"test.dat\")
  >>> x=Vector(float,20,fill=1.0)
  >>> hReadFileBinary(x,\"test.dat\")
  >>> x
  Vec(float,20)=[3.0,3.0,3.0,3.0,3.0,...,2.0,2.0]
*/
template <class Iter>
void HFPP_FUNC_NAME(const Iter vec,   const Iter vec_end, HString filename) {
  if (vec_end<=vec) return;
  char * v1 = reinterpret_cast<char*>(&(*vec));
  char * v2 = reinterpret_cast<char*>(&(*vec_end));
  fstream outfile(filename.c_str(), ios::out | ios::binary | ios::app);
  if (outfile){
    outfile.write(v1, (HInteger)(v2-v1));
    outfile.close();
  } else throw PyCR::IOError("hWriteFileBinaryAppend: Unable to open file "+filename+".");
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//-----------------------------------------------------------------------
//$DOCSTRING: Dump a single vector to a file in binary format (machine dependent).
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hWriteFileBinary
//-----------------------------------------------------------------------
#define HFPP_WRAPPER_TYPES HFPP_ALL_PYTHONTYPES
#define HFPP_FUNCDEF (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (HFPP_TEMPLATED_TYPE)(vec)()("Output data vector.")(HFPP_PAR_IS_VECTOR)(STDIT)(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HString)(filename)()("Filename (including path if necessary) to write file to.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_2 (HInteger)(start)()("Start writing at element 'start' (zero-based index), the data block length to write is determined by the vector length.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Description:
  Attention: if a file exists already and a start value is specified,
  then it will not be overwritten, but the data will be written into
  the location asked by start. This can be dangerous if one writes a
  smaller file (e.g. with ``start=0``) to a pre-existing older and larger
  file because then the 'new' file will contain the rest of the old
  file.

  Usage:
  vec.writedump(filename,start) -> writes binary vector data at location start in the file

  See also:
  hReadFileBinary, hWriteFileBinary, hWriteFileBinaryAppend

  Example:
  >>> v=Vector(float,10,fill=3.0)
  >>> hWriteFileBinary(v,\"test.dat\",0)
  >>> v.fill(2.0)
  >>> hWriteFileBinary(v,\"test.dat\",5)
  >>> x=Vector(float,15,fill=1.0)
  >>> hReadFileBinary(x,\"test.dat\")
  >>> x
  Vector(float, 15, [3.0, 3.0, 3.0, 3.0, 3.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0, 2.0])
*/
template <class Iter>
void HFPP_FUNC_NAME(const Iter vec, const Iter vec_end, HString filename, HInteger start) {
  if (vec_end<=vec) return;
  char * v1 = reinterpret_cast<char*>(&(*vec));
  char * v2 = reinterpret_cast<char*>(&(*(vec+1)));
  char * vend = reinterpret_cast<char*>(&(*vec_end));
  fstream outfile;
  ifstream f(filename.c_str(), ios::binary | ios::in);
  if (f) {//file exists
    f.close();
    outfile.open(filename.c_str(), ios::binary | ios::out | ios::in);
  } else {//file exists so, needs to be created (and cannot be opened as r/w)
    f.close();
    outfile.open(filename.c_str(), ios::binary | ios::out);
  };
  if (outfile){
    if (start>0) outfile.seekp(((HInteger) (v2-v1))*start);
    outfile.write(v1, (HInteger)(vend-v1));
    outfile.close();
  } else throw PyCR::IOError("hWriteFileBinary: Unable to open file "+filename+".");
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//-----------------------------------------------------------------------
//$DOCSTRING: Read a single vector from a file which was dumped in (machine-dependent) binary format
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hReadFileBinary
//-----------------------------------------------------------------------
#define HFPP_WRAPPER_TYPES HFPP_ALL_PYTHONTYPES
#define HFPP_FUNCDEF (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (HFPP_TEMPLATED_TYPE)(vec)()("Input data vector.")(HFPP_PAR_IS_VECTOR)(STDIT)(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HString)(filename)()("Filename (including path if necessary) to read data from. The vector needs to have the length corresponding to the file size.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Usage:
  vec.readdump(filename) -> reads dumped vector from file

  See also:
  hReadFileBinary, hWriteFileBinary, hWriteFileBinaryAppend, hWriteFileText, hReadFileText

  Example:
  >>> v=Vector(float,10,fill=3.0)
  >>> hWriteFileBinary(v,\"test.dat\")
  >>> x=Vector(float,10,fill=1.0)
  >>> hReadFileBinary(x,\"test.dat\")
  >>> x
  Vec(float,10)=[3.0,3.0,3.0,3.0,3.0,...]
*/
template <class Iter>
void HFPP_FUNC_NAME(const Iter vec,   const Iter vec_end, HString filename) {
  char * v1 = reinterpret_cast<char*>(&(*vec));
  char * v2 = reinterpret_cast<char*>(&(*vec_end));
  fstream outfile(filename.c_str(), ios::in | ios::binary);
  if (outfile){
    outfile.read(v1, (HInteger)(v2-v1));
    outfile.close();
  } else throw PyCR::IOError("hReadFileBinary: Unable to open file "+filename+".");
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//-----------------------------------------------------------------------
//$DOCSTRING: Read a section (block) of a single vector from a file which was dumped in (machine-dependent) binary format.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hReadFileBinary
//-----------------------------------------------------------------------
#define HFPP_WRAPPER_TYPES HFPP_ALL_PYTHONTYPES
#define HFPP_FUNCDEF (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (HFPP_TEMPLATED_TYPE)(vec)()("Input data vector.")(HFPP_PAR_IS_VECTOR)(STDIT)(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HString)(filename)()("Filename (including path if necessary) to read data from. The vector needs to have the length corresponding to the file size.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_2 (HInteger)(start)()("Start reading at element 'start' (zero-based index), the data block length to read is determined by the vector length.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Usage:
  vec.readdump(filename,start) -> reads a specfic block from file starting at element start (blocklength determined by ``len(vec)``).

  See also:
  hReadFileBinary, hWriteFileBinary, hWriteFileBinaryAppend, hWriteFileText, hReadFileText

  Example:
  >>> x=hArray(range(20))
  >>> x.writefilebinary(\"test.dat\")
  >>> v=hArray(int,10)
  >>> v.readfilebinary(\"test.dat\",0)
  >>> v.pprint()
  [0,1,2,3,4,5,6,7,8,9]

  #Now read the third block (of length 3) into the first part of the vector
  >>> v[0:3].readfilebinary(\"test.dat\",2*3)
  >>> v.pprint()
  [6,7,8,3,4,5,6,7,8,9]
*/
template <class Iter>
void HFPP_FUNC_NAME(const Iter vec,   const Iter vec_end, HString filename, HInteger start) {
  char * v1 = reinterpret_cast<char*>(&(*vec));
  char * v2 = reinterpret_cast<char*>(&(*(vec+1)));
  char * vend = reinterpret_cast<char*>(&(*vec_end));
  ifstream outfile(filename.c_str(), ios::in | ios::binary);
  if (outfile){
    if (start>0) outfile.seekg(((HInteger) (v2-v1))*start);
    outfile.read(v1, (HInteger)(vend-v1));
    outfile.close();
  } else throw PyCR::IOError("hReadFileBinary: Unable to open file "+filename+".");
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//-----------------------------------------------------------------------
//$DOCSTRING: Read columns of data from a file in text (ASCII) form into an array
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hReadFileText
//-----------------------------------------------------------------------
#define HFPP_WRAPPER_TYPES HFPP_ALL_PYTHONTYPES
#define HFPP_FUNCDEF (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (HFPP_TEMPLATED_TYPE)(vec)()("Input data vector.")(HFPP_PAR_IS_VECTOR)(STDIT)(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HString)(filename)()("Filename (including path if necessary) to read data from.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_2 (HInteger)(skiprows)()("The number of rows to skip before reading the data.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_3 (HInteger)(columns)()("An array with a list of column numbers (starting at zero) from which to read data into the array. Read all if empty.")(HFPP_PAR_IS_VECTOR)(STDIT)(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_4 (HInteger)(nrows)()("The number of rows to read. If equal to -1 read all.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Usage:
  vec.readtexttable(filename,skiprows,columns,nrows) -> reads a number of columns into the vector vec

  See also:
  hReadFileBinary, hWriteFileBinary, hWriteFileBinaryAppend, hWriteFileText, hReadFileText

  Example:
  >>> from pycrtools import *
  >>> vec=Vector(float,24*3)
  >>> hReadFileText(vec, \"AERAcoordinates-1.txt\", 4, Vector([5,3,7]),10)

  # This will skip the first 4 rows and then read 10 rows where columns
  # 5,3,&7 will be read into the vector vec. The ordering will be [l0.c5,
  # l0.c3, l0.c7, l1.c5, l1.c3, l1.c7, ..., l9.c7] (where l and c stand
  # for row and column respectively)
*/
template <class Iter, class IterI>
void HFPP_FUNC_NAME(const Iter vec,   const Iter vec_end,
                    const HString filename,
                    const HInteger skiprows,
                    const IterI columns, const IterI columns_end,
                    const HInteger nrows)
{
  ifstream infile(filename.c_str(), ios::in);
  HInteger nskip(skiprows);
  HInteger rowsread(0);
  HString row;
  IterI col_it;
  Iter vec_it(vec);
  HInteger col_max = 0;

  typedef IterValueType T;
  vector<HString> words;

  // Check column numbers
  col_it = columns;
  while (col_it != columns_end) {
    if (*col_it < 0) {
      throw PyCR::IndexError("Negative index numbers not allowed");
    }
    if (*col_it > col_max) {
      col_max = *col_it;
    }
    ++col_it;
  }

  if (infile){
    while(getline(infile,row) && (!infile.eof()) && (rowsread != nrows) && (vec_it < vec_end)) {
      if (nskip>0) {
	--nskip;
      } else {
	++rowsread;
	words = stringSplit(row); //Split the row into words separated by whitespaces
        if (col_max < (HInteger)words.size()) {
          col_it = columns;
          while (col_it != columns_end) { //loop over all columns listed in the columns array
            if ( vec_it < vec_end) {
              *vec_it = hfcast<T>(words[*col_it]);
               ++vec_it;
            }
            ++col_it;
          }
         } else {
          // warning: column index out of range for this line: line is skipped
          cout << "Warning: requested field not found in row " << rowsread << ": row skipped" << endl;
        }
      }
    }
    infile.close();
  } else {
    throw PyCR::IOError("hReadFileText: Unable to open file "+filename+".");
  }
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"

//-----------------------------------------------------------------------
//$DOCSTRING: Write columns of data in an array to a (ASCII) text file.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hWriteFileText
//-----------------------------------------------------------------------
#define HFPP_WRAPPER_TYPES HFPP_ALL_PYTHONTYPES
#define HFPP_FUNCDEF (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (HFPP_TEMPLATED_TYPE)(vec)()("Output data vector.")(HFPP_PAR_IS_VECTOR)(STDIT)(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HString)(filename)()("Filename (including path if necessary) to write data to.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_2 (HString)(header)()("The header to preceed the table, e.g. a description of the columns.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_3 (HInteger)(ncolumns)()("The number of columns in the table.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_4 (HInteger)(nrows)()("The number of rows to write. If equal to -1 write all.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_5 (HBool)(append)()("If true append data to the file, rather than overwrite it.")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Usage:
  vec.writetexttable(filename,header,ncolumns,nrows) -> write vec as table into filename

  See also:
  hReadFileBinary, hWriteFileBinary, hWriteFileBinaryAppend, hWriteFileText, hReadFileText

  Example:
  >>> vec=Vector(float,24*3,fill=range(24*3))
  >>> vec.writetexttable(\"table.dat\",\"# col1, col2, col3\", 3, 24,False)
*/
template <class Iter>
void HFPP_FUNC_NAME(const Iter vec, const Iter vec_end,
                    const HString filename,
                    const HString header,
                    const HInteger ncolumns,
                    const HInteger nrows,
                    const HBool append)
{
  Iter vec_it(vec);
  HInteger row(0);
  HInteger col;
  ofstream outfile;

  // Checks on input parameters
  if (ncolumns <= 0) {
    throw PyCR::ValueError("Number of columns <= 0.");
  }
  if ((std::distance(vec,vec_end) % ncolumns) != 0) {
    throw PyCR::ValueError("Vector size is not a multiple of the number of columns.");
  }

  if (append) {
    outfile.open(filename.c_str(), ios::out | ios::app);
  } else {
    outfile.open(filename.c_str(), ios::out);
  }

  if (outfile) {
    if (!append) { // Add a header for new files.
      outfile << header << endl;
    }
    while((row != nrows) && (vec_it != vec_end)) {
      col=ncolumns;
      while (col != 0) { // Loop over all columns
	if (vec_it != vec_end) {
	  outfile << *vec_it << " ";
	  ++vec_it; --col;
	}
      }
      ++row;
      outfile << endl;
    }
    outfile.close();
  } else {
    throw PyCR::IOError("hWriteFileText: Unable to open file "+filename+".");
  }
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"



// ========================================================================
//
//  AERA
//
// ========================================================================

#ifdef PYCRTOOLS_WITH_AERA

//$DOCSTRING: Function to open a file based on a filename and returning an AERA datareader object.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERAFileOpen
//------------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF  (AERADatareader)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_0 (HString)(Filename)()("Filename of file to open including full directory name")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING
*/
AERA::Datareader& HFPP_FUNC_NAME (const HString Filename)
{
  AERA::Datareader* dr_ptr = NULL;

  dr_ptr = new AERA::Datareader();

  if (dr_ptr != NULL) {
    dr_ptr->open(Filename);
    dr_ptr->read();
  }

  return *dr_ptr;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"



//$DOCSTRING: Function to close an AERA data file.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERAFileClose
//------------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF  (HBool)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING
*/
HBool HFPP_FUNC_NAME (AERA::Datareader& dr)
{
  HBool status = false;

  status = dr.close();

  return status;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"



//$DOCSTRING: Get attribute of an AERA data file.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERAGetAttribute
//-----------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF (HPyObject)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HString)(keyName)()("Key name of parameter")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Description:
  The following keywords can be used

    =============== ========================
    File keyname    Description
    --------------- ------------------------
    filename        File name
    nevents         Number of events
    runnumber       Run number
    runmode         Run mode
    fileserialnr    File serial number
    firsteventnr    First event number
    firsteventtime  Time of first event
    lasteventnr     Last event number
    lasteventtime   Time of last event
    =============== ========================

    ================= ================================
    Event keyname     Description
    ----------------- --------------------------------
    nlocalstations    Number of local stations
    eventlength       Event length
    eventid           Event ID
    t3                T3 event ID
    timestampsec      Timestamp in seconds
    timestampnanosec  Nanosecond part of timestamp
    eventtype         Type of event
    ================= ================================

    ===================== ===============================
    Localstation keyname  Description
    --------------------- -------------------------------
    bodylength            Length of the event body
    localstationid        Local station ID
    hardware              Hardware description
    triggerflag           Trigger flag
    triggerposition       Trigger position
    samplingfreq          Sampling frequency
    channelmask           Channel mask
    adcresolution         ADC resolution
    tracelength           Trace length
    ===================== ===============================
*/
HPyObject HFPP_FUNC_NAME (AERA::Datareader& dr, const std::string keyValue)
{
  std::string keyName = keyValue;
  HPyObject pyObj = HPyObject(NULL);
  AERA::Data::Header* header_ptr = dr.header();
  stringToLower(keyName);

  // ___________________________________________________________________
  //                                                                File

  if ( "filename" == keyName ) { //                            Filename
    std::string result = dr.getFileName();
    return HPyObject(result);
  } else if ( "nevents" == keyName ) { //              Number of events
    int result = dr.nEvents();
    return HPyObject(result);
  } else if ( NULL != header_ptr ) {
    if ( "runnumber" == keyName ) { //                       Run number
      int result = header_ptr->getRunNumber();
      return HPyObject(result);
    } else if ( "runmode" == keyName ) { //                    Run mode
      int result = header_ptr->getRunMode();
      return HPyObject(result);
    } else if ( "fileserialnr" == keyName ) { //         File serial nr
      int result = header_ptr->getFileSerialNumber();
      return HPyObject(result);
    } else if ( "firsteventnr" == keyName ) { //     First event number
      int result = header_ptr->getFirstEvent();
      return HPyObject(result);
    } else if ( "firsteventtime" == keyName ) { //     Time first event
      std::string result = header_ptr->getFirstEventTime();
      return HPyObject(result);
    } else if ( "lasteventnr" == keyName ) {  //      Last event number
      int result = header_ptr->getLastEvent();
      return HPyObject(result);
    } else if ( "lasteventtime" == keyName ) {  //      Time last event
      std::string result = header_ptr->getLastEventTime();
      return HPyObject(result);
    }
  }

  // ___________________________________________________________________
  //                                                               Event

  AERA::Data::Event* event_ptr = dr.currentEvent();
  AERA::Data::EventHeader* eventhdr_ptr = NULL;

  if ( NULL != event_ptr ) {
    if ( "nlocalstations" == keyName ) { //    Number of local stations
      AERA::Data::Event* event_ptr = dr.currentEvent();
      int result = event_ptr->nLocalStations();
      return HPyObject(result);
    }
    eventhdr_ptr = event_ptr->eventHeader();
    if ( NULL != eventhdr_ptr ) {
      if ( "eventlength" == keyName ) { //                 Event length
        int result = eventhdr_ptr->getEventLength();
        return HPyObject(result);
      } else if ( "eventid" == keyName ) { //                  Event ID
        int result = eventhdr_ptr->getEventID();
        return HPyObject(result);
      } else if ( "t3" == keyName ) { //                    T3 event ID
        int result = eventhdr_ptr->getT3eventID();
        return HPyObject(result);
      } else if ( "timestampsec" == keyName ) { //        Timestamp (s)
        unsigned int result = eventhdr_ptr->getSeconds();
        return HPyObject(result);
      } else if ( "timestampnanosec" == keyName ) { //   Timestamp (ns)
        unsigned int result = eventhdr_ptr->getNanoSeconds();
        return HPyObject(result);
      } else if ( "eventtype" == keyName ) {//        Type of the event
        int result = eventhdr_ptr->getEventType();
        return HPyObject(result);
      }
    }
  } else {
    throw PyCR::ValueError("Event pointer is NULL.");
    return pyObj;
  }

  // ___________________________________________________________________
  //                                                       Local station

  if ( NULL != event_ptr ) {
    AERA::Data::LocalStation* ls_ptr = event_ptr->currentLocalStation();
    if ( NULL != ls_ptr ) {
      if ( "bodylength" == keyName ) { //          Length of event body
        UINT16 result = ls_ptr->getLength();
        return HPyObject(result);
      } else if ( "localstationid" == keyName ) { //   Local station ID
        UINT16 result = ls_ptr->getLocalStationID();
        return HPyObject(result);
      } else if ( "hardware" == keyName ) { //     Hardware description
        std::string result = ls_ptr->getHardware();
        return HPyObject(result);
      } else if ( "triggerflag" == keyName ) { //          Trigger flag
        UINT16 result = ls_ptr->getTriggerFlag();
        return HPyObject(result);
      } else if ( "triggerposition" == keyName ) { //  Trigger position
        UINT16 result = ls_ptr->getTriggerPosition();
        return HPyObject(result);
      } else if ( "samplingfreq" == keyName ) { //   Sampling frequency
        UINT16 result = ls_ptr->getSamplingFrequency();
        return HPyObject(result);
      } else if ( "channelmask" == keyName ) { //          Channel mask
        UINT16 result = ls_ptr->getChannelMask();
        return HPyObject(result);
      } else if ( "adcresolution" == keyName ) { //      ADC resolution
        UINT16 result = ls_ptr->getADCResolution();
        return HPyObject(result);
      } else if ( "tracelength" == keyName ) { //          Trace length
        UINT16 result = ls_ptr->getTraceLength();
        return HPyObject(result);
      }
    } else {
      throw PyCR::ValueError("LocalStation pointer is NULL.");
      return pyObj;
    }
  }

  // ___________________________________________________________________
  //                                                                Test

  if ( "tvectori" == keyName ) { //                  Test vector integer
    std::vector<HInteger> result(1024,hfnull<HInteger>());
    return HPyObject(result);
  } else   if ( "tvectorf" == keyName ) { //          Test vector number
    std::vector<HNumber> result(1024,hfnull<HNumber>());
    return HPyObject(result);
  } else   if ( "tvectorc" == keyName ) { //         Test vector complex
    std::vector<HComplex> result(1024,hfnull<HComplex>());
    return HPyObject(result);
  }

  // ___________________________________________________________________
  //                                                          Other keys

  throw PyCR::KeyError("Key is not defined.");

  if ( "default" == keyName ) { //                              Default
    cout << "Using the default value (0)" << endl;
    pyObj = HPyObject(hfnull<HInteger>());
  }

  return pyObj;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Time series data for alle channels of current localstation.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERAGetADCData
//-----------------------------------------------------------------------
#define HFPP_FUNC_MASTER_ARRAY_PARAMETER 1
#define HFPP_FUNCDEF (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_UNMUTABLE_SCALAR)()(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HFPP_TEMPLATED_TYPE)(vec)()("Output vector")(HFPP_PAR_IS_VECTOR)(STDIT)(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING
*/
template <class Iter>
void HFPP_FUNC_NAME (AERA::Datareader& dr,
                     const Iter vec, const Iter vec_end)
{
  Iter v_begin = vec;
  Iter v_end   = vec_end;
  int tracelength = 0;
  AERA::Data::Event* event_ptr = dr.currentEvent();
  AERA::Data::LocalStation* station_ptr = NULL;

  if ( NULL != event_ptr ) {
    station_ptr = event_ptr->currentLocalStation();
    if ( NULL != station_ptr ) {
      tracelength = station_ptr->getTraceLength();
      for (int channel = 0; channel < N_CHANNELS; ++channel) {
        v_begin = vec + tracelength*channel;
        v_end = v_begin + tracelength;
        if ( (v_begin < vec_end) && (v_end <= vec_end) ) {
          hAERAGetADCData(dr, v_begin, v_end, channel);
        }
      }
    } else {
      cerr << "ERROR: [hAERAGetADCData] LocalStation pointer is NULL." << endl;
    }
  } else {
    cerr << "ERROR: [hAERAGetADCData] Event pointer is NULL." << endl;
  }
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Time series data for specified channel of the current localstation.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERAGetADCData
//-----------------------------------------------------------------------
#define HFPP_FUNC_MASTER_ARRAY_PARAMETER 1
#define HFPP_FUNCDEF (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_UNMUTABLE_SCALAR)()(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_1 (HFPP_TEMPLATED_TYPE)(vec)()("Output vector")(HFPP_PAR_IS_VECTOR)(STDIT)(HFPP_PASS_AS_REFERENCE)
#define HFPP_PARDEF_2 (HInteger)(Channel)()("Channel number to read out")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING
*/
template <class Iter>
void HFPP_FUNC_NAME (AERA::Datareader& dr,
                     const Iter vec, const Iter vec_end,
                     const HInteger channel)
{
  vector<int> ADCChannelData;
  vector<int>::iterator ADC_it;
  AERA::Data::Event* event_ptr = dr.currentEvent();
  AERA::Data::LocalStation* station_ptr = NULL;
  Iter vec_it = vec;

  if ( ( 0 <= channel ) && ( N_CHANNELS > channel ) ) {
    if ( NULL != event_ptr ) {
      station_ptr = event_ptr->currentLocalStation();
      if ( NULL != station_ptr ) {
        ADCChannelData = station_ptr->getADCData((unsigned short) channel);
        ADC_it = ADCChannelData.begin();
        while ( ( ADC_it != ADCChannelData.end() ) &&
                ( vec_it != vec_end ) ) {
          *vec_it = (IterValueType)(*ADC_it);
          ++ADC_it; ++vec_it;
        }
      } else {
        cerr << "ERROR [hAERAGetADCData]: Local station pointer is NULL." << endl;
      }
    } else {
      cerr << "ERROR [hAERAGetADCData]: Event pointer is NULL." << endl;
    }
  } else {
    throw PyCR::ValueError("Invalid value of channel number.");
  }
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"

//$DOCSTRING: Set the event pointer to the first event of the file.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERAFirstEvent
//-----------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF (HBool)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Description:
  Returns ``True`` if the event pointer was successfully changed, ``False`` otherwise.
*/
HBool HFPP_FUNC_NAME (AERA::Datareader& dr)
{
  HBool status = false;
  AERA::Data::Event* event_ptr = NULL;

  event_ptr = dr.firstEvent();
  if ( NULL != event_ptr ) {
    status = hAERAFirstLocalStation(dr);
  }

  return status;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Set the event pointer to the previous event.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERAPrevEvent
//-----------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF (HBool)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Description:
  Return ``True`` if the event pointer was successfully changed, ``False`` otherwise.
*/
HBool HFPP_FUNC_NAME (AERA::Datareader& dr)
{
  HBool status = false;
  AERA::Data::Event* event_ptr = NULL;

  if ( ! dr.isFirstEvent() ) {
    event_ptr = dr.prevEvent();

    if ( event_ptr != NULL ) {
      status = true;
    }
  }
  return status;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Set the event pointer to the next event.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERANextEvent
//-----------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF (HBool)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Description:
  Returns ``True`` if the event pointer was successfully changed, ``False`` otherwise.
*/
HBool HFPP_FUNC_NAME (AERA::Datareader& dr)
{
  HBool status = false;
  AERA::Data::Event* event_ptr = NULL;

  if ( ! dr.isLastEvent() ) {
    event_ptr = dr.nextEvent();

    if ( event_ptr != NULL ) {
      status = true;
    }
  }
  return status;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Set the event pointer to the last event of this file.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERALastEvent
//-----------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF (HBool)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Description:
  Returns ``True`` if the event pointer was successfully changed, ``False`` otherwise.
*/

HBool HFPP_FUNC_NAME (AERA::Datareader& dr)
{
  HBool status = false;
  AERA::Data::Event* event_ptr = NULL;

  event_ptr = dr.lastEvent();
  if ( NULL != event_ptr ) {
    status = hAERAFirstLocalStation(dr);
  }

  return status;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Set the Local Station pointer to the first Local station of this event.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERAFirstLocalStation
//-----------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF (HBool)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END ----------------------------------------------------------
/*!
 \brief $DOCSTRING
  $PARDOCSTRING

  Description:
  Returns ``True`` if the local station pointer was successfully changed, ``False`` otherwise.
*/
HBool HFPP_FUNC_NAME (AERA::Datareader& dr)
{
  HBool status = false;
  AERA::Data::Event* event_ptr = dr.currentEvent();
  AERA::Data::LocalStation* localstation_ptr = NULL;

  if ( NULL != event_ptr ) {
    localstation_ptr = event_ptr->firstLocalStation();

    if ( NULL != localstation_ptr ) {
      status = true;
    }
  }

  return status;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Set the local station pointer to the previous local station of the current event.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERAPrevLocalStation
//-----------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF (HBool)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Description:
  Return ``True`` if the local station pointer was successfully changed, ``False`` otherwise.
*/
HBool HFPP_FUNC_NAME (AERA::Datareader& dr)
{
  HBool status = false;
  AERA::Data::Event* event_ptr = dr.currentEvent();
  AERA::Data::LocalStation* localstation_ptr = NULL;

  if ( NULL != event_ptr ) {
    if ( ! event_ptr->isFirstLocalStation() ) {
      localstation_ptr = event_ptr->prevLocalStation();

      if ( NULL != localstation_ptr ) {
        status = true;
      }
    }
  }

  return status;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Set the local station pointer to the next local station of the current event.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERANextLocalStation
//-----------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF (HBool)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END ----------------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Description:
  Returns ``True`` if the local station pointer was successfully changed, ``False`` otherwise.
*/
HBool HFPP_FUNC_NAME (AERA::Datareader& dr)
{
  HBool status = false;
  AERA::Data::Event* event_ptr = dr.currentEvent();
  AERA::Data::LocalStation* localstation_ptr = NULL;

  if ( NULL !=  event_ptr ) {
    if ( ! event_ptr->isLastLocalStation() ) {
      localstation_ptr = event_ptr->nextLocalStation();

      if ( NULL != localstation_ptr ) {
        status = true;
      }
    }
  }

  return status;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Set the local station pointer to the last local station of the current event.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERALastLocalStation
//------------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF  (HBool)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING

  Description:
  Returns ``True`` if the local station pointer was successfully changed, ``False`` otherwise.
*/
HBool HFPP_FUNC_NAME (AERA::Datareader& dr)
{
  HBool status = false;
  AERA::Data::Event* event_ptr = dr.currentEvent();
  AERA::Data::LocalStation* localstation_ptr = NULL;

  if ( NULL != event_ptr ) {
    localstation_ptr = event_ptr->lastLocalStation();

    if ( NULL != localstation_ptr ) {
      status = true;
    }
  }

  return status;
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Print a brief summary of the AERA data file contents and current settings.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERAFileSummary
//------------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF  (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING
*/
void HFPP_FUNC_NAME (AERA::Datareader& dr)
{
  dr.summary();
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Print a brief summary of the AERA event contents.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERAEventSummary
//------------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF  (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING
*/
void HFPP_FUNC_NAME (AERA::Datareader& dr)
{
  AERA::Data::Event* event_ptr = dr.currentEvent();

  if ( NULL != event_ptr ) {
    event_ptr->summary();
  }
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"


//$DOCSTRING: Print a brief summary of the AERA local station contents.
//$COPY_TO HFILE START --------------------------------------------------
#define HFPP_FUNC_NAME hAERALocalStationSummary
//------------------------------------------------------------------------
#define HFPP_BUILD_ADDITIONAL_Cpp_WRAPPERS HFPP_NONE
#define HFPP_FUNCDEF  (HFPP_VOID)(HFPP_FUNC_NAME)("$DOCSTRING")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_VALUE)
#define HFPP_PARDEF_0 (AERADatareader)(dr)()("AERA Datareader object")(HFPP_PAR_IS_SCALAR)()(HFPP_PASS_AS_REFERENCE)
//$COPY_TO END --------------------------------------------------
/*!
  \brief $DOCSTRING
  $PARDOCSTRING
*/
void HFPP_FUNC_NAME (AERA::Datareader& dr)
{
  AERA::Data::Event* event_ptr = dr.currentEvent();
  AERA::Data::LocalStation* localstation_ptr = NULL;

  if ( NULL != event_ptr ) {
    localstation_ptr = event_ptr->currentLocalStation();

    if ( NULL != localstation_ptr ) {
      localstation_ptr->summary();
    }
  }
}
//$COPY_TO HFILE: #include "hfppnew-generatewrappers.def"

#endif /* PYCRTOOLS_WITH_AERA */