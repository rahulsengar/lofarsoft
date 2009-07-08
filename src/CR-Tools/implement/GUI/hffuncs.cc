//================================================================================
// ATTENTION: DON'T EDIT THIS FILE!!! IT IS GENERATED AUTOMATICALLY BY hfprep.awk
//================================================================================
//     File was generated from hffuncs_awk.cc on Wed Jul 08 21:36:51 CEST 2009
//--------------------------------------------------------------------------------
//
//#define DBG_MODE 0
//#define DBG_MODE 1

// CASA/casacore header files

//using casa::Bool;
//using casa::Double;
//using casa::Matrix;
//using casa::String;
//using casa::Vector;

using namespace std;
#include <boost/python/class.hpp>
#include <boost/python/module.hpp>
#include <boost/python/def.hpp>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/implicit.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/overloads.hpp>
#include <boost/python/args.hpp>
#include <boost/python/tuple.hpp>
#include <boost/python/class.hpp>
#include <boost/python/return_internal_reference.hpp>
#include <boost/python/operators.hpp>
#include <boost/python/object_operators.hpp>
#include <boost/thread/thread.hpp>

//#include <QtGui/QApplication>
//#include <QtGui/QtGui>

//#include <mgl/mgl_qt.h>

//#include <GUI/mainwindow.h>
#include <GUI/hfdefs.h>
#include <GUI/hfcast.h> 
#include <GUI/hfget.h>
#include <GUI/hffuncs.h>  
#include <crtools.h>

#include "Data/LopesEventIn.h"
#include "Data/LOFAR_TBB.h"
#include "dal/TBB_Timeseries.h"

/*!
\brief Get the extension of the filename
 */
HString file_get_extension(HString filename){
  HInteger size=filename.size();
  HInteger pos=filename.rfind('.',size);
  pos++;
  if (pos>=size) {return "";}
  else {return filename.substr(pos,size-pos);};
}
/*!
\brief Determine the filetype based on the extension of the filename
 */

HString determine_filetype(HString filename){
  HString ext=file_get_extension(filename);
  HString typ="";
  if (ext=="event") typ="LOPESEvent";
  else if (ext=="h5") typ="LOFAR_TBB";
  return typ;
}


/*========================================================================
  class ObjectFunctionClass
  ========================================================================

  Generic class that object functions are based on

*/

//virtual functions//

/* 
The process_X methods will be defined in the object functions to call
a single templated (or not if typing is not relevant) method process
that defines the function to be performed.
*/
void ObjectFunctionClass::process_P(F_PARAMETERS_T(HPointer)) {ERROR("ObjectFunctionClass: generic process call of function" << getName(true) << ", Call of Type HPointer not defined");};
void ObjectFunctionClass::process_I(F_PARAMETERS_T(HInteger)) {ERROR("ObjectFunctionClass: generic process call of function" << getName(true) << ", Call of Type HInteger not defined");};
void ObjectFunctionClass::process_N(F_PARAMETERS_T(HNumber) ) {ERROR("ObjectFunctionClass: generic process call of function" << getName(true) << ", Call of Type HNumber not defined");};
void ObjectFunctionClass::process_C(F_PARAMETERS_T(HComplex)) {ERROR("ObjectFunctionClass: generic process call of function" << getName(true) << ", Call of Type HComplex not defined");};
void ObjectFunctionClass::process_S(F_PARAMETERS_T(HString) ) {ERROR("ObjectFunctionClass: generic process call of function" << getName(true) << ", Call of Type HString not defined");};

//These methods can be set by the programmer to initialize parametes
//at the beginning or free allocated memory at destruction time
void ObjectFunctionClass::setParameters(){};
void ObjectFunctionClass::startup(){};
void ObjectFunctionClass::cleanup(){};
void ObjectFunctionClass::process_end(){
  DBG("setLinkpathChanged(false): " << data_pointer->getName(true) << ", funcname=" << getName(true));	      
  data_pointer->setLinkpathChanged(false);	      
};


//This is a dummy function we need in order to fool the compiler
//so that templated methods of all typese are created ...
//This calls all templated methods in the class
//I currently don't know of a smater way doing this, after all the actual typing
//is only done at run time and by using void pointers, there is no other way
//the compiler knows which methods to create.
template <class T>
void ObjectFunctionClass::instantiate_one(){
  T val; HString s = "";
  setParameter(s,val);
  getParameterDefault<T>(s);
  putResult("",val);
}; 
//This ensures all templates for all known types are created
//The user could change this and only create selected ones.
void ObjectFunctionClass::instantiate(){
  DATATYPE* type;
  switch (*type) {
#define SW_TYPE_COMM(EXT,TYPE) instantiate_one<TYPE>();
#include "switch-type.cc"
  };
}

/*!  Function to set names and default values of parameters used in the
function. It is also possible to set a default value in case that object is
not found.  The external names are names of other objects in the network that
contain the parameter values. If not given the object name is the parameter
name plus a prefix (either ' or ":").

I have to use explicit overloading here
*/

void ObjectFunctionClass::setParameter(HString internal_name, HPointer default_value, HString prefix, HString external_name){setParameterT(internal_name, default_value, prefix, external_name);};
void ObjectFunctionClass::setParameter(HString internal_name, HInteger default_value, HString prefix, HString external_name){setParameterT(internal_name, default_value, prefix, external_name);};
void ObjectFunctionClass::setParameter(HString internal_name, HNumber default_value, HString prefix, HString external_name){setParameterT(internal_name, default_value, prefix, external_name);};
void ObjectFunctionClass::setParameter(HString internal_name, HComplex default_value, HString prefix, HString external_name){setParameterT(internal_name, default_value, prefix, external_name);};
void ObjectFunctionClass::setParameter(HString internal_name, HString default_value, HString prefix, HString external_name){setParameterT(internal_name, default_value, prefix, external_name);};


template <class T>  
void ObjectFunctionClass::setParameterT(HString internal_name, T default_value, HString prefix, HString external_name){
  DATATYPE type=WhichType<T>();
  HString ext_name;
  if (external_name=="") {ext_name=internal_name;} else {ext_name=external_name;};
  ext_name=prefix+ext_name;
  pit=parameter_list.find(internal_name);
  parameter_item p_i;
  DBG("ObjectFunctionClass::setParameter: internal_name=" << internal_name 
      << ", ext_name=" << ext_name << ", default_value=" << default_value 
      << " Type=" << datatype_txt(type));
  if (pit != parameter_list.end()) {  //Name already exists
    if ((pit->second).type==type) {  // and type is the same
      *(static_cast<T*>(p_i.ptr))=default_value; //just assign a new value
      return;
    }
    else { //need to delete old value first to make room for a new one of different type
      del_value(p_i.ptr,type);
    };
  };

  p_i.name=ext_name;
  p_i.type=type;
  p_i.ptr=new T;
  set_ptr_to_value(p_i.ptr,p_i.type,default_value);
  parameter_list[internal_name]=p_i;
}

/*!
\brief Retrieves all parameters from the network and create them, if they do not exist. 

In this form it is only useful at the time of initialization. Since
the value is not stored locally yet.

 */
void ObjectFunctionClass::getParameters(){
  pit=parameter_list.begin();
  SaveCall(data_pointer){
    while (pit!=parameter_list.end()){
      switch (pit->second.type) {
#define SW_TYPE_COMM(EXT,TYPE)						\
	getParameter(pit->second.name,*(static_cast<TYPE*>(pit->second.ptr)));
#include "switch-type.cc"
	ERROR("getParameters: Undefined Datatype encountered while retrieving vector." << " Object name=" << data_pointer->getName(true));
      };
      pit++;
    };
  };
}

HString ObjectFunctionClass::getParametersObjectName(){ 
  return "'Parameters="+data_pointer->getName();
}

HString ObjectFunctionClass::getResultsObjectName(){ 
  return ":Results="+data_pointer->getName();
}


/*! 
\brief Find the object named "Parameters", which is linked to all
objects containing parameters of the current (function) object. Create
it, if not found.
 */
Data* ObjectFunctionClass::getParametersObject(){
  DataList pobjs=data_pointer->Find(getParametersObjectName());
  if (pobjs.size()==0) {
    Data* pobj;
    pobj=data_pointer->newObject("'Parameters",data_pointer->getDefaultDirection());
    pobj->setNetLevel(999);
    vector<HString> parlist=getParameterList(data_pointer->getName());
    pobj->put_silent(parlist);
    return pobj;
  } else {
    return pobjs[0];
  };
}

/*!  Find the object named "Result", which is linked to all
objects containing results from the current (function) object. Create
it, if not found.
 */
Data* ObjectFunctionClass::getResultsObject(){
  DataList objs=data_pointer->Find(getResultsObjectName());
  if (objs.size()==0) {
    Data* obj;
    obj=data_pointer->newObject("Results");
    obj->setNetLevel(999);
    obj->putOne_silent(data_pointer->getName());
    return obj;
  } else {
    return objs[0];
  };
}

/*!
\brief Find (and create if necessary) an object containing the
parameter "name" for an operation.

 First search for an object connected to a Parameters (note Plural)
object, i.e. an object of name "Parameter" which by default is linked to
all Parameter (note Singuar) objects.

To improve efficiency, the pointer to the data objects are
stored. Hence the second time the function is called one need not
search for the object again. This happens only, when the network path
was changed (link changed or object deleted/added).

THIS ONLY WOKRS FOR PUSH CONNECTIONS - A VERSION FOR PULL CONNECTIONS
IS NOT YET IMPLEMENTED.
*/

Data* ObjectFunctionClass::getParameterObject(HString name){
  if (data_pointer->getLinkpathChanged()) {
    DBG("getParameter(" << data_pointer->getName(true) <<"): Searching object " << name);
    Data* pobj=getParametersObject(); //will be  created if it does not exist
    DataList objs=pobj->Find("'"+name); // First search Object linked to Parameter object
    if (objs.size()==0) {
      objs=data_pointer->Find("'"+name); // Search entire net
      if (objs.size()==0) { // Still not found? Then create it.
	objs.push_back(pobj->newObject(name,data_pointer->getDefaultDirection()));
	objs[0]->setNetLevel(100);
      };
    };
    setParameterObjectPointer(name,objs[0]);
    return objs[0];
  } else {
    Data * obj=getParameterObjectPointer(name);
    if (!isDataObject(obj)){
      ERROR("getParameter(" << data_pointer->getName(true) <<"): Pointer to parameter " << name << " was not properly cached. System error.");
      data_pointer->setLinkpathChanged(true);
      obj=getParameterObject(name);
    };
    return obj;
  }
} 

/*!

\brief Retrieves the pointer to a parameter object (used for temporary
cashing of the object pointer) - assumes that the network structure
has not changed, since the last setParametersObjectPointer operation.

 */


Data* ObjectFunctionClass::getParameterObjectPointer(HString name){
  ppit=parameter_pointer.find(name);
  if (ppit==parameter_pointer.end()) return &NullObject;
  else return ppit->second;
}

/*!

\brief Stores the pointer to a parameter object for temporary cashing
until the network has changed again

 */
void ObjectFunctionClass::setParameterObjectPointer(HString name, Data* ptr){ parameter_pointer[name]=ptr;}

/*!
\brief Retrieve the corresponding parameter object (to name) and return its value. If object does not exist, create it and return and assign the default value
*/

template <class T>
T ObjectFunctionClass::getParameter(const HString name, const T defval){
  Data * obj=getParameterObject(name);
  if (obj->Empty()) {
    obj->noMod()->putOne(defval);
    return defval;
  } else {
    return obj->getOne<T>();
  };
}

template <class T>
T ObjectFunctionClass::putParameter(const HString name, const T val){
  Data * obj=getParameterObject(name);
  obj->noMod()->putOne(val);
}


/*!
\brief Find (and create if necessary) an object containing the result of an operation.
 */
Data* ObjectFunctionClass::getResultObject(HString name){
  Data* robj=getResultsObject(); //will be  created if it does not exist
  DataList objs=robj->Find("'"+name); // First search Object linked to Results object
  if (objs.size()==0) {
    Data* obj=data_pointer->insertNew(name,robj);
    obj->setNetLevel(100);
    return obj;}
  else {
    return objs[0];
  };
}

/*!

\brief Stores the result of an operation in a result object, which will be created if necessary.

This method will find create a result object (if necessary) and attach
to it an object containing output of the operation from the currently
active data object. Will be called at the end of a Data object
function call.

 */
template <class T>  
Data* ObjectFunctionClass::putVecResult(HString name,vector<T> vec){
  Data* robj=getResultObject(name);
  robj->put_silent(vec);
  return data_pointer;
}

template <class T>  
Data* ObjectFunctionClass::putResult(HString name,T val){
  Data* robj=getResultObject(name);
  robj->putOne_silent(val);
  return data_pointer;
}



/*!
Returns the external name (i.e. the name of the object to be accessed) of an internal parameter
*/
HString ObjectFunctionClass::getParameterName(HString internal_name) {
  map<HString,parameter_item>::iterator it;
  DBG("ObjectFunctionClass::getParameterName: internal_name=" << internal_name);
  it=parameter_list.find(internal_name);

  if (it != parameter_list.end()) {
    return (it->second).name;
  } else {
    ERROR("ObjectFunctionClass::getParameterName: Name " << internal_name << " not found. Error in programming network function.");
    return "";
  };
}

/*!
Returns a string vector containing all defined parameters (i.e., return the external names). The string parameter first_element can be inserted as the first element.
*/

vector<HString> ObjectFunctionClass::getParameterList(HString first_element) {
  DBG("getParameterList: start.");
  map<HString,parameter_item>::iterator it;
  it=parameter_list.begin();
  vector<HString> vec;
  if (first_element!="") vec.push_back(first_element);
  while (it != parameter_list.end()) {
    vec.push_back((it->second).name);
    it++;
  };
  return vec;
}


/*!
Returns the default value of an internal parameter, typically used  if the external name (object) does not exist. Can also be used as a static variable of an object.
*/
template <class T>
T ObjectFunctionClass::getParameterDefault(HString internal_name) {
  map<HString,parameter_item>::iterator it;
  it=parameter_list.find(internal_name);
  if (it != parameter_list.end()) {
    DBG("getParameterDefault: Type=" << datatype_txt(WhichType<T>()));
    DBG("getParameterDefault: type=" << datatype_txt((it->second).type));
    DBG("getParameterDefault: ptr=" << (it->second).ptr);
    DBG("getParameterDefault: cast<Int>=" << cast_ptr_to_value<HInteger>((it->second).ptr,(it->second).type));
    //.ptr contains a pointer to a location storing the default value.
    //cast_ptr_to_value retrieves that value under the assumption it is of type .type
    T ret=cast_ptr_to_value<T>((it->second).ptr,(it->second).type);
    DBG("ret=" << ret);
    return ret;
  } else {
    ERROR("ObjectFunctionClass::getParameterDefault: Name " << internal_name << " not found. Error in programming of a network function.");
    return mycast<T>(0);
  };
}

ObjectFunctionClass::ObjectFunctionClass(Data * dp){
  DBG("ObjectFunctionClass: Constructor");
  data_pointer=dp;
};

ObjectFunctionClass::~ObjectFunctionClass(){
  map<HString,parameter_item>::iterator it; //Now destroy all the parameter default variables, created with setParameter
  it = parameter_list.begin();
  while (it !=parameter_list.end()){
    DBG("~ObjectFunctionClass(): delete parameter " << (it->second).name);
    del_value ((it->second).ptr,(it->second).type);
    it++;
  };
};


/*========================================================================
  class DataFuncDescriptor
  ========================================================================

This class stores informations about the functions that are being used
in an object. The Desriptor is then stored in the library (with empty
pointer) and in each object, but with a pointer to the actual instance
of the class.

*/

DataFuncDescriptor::DataFuncDescriptor(){setType(UNDEF);}

DataFuncDescriptor::~DataFuncDescriptor(){}

//Save the basic information about the function class (name, library it belongs to, and a short description)
void DataFuncDescriptor::setInfo(HString name, HString library, HString shortdocstring, HString docstring){
  DBG("DataFuncDescriptor.set: name=" << name << " library=" << library);
  fd.name=name;
  fd.library=library;
  fd.docstring=docstring;
  fd.shortdocstring=shortdocstring;
}

/*
Used to describe the default behaviour during creation of function. Is a vector buffer present? And what is its type
*/
void DataFuncDescriptor::setType(DATATYPE typ){fd.deftype=typ;}
bool DataFuncDescriptor::getBuffered(){return fd.buffered;}
void DataFuncDescriptor::setBuffered(bool buf){fd.buffered=buf;}

/*!
Creates a new instance of the class (on the heap) and returns a
  function descriptor, which also contains the pointer to the new
  instance. This instance will be called by "get" later on.
*/

ObjectFunctionClass* DataFuncDescriptor::newFunction(Data *dp, DATATYPE typ){
  DataFuncDescriptor fdnew;
  DATATYPE newtype=typ;
  
  //Call the Constructor of that function - defined with
  //DATAFUNC_CONSTRUCTOR (this will actually create the method and
  //hence call its constructor.
  fdnew=(*getConstructor())(dp);
  fdnew.getFunction()->setInfo(fdnew.getName(false),fdnew.getLibrary(),
			       fdnew.getDocstring(true),fdnew.getDocstring(false));
  fdnew.getFunction()->setConstructor(fdnew.getConstructor());
  fdnew.getFunction()->setFunction(fdnew.getFunction());
  //Create a new buffer if that is the default behaviour
  if (newtype==UNDEF) {newtype=fdnew.getType();};
  if (fdnew.getBuffered() && !(dp->hasData())) {
    dp->newVector(newtype);
  } else {
    dp->setType(newtype);
  };
  return fdnew.getFunction();
}
    
//returns the function to call
void DataFuncDescriptor::setFunction(ObjectFunctionClass* f_ptr){fd.f_ptr=f_ptr;}
ObjectFunctionClass* DataFuncDescriptor::getFunction(){return fd.f_ptr;}

//This is a pointer to an ordinary function, that puts the new class on the heap (constructs it)
//and returns a pointer to the instance of the class (which is retrieved by a call to f_ptr)
void DataFuncDescriptor::setConstructor(ConstructorFunctionPointer ptr){fd.constructor=ptr;}
ConstructorFunctionPointer DataFuncDescriptor::getConstructor(){return fd.constructor;}

//Return the library name the function belongs to
HString DataFuncDescriptor::getLibrary(){return fd.library;};

//Return the default type of the function 
DATATYPE DataFuncDescriptor::getType(){return fd.deftype;};

//Return the name of the function being described 
//(either with (fullname=false) or without the library name attached)
HString DataFuncDescriptor::getName(bool fullname){
  HString s;
  if (fullname){
    s= fd.library + ":" + fd.name;
  } else {
    s= fd.name;
  };
  DBG("DataFuncDescriptor.getName = " << s);
  return s;
}

//Retrieve the long or short description of the function
HString DataFuncDescriptor::getDocstring(bool shortdoc){
  if (shortdoc) {return fd.shortdocstring;}
  else {return fd.docstring;};
}


/*========================================================================
  class DataFuncLibraryClass 
  ========================================================================*/
/*
This class stores Libraries of pointers to function classes that an object can use.
*/


DataFuncLibraryClass::DataFuncLibraryClass(){
  DBG("FuncLibrary: Constructor");
  DBG("FuncLibrary: size=" << func_library.size());
}

DataFuncLibraryClass::~DataFuncLibraryClass(){}

/* 
Here provide the library with a pointer to the function constructing the object. 
The constructor function will be called with construct=false, which means that
only the Function Descriptor is returned, but no actual instance is created. 

When a function is later assigned to an object, then the call to the
constructor fucntion stored in the libray with construct=true is made, 
an instance is created on the heap, and the Function Descriptor will contain 
an actual pointer to the class.
*/
void DataFuncLibraryClass::add(ConstructorFunctionPointer c_ptr){
  DBG ("FuncLibrary: add");
  DataFuncDescriptor fd=(*c_ptr)(false);
  HString fname=fd.getName(true);
  DBG ("FuncLibrary: add, name=" << fname);
  func_library[fname]=fd;
  DBG("FuncLibrary: size=" << func_library.size());
}

//Retrieves the pointer to the function class
ObjectFunctionClass* DataFuncLibraryClass::f_ptr(HString name, HString library){
  it=func_library.find(library+":"+name);
  if (it !=func_library.end()) {
    return (it->second).getFunction();
  } else {
    return NULL;
  };
}

//Retrieves the struct describing the function class 
DataFuncDescriptor* DataFuncLibraryClass::FuncDescriptor(HString name, HString library){
  it=func_library.find(library+":"+name);
  if (it !=func_library.end()) {
    return &(it->second);
  } else {
    return NULL;
  };
}

//Checks whether a function is in the library or not
bool DataFuncLibraryClass::inLibrary(HString name, HString library){
  it=func_library.find(library+":"+name);
  return (it !=func_library.end());
}

//prints a directory of all functions in the Library
void DataFuncLibraryClass::dir(){
  for (it=func_library.begin();it!=func_library.end();it++) {
    MSG((it->second).getName(true) << " - " << (it->second).getDocstring()); 
  };
}

//End DataFuncLibraryClass 
//........................................................................


/*
========================================================================
  Math functions
========================================================================

These are needed to deal with mathematical operations on strings (and
vectors).

They will be called by the process method. Maybe it would be possible
to generate the associated .process method automatically by a nawk
preprocessor step.


*/

//!!!This should be automized by a #define command or nawk preprocesser
//otherwise it becomes a nightmare. So, let's not add too many 
//math functions at this point.

#ifndef HF_MATH_FUNC
#undef HF_MATH_FUNC
#endif
#ifndef HF_MATH_FUNC2
#undef HF_MATH_FUNC2
#endif

#define HF_MATH_FUNC( FUNC ) \
template<>        inline HString FUNC<HString>(const HString v){return mycast<HString>(FUNC(mycast<HComplex>(v)));};\
template<>        inline HPointer FUNC<HPointer>(const HPointer v){return v;};\
template<class T> inline T FUNC(const T v)

#define HF_MATH_FUNC2( FUNC )						\
  inline HString FUNC(const HString v1, const HString v2){return mycast<HString>(FUNC(mycast<HComplex>(v1),mycast<HComplex>(v2)));}; \
  template<class S>        inline HString FUNC(const HString v1, const S v2){return mycast<HString>(FUNC(mycast<S>(v1),v2));}; \
  template<class S>        inline HString FUNC(const S v1, const HString v2){return mycast<HString>(FUNC(v1,mycast<S>(v2)));}; \
  inline HPointer FUNC(const HPointer v1, const HPointer v2){return mycast<HPointer>(FUNC(mycast<HInteger>(v1),mycast<HInteger>(v2)));}; \
  template<class S>        inline HPointer FUNC(const HPointer v1, const S v2){return mycast<HPointer>(FUNC(mycast<S>(v1),v2));}; \
  template<class S>        inline HPointer FUNC(const S v1, const HPointer v2){return mycast<HPointer>(FUNC(v1,mycast<S>(v2)));}; \
  template<class T, class S> inline T FUNC(const T v1,const S v2)	

//---square
//template<class T> inline T hf_square(const T v)
HF_MATH_FUNC(hf_Sqrt){return sqrt(v);};
HF_MATH_FUNC(hf_Square){return v*v;}; 
HF_MATH_FUNC2(hf_Sub){return v1-v2;};
HF_MATH_FUNC2(hf_Mul){return v1*v2;}; 
HF_MATH_FUNC2(hf_Add){return v1+v2;}; 
HF_MATH_FUNC2(hf_Div){return v1/v2;}; 
HF_MATH_FUNC2(hf_Pow){return pow(v1,v2);}; 


//template<>        inline HString hf_square<HString>(const HString v){return mycast<HString>(hf_square(mycast<HNumber>(v)));};
//template<>        inline HPointer hf_square<HPointer>(const HPointer v){return v;};


//---sqrt - square root
//template<class T> inline T hf_sqrt(const T v)
//# REPEAT_INLINE_TEMPLATE sqrt 

//template<>        inline HString hf_sqrt<HString>(const HString v){return mycast<HString>(hf_sqrt(mycast<HNumber>(v)));};
//template<>        inline HPointer hf_sqrt<HPointer>(const HPointer v){return v;};


//--End Math functions-----------------------------------------------------


/*========================================================================
  Define DataFunc Object Library "Sys"
  ========================================================================

This contains a library of some basic functions that can be assigned
to objects for exectuion in the network.

There are a few special macros used here:

DEFINE_PROCESS_CALLS: this defines the virtual methods process_X,
which will call the non-virtual put templated method process<T>. Use
F_PARAMETERS to define the function variable list.

DEFINE_PROCESS_CALLS_NUMONLY: only defines the calls for numeric
datatypes, allowing one to do an explicit specialization for Strings
and Pointers (See Sys_Range for an example).

DEFINE_PROCESS_CALLS_IGNORE_DATATYPE: this defines the virtual methods
process_X, which will call the non-virtual non-templated method
process. Use F_PARAMETERS-NOVEC to define the function variable
list. Use this, if the function does not perform any operation on the
data vector in the object.

DATAFUNC_CONSTRUCTOR(Name,Library,Doc): is a macro that needs to come
AFTER every new class definiton to generate a constructor function
that generates a description of the function for the library AND
constructs an instance of the class when it is assigned to an object.

In addition the class needs to be "added" to the library in the
"DataFunc_XXX_Library_publish" function. Ideally the macro would do
this automatically, but it doesn't yet ...

Also, the "DataFunc_XXX_Library_publish" functions needs to be called
by the constructor of the data function - also not yet an ideal
situation. I'd prefer a dynamic solution.

A somewhat smarter preprocesser (like an (n)awk-script) could probably
do that easily.
*/

//------------------------------------------------------------------------------
//$DEFINE PREPROCESSOR
/*------------------------------------------------------------------------------
$INDEX: Function Lib Name
$BEGIN: Function class DataFunc_$Lib_$Name : public ObjectFunctionClass {	\
public:\
DEFINE_PROCESS_CALLS\
 DataFunc_$Lib_$Name (Data* dp) : ObjectFunctionClass(dp){	\
   dp->setUpdateable($updateable);		\
    setParameters();\
    startup();\
    getParameters();\
    }\
 ~DataFunc_$Lib_$Name(){cleanup(); } \
 \
void setParameters(){\
SET_FUNC_PARAMETER_AWK($*Par);\
};\
 \
template <class T> void process(F_PARAMETERS) {\  
  GET_FUNC_PARAMETER_AWK($*Par);

$END: Function }; DATAFUNC_CONSTRUCTOR($Name,$Lib,"$Info",$Type,$buffered);
$PUBLISH: Function PUBLISH_OBJECT_FUNCTION($Lib,$Name); 
------------------------------------------------------------------------------*/


//------------------------------------------------------------------------------
//$NEW: Function
/*------------------------------------------------------------------------------
Lib: Sys
Name: Unit
Info: Multiplies the data with a unit scale factor and also returns the apropriate unit string.
Type: NUMBER
buffered: false
updateable: false
Par: UnitName, HString, ""
Par: UnitPrefix, HString,""
Par: UnitScaleFactor, HNumber, 1.0
------------------------------------------------------------------------------*/
class DataFunc_Sys_Unit : public ObjectFunctionClass { 
public:
DEFINE_PROCESS_CALLS
 DataFunc_Sys_Unit (Data* dp) : ObjectFunctionClass(dp){	
   dp->setUpdateable(false);		
    setParameters();
    startup();
    getParameters();
    }
 ~DataFunc_Sys_Unit(){cleanup(); } 
 
void setParameters(){
SET_FUNC_PARAMETER_AWK(UnitName, HString, "");
SET_FUNC_PARAMETER_AWK(UnitPrefix, HString,"");
SET_FUNC_PARAMETER_AWK(UnitScaleFactor, HNumber, 1.0);
};
 
template <class T> void process(F_PARAMETERS) {
  GET_FUNC_PARAMETER_AWK(UnitName, HString, "");
  GET_FUNC_PARAMETER_AWK(UnitPrefix, HString,"");
  GET_FUNC_PARAMETER_AWK(UnitScaleFactor, HNumber, 1.0);
  dp->getFirstFromVector(*vp,vs);
  INIT_FUNC_ITERATORS(it,end);
  while (it!=end) {*it=hf_Div(*it,UnitScaleFactor);it++;};
}
//------------------------------------------------------------------------------
//$END Function
}; DATAFUNC_CONSTRUCTOR(Unit,Sys,"Multiplies the data with a unit scale factor and also returns the apropriate unit string.",NUMBER,false);
//------------------------------------------------------------------------------


class DataFunc_Sys_Copy : public ObjectFunctionClass {
public:  
  DEFINE_PROCESS_CALLS 

  DataFunc_Sys_Copy(Data*dp){dp->setUpdateable(false);};
  
  template <class T>
  void process(F_PARAMETERS) {
    (*dp).getFirstFromVector(*vp,vs);
    //This is bad - better use the setParameter method to also allow other than the first object to be accessed ...
    if (vs != NULL) {(*vp) = (*vs).get(*vp);};
  }
};
DATAFUNC_CONSTRUCTOR(Copy,Sys,"Copies the content of one object to the next.",INTEGER, false)

class DataFunc_Sys_Neighbours : public ObjectFunctionClass {
public:  
  DEFINE_PROCESS_CALLS 
  
  DataFunc_Sys_Neighbours(Data*dp){dp->setUpdateable(true);};

  template <class T>
  void process(F_PARAMETERS) {
    vector<HString> vec;
    vec=dp->getNeighbourNames(DIR_TO);
    vec.push_back("");
    vec_append(vec,dp->getNeighbourNames(DIR_FROM));
    copycast_vec<T,HString>(vp,&vec);
    if (vs != NULL) {(*vp) = (*vs).get(*vp);};
  }
};

DATAFUNC_CONSTRUCTOR(Neighbours,Sys,"Returns a list of names of all neighbour objects, first in To direction and the in FROM direction separated by an empty string.",STRING, true)
//Name, Library, Description, Default Type, Buffered or not


    /*

    !!!Actually the treatment of vector selection here and in the
    following is wrong/inefficient!!
    
    The function will only be called by get with a vector selector
    that is independent of the value (the vs1,vs2 split is already
    done within get), so selection could be done before calculation
    (and hence reduce processing time).

    getFirstFromVector: This is bad - better use the setParameter method to also allow other than the first object to be accessed ...
    
    */
 

#define HF_MATH_DATAFUNC(NAME,DOC,TYP) \
class DataFunc_Sys_##NAME : public ObjectFunctionClass {\
public:\
  DEFINE_PROCESS_CALLS \
  DataFunc_Sys_##NAME(Data*dp){dp->setUpdateable(false);};\
  template <class T>\
  void process(F_PARAMETERS) {\
    address i,size;\
    (*dp).getFirstFromVector(*vp,vs);\
    size=(*vp).size();\
    for (i=0; i<size; i++) {(*vp)[i]=hf_##NAME((*vp)[i]);};\
  }\
};\
DATAFUNC_CONSTRUCTOR(NAME,Sys,DOC,TYP, false)

HF_MATH_DATAFUNC(Sqrt,"Takes the square root of the first object vector connected to the object.",NUMBER)
HF_MATH_DATAFUNC(Square,"Takes the square root of the first object vector connected to the object.",NUMBER)



//------------------------------------------------------------------------------
//$NEW: Function
/*------------------------------------------------------------------------------
Lib: Sys
Name: Offset
Info: Subtracts the first elements in the data vector from the entire data vector.
Type: NUMBER
buffered: false
updateable: false
Par: OffsetValue, HNumber, 0.0
Par: OffsetFixed, HInteger, int(false)
------------------------------------------------------------------------------*/
class DataFunc_Sys_Offset : public ObjectFunctionClass { 
public:
DEFINE_PROCESS_CALLS
 DataFunc_Sys_Offset (Data* dp) : ObjectFunctionClass(dp){	
   dp->setUpdateable(false);		
    setParameters();
    startup();
    getParameters();
    }
 ~DataFunc_Sys_Offset(){cleanup(); } 
 
void setParameters(){
SET_FUNC_PARAMETER_AWK(OffsetValue, HNumber, 0.0);
SET_FUNC_PARAMETER_AWK(OffsetFixed, HInteger, int(false));
};
 
template <class T> void process(F_PARAMETERS) {
  GET_FUNC_PARAMETER_AWK(OffsetValue, HNumber, 0.0);
  GET_FUNC_PARAMETER_AWK(OffsetFixed, HInteger, int(false));
// The following line with $$ is a comment for the preprocessor, but
// the bracket makes life easier for the editor since we have to add
// one at the end of this block as well
dp->getFirstFromVector(*vp,vs);

 INIT_FUNC_ITERATORS(it,end);

T off;
if (bool(OffsetFixed)) {
  off=mycast<T>(OffsetValue);
 } else {
  off=*it;
  putParameter("OffsetValue", off);
 };

while (it!=end) {
  *it=hf_Sub(*it,off);
  it++;
 };
}

//------------------------------------------------------------------------------
//$END Function
}; DATAFUNC_CONSTRUCTOR(Offset,Sys,"Subtracts the first elements in the data vector from the entire data vector.",NUMBER,false);
//------------------------------------------------------------------------------



class DataFunc_Sys_Print : public ObjectFunctionClass {
public:

  DEFINE_PROCESS_CALLS 

  DataFunc_Sys_Print(Data*dp){};

  //Needs treatment of vector selector ...
  template <class T>
  void process(F_PARAMETERS) {
    DBG("DataFunc_Sys_Print: process, data object pointer dp=" << dp);
    (*dp).getFirstFromVector(*vp,vs);    //copy data vector from predecessor 
    //This is bad - better use the setParameter method to also allow other than the first object to be accessed ...
    cout << (*dp).getName() << ": ";  //and print
    printvec(*vp);
  }
};
DATAFUNC_CONSTRUCTOR(Print,Sys,"Prints contents of data vector to stdout.",STRING, false);



class DataFunc_Sys_Range : public ObjectFunctionClass {
public:

  DEFINE_PROCESS_CALLS_NUMONLY

  //  Define your own constructor to set the default parameters for the function
  DataFunc_Sys_Range (Data * dp){
    //set the default parameters, which are by default integers - perhaps change to float numbers later??
    DBG("DataFunc_Sys_Range: initialization called.");
    SET_FUNC_PARAMETER(start, HInteger, 0);
    SET_FUNC_PARAMETER(end, HInteger, 0);
    SET_FUNC_PARAMETER(inc, HInteger, 1);
  }

  template <class T>
  void process(F_PARAMETERS) {
    HInteger i,size;

    //This macro will define and obtain set local variables with
    //values from corresponding parameter objects (set by
    //setParameter) or their default values, if they are not
    //present. See init for those values. If one wants a
    //specific/fixed datatype then use GET_FUNC_PARAMETER_T(NAME,TYPE)
    //instead. This may only be called once in the execution of a
    //function.

    GET_FUNC_PARAMETER(start,T);
    GET_FUNC_PARAMETER(end,T);
    GET_FUNC_PARAMETER(inc,T);
    if (inc==static_cast<T>(0)) {
      ERROR("DataFunc_Sys_Range: Increment inc=0!");
      return;
    }
    //Create the data vector to be returned
    vp->clear(); 
    size=floor(abs((end-start)/inc))+1;
    if (vp->capacity()<size) {vp->reserve(size);};

    //Fill the return data vector and make sure it is casted to the right output type
    for (i=0;i<size;i++) {vp->push_back(static_cast<T>(i)*inc+start);};
    
    //Perform a selection on the data vector, if necessary.  This
    //could be done smarter for some selections, i.e., by splitting
    //the vector selector into vs1 and vs2 as in Sys_Square ....
    if (vs!=NULL) {(*vp)=vs->get(*vp);};
  }

  //Now we have to do some specialization for non-numeric datataypes.
  //Again, this is not really nice, if we ever need to add an
  //additional, non-numeric type. However, that could/should also be
  //done with a macro once we have more than one of these cases!!  I
  //tried this with an explicit specialization of the method process,
  //using template<>, but the compiler didn't like that for some
  //reasons.
  void process_S(F_PARAMETERS_T(HString)) {
    vector<HNumber> vn;
    process<HNumber>(&vn, F_PARAMETERS_CALL_NOVEC);
    copycast_vec<HNumber,HString>(&vn,vp);
  }
};
DATAFUNC_CONSTRUCTOR(Range,Sys,"Return a range of numbers (0,1,2,3,4,...N). Parameter objects: start=0, end=0, inc=1",INTEGER, false);


//------------------------------------------------------------------------
//End Sys Library
//------------------------------------------------------------------------


/*========================================================================
  Define DataFunc Object Library "CR"
  ========================================================================

  This library allows one to access the CR IO and analysis functions

*/


//------------------------------------------------------------------------------
//$NEW: Function
/*------------------------------------------------------------------------------
Lib: CR
Name: dataReaderObject
Info: Creates a DataReader object for reading CR data and stores its pointer.
Type: POINTER
buffered: true
updateable: true
Par: Filename, HString, dataset_lopes
------------------------------------------------------------------------------*/
class DataFunc_CR_dataReaderObject : public ObjectFunctionClass { 
public:
DEFINE_PROCESS_CALLS
 DataFunc_CR_dataReaderObject (Data* dp) : ObjectFunctionClass(dp){	
   dp->setUpdateable(true);		
    setParameters();
    startup();
    getParameters();
    }
 ~DataFunc_CR_dataReaderObject(){cleanup(); } 
 
void setParameters(){
SET_FUNC_PARAMETER_AWK(Filename, HString, dataset_lopes);
};
 
template <class T> void process(F_PARAMETERS) {
  GET_FUNC_PARAMETER_AWK(Filename, HString, dataset_lopes);

HString Filetype = determine_filetype(Filename);
  
static HString oldfilename="";
bool opened;
union{void* ptr; CR::DataReader* drp; CR::LOFAR_TBB* tbb; CR::LopesEventIn* lep;};
  
  DBG("dataReaderObject: Retrieved filename parameter =" << Filename); // gets filename from object set in hfnet.py

  // If a pointer to a DataReader was stored already in the data vector, delete that c++ object first
  //before we create a new DataReader below (and store the Pointer in the data vector)
  if (vp->size()>0 && AsPtr(vp->at(0))!=NULL) {
    DBG("dataReaderObject: Delete old data reader object " << AsPtr(vp->at(0))); 
    drp=reinterpret_cast<DataReader*>(AsPtr(vp->at(0)));
    delete drp;
  }
  vp->clear();
  
  //Create the a pointer to the DataReader object and store the pointer
  //Here we could have if statements depending on data types
      
  DBG("DataFunc_CR_dataReaderObject: Opening File, Filename=" << Filename);
  if (Filetype=="LOPESEvent") {
    lep = new CR::LopesEventIn;
    DBG("DataFunc_CR_dataReaderObject: lep=" << ptr << " = " << reinterpret_cast<HInteger>(ptr));
    opened=lep->attachFile(Filename);
    if (oldfilename!=Filename) {MSG("Filename="<<Filename);lep->summary();};
    oldfilename=Filename;
  } else if (Filetype=="LOFAR_TBB") {
    tbb = new CR::LOFAR_TBB(Filename,32768);
    MSG("ATTENTION: Hardcoded initial NBlocksize to 1024!");
    DBG("DataFunc_CR_dataReaderObject: tbb=" << ptr << " = " << reinterpret_cast<HInteger>(ptr));
    opened=tbb!=NULL;
    if (oldfilename!=Filename) {MSG("Filename="<<Filename);tbb->summary();};
    oldfilename=Filename;
  } else {
    ERROR("DataFunc_CR_dataReaderObject: Unknown Filetype = " << Filetype  << ", name=" << dp->getName(true));
    opened=false;
  }

  putResult("Filetype",Filetype);

  if (!opened){
    ERROR("DataFunc_CR_dataReaderObject: Opening file " << Filename << " failed." << " Objectname=" << dp->getName(true));
    vp->push_back(mycast<T>(reinterpret_cast<HPointer>(Null_p))); 
    MSG("vp->size="<<vp->size()<<", v(0)="<<(*vp)[0]);
    return;
  };

    //Store the pointer in the object, so that other objects can access
    //it. The object should actually me made read-only, since the pointer is
    //not to be changed by put ever again until the window is deleted
  vp->push_back(mycast<T>(ptr)); 

      //Read the data Header, containing important information about the file (e.g. size)
  casa::Record hdr=drp->headerRecord();
      
      //      uint nfields=hdr.nfields();
      //      uint i;
      //      for (i=0; i<nfields; i++) {
      //	MSG("hdr name="<<hdr.name(i) << " type="<<hdr.dataType(i));
      //      };

      //Now store File and Header information in Result objects
  HInteger date=hdr.asuInt("Date"); putResult("Date",date);
  HString observatory=hdr.asString("Observatory"); putResult("Observatory",observatory);
  HInteger filesize=hdr.asInt("Filesize"); putResult("Filesize",filesize);
  vector<HInteger> AntennaIDs; hdr.asArrayInt("AntennaIDs").tovector(AntennaIDs); putVecResult("AntennaIDs",AntennaIDs);
  HInteger nofAntennas=drp->nofAntennas();putResult("nofAntennas",nofAntennas);
  DBG("DataFunc_CR_dataReaderObject: Success.");
}

void cleanup(){
  delete reinterpret_cast<CR::DataReader*>(data_pointer->getOne<HPointer>()); 
}

//------------------------------------------------------------------------------
//$END Function
}; DATAFUNC_CONSTRUCTOR(dataReaderObject,CR,"Creates a DataReader object for reading CR data and stores its pointer.",POINTER,true);
//------------------------------------------------------------------------------



/*!
  \brief The function converts a column in an aips++ matrix to an stl vector
 */

template <class S, class T>
void aipscol2stlvec(casa::Matrix<S> data, vector<T>& stlvec, HInteger col){
    HInteger i,nrow,ncol;
//    vector<HNumber>::iterator p;
    
    nrow=data.nrow();
    ncol=data.ncolumn();
    if (ncol>1) {
      MSG("aipscol2stlvec: ncol="<<ncol <<" (nrow="<<nrow<<")");
    };
    if (col>=ncol) {
	ERROR("aipscol2stlvec: column number col=" << col << " is larger than total number of columns (" << ncol << ") in matrix.");
	stlvec.clear();
	return;
    }

    stlvec.resize(nrow);
    casa::Vector<S> CASAVec = data.column(col);
    
//    p=stlvec.begin();
    
    for (i=0;i<nrow;i++) {
//	*p=mycast<T>(CASAVec[i]); 
	stlvec[i]=mycast<T>(CASAVec[i]); 
//	p++;
    };
}

/*!
  \brief The function converts an aips++ vector to an stl vector
 */

template <class S, class T>
void aipsvec2stlvec(casa::Vector<S> data, vector<T>& stlvec){
    HInteger i,n;
//    vector<R>::iterator p;
    
    n=data.size();
    stlvec.resize(n);
//    p=stlvec.begin();
    for (i=0;i<n;i++) {
//	*p=mycast<T>(data[i]); 
	stlvec[i]=mycast<T>(data[i]); 
//	p++;
    };
}


class DataFunc_CR_dataRead : public ObjectFunctionClass {
public:
  //The following line is necessary to define the process method for this class

  DEFINE_PROCESS_CALLS 

  //  This function reads Data from an DataReader Object to read CR data
  DataFunc_CR_dataRead (Data* dp) : ObjectFunctionClass(dp){
    DBG("DataFunc_CR_dataRead: initialization called.");

    HPointer ptr=NULL;
    SET_FUNC_PARAMETER(File, HPointer, ptr);
    SET_FUNC_PARAMETER(Antenna, HInteger, 0);
    SET_FUNC_PARAMETER(Blocksize, HInteger, -1);
    SET_FUNC_PARAMETER(Block, HInteger, 0);
    SET_FUNC_PARAMETER(maxBlocksize, HInteger, 65536);
    SET_FUNC_PARAMETER(Filesize, HInteger, -1);
    SET_FUNC_PARAMETER(Stride, HInteger, 0);
    SET_FUNC_PARAMETER(Shift, HInteger, 0);
    SET_FUNC_PARAMETER(Datatype, HString, "Fx");
    getParameters();

    //Now create a new, but empty data vector as buffer
    vector<HNumber> vec;
    dp->noMod(); dp->put(vec);     
  }

  ~DataFunc_CR_dataRead (){
    MSG("~DataFunc_CR_dataRead: Here something probably needs to happen for destroying the plotter");
  }
  
  template <class T>
  void process(F_PARAMETERS) {

    //First retrieve the pointer to the pointer to the dataRead and check whether it is non-NULL.
    GET_FUNC_PARAMETER(File,HPointer);
    DBG("File=" << File);
    if (File==NULL){
	ERROR("dataRead: pointer to FileObject is NULL, DataReader not found." << ", name=" << dp->getName(true)); 
	return;
    };
    DataReader *drp=reinterpret_cast<DataReader*>(File); 


//!!!One Needs to verify somehow that the parameters make sense !!!
    GET_FUNC_PARAMETER(Antenna, HInteger);
    GET_FUNC_PARAMETER(Blocksize,  HInteger);
	GET_FUNC_PARAMETER(maxBlocksize,  HInteger);
	GET_FUNC_PARAMETER(Filesize,  HInteger);
    GET_FUNC_PARAMETER(Block,  HInteger);
    GET_FUNC_PARAMETER(Stride,  HInteger);
    GET_FUNC_PARAMETER(Shift, HInteger);
    GET_FUNC_PARAMETER(Datatype, HString);
    
    DBG("Reading Antenna=" << Antenna);
    DBG("nofAntennas=" << drp->nofAntennas());
    if (Antenna > drp->nofAntennas()-1) {ERROR("Requested Antenna number too large!");};

    if (Blocksize<1) Blocksize=maxBlocksize;
	drp->setBlocksize(Blocksize);
    drp->setBlock(Block);
    drp->setStride(Stride);
    drp->setShift(Shift);
	HInteger maxBlock=Filesize/Blocksize-1;	
	putResult("maxBlock",maxBlock);

    Vector<uint> antennas(1,Antenna);
    drp->setSelectedAntennas(antennas);
//    Vector<uint> selantennas=drp->selectedAntennas();
//    MSG("No of Selected Antennas" << drp->nofSelectedAntennas ()<< " SelectedAntennas[0]=" <<selantennas[0]);

    if (Datatype=="Time") {aipsvec2stlvec(drp->timeValues(),*vp);}
    else if (Datatype=="Frequency") {aipsvec2stlvec(drp->frequencyValues(),*vp);}
    else if (Datatype=="Fx") {aipscol2stlvec(drp->fx(),*vp,0);}
    else if (Datatype=="Voltage") {aipscol2stlvec(drp->voltage(),*vp,0);}
    else if (Datatype=="invFFT") {aipscol2stlvec(drp->invfft(),*vp,0);}
    else if (Datatype=="FFT") {aipscol2stlvec(drp->fft(),*vp,0);}
    else if (Datatype=="CalFFT") {aipscol2stlvec(drp->calfft(),*vp,0);}
    else {
	ERROR("DataFunc_CR_dataRead: Datatype=" << Datatype << " is unknown." << ", name=" << dp->getName(true));
	vp->clear();
	return;
    };
  }
};

//The following macro has to come at the of the definiton. It defines
//a constructor function (no class) with a pointer that is called when
//an object is assigned this function.
DATAFUNC_CONSTRUCTOR(dataRead,CR,"Function retrieving a vector from the dataReader.",NUMBER, true);

/*------------------------------------------------------------------------
End DataFunc Object Library "CR"
------------------------------------------------------------------------*/


/*------------------------------------------------------------------------
Python Interface Functions
------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
// $xxxNEW: Function
/*------------------------------------------------------------------------------
Lib: Py
Name: PyFunc
------------------------------------------------------------------------------*/

class DataFunc_Py_PyFunc : public ObjectFunctionClass {
public:
  //This is necessary to define the process method for this function
  //which in this case ignores the input vector:

  DEFINE_PROCESS_CALLS_IGNORE_DATATYPE
  
  DataFunc_Py_PyFunc (Data* dp): ObjectFunctionClass(dp){
    PyObject* pyobj = dp->retrievePyFunc();

    DBG("DataFunc_Py_PyFunc: initialization called.");
    if (pyobj==NULL){
      ERROR("PyFunc: pointer to PythonObject is NULL. Object PythonObject does not exist. Define PythonObject and use pytore before assigning PyFunc to this Object." << ", name=" << dp->getName(true)); 
      return;
    };

    //Call the startup method if present
    char AttribStr[] = "hfstartup";
    //first we need to check if attribute is present in the Python Object
    if (!PyObject_HasAttrString(pyobj, AttribStr)) {
	ERROR("PyFunc: Object does not have Attribute " << AttribStr << ", name=" << dp->getName(true)); return;};
    int ret=boost::python::call_method<int>(pyobj,AttribStr,boost::ref(*dp));
    if (ret!=0) {
	ERROR("PyFunc - startup method returned user-defined error code" << ret << ", name=" << dp->getName(true));
    };
  }

    ~DataFunc_Py_PyFunc (){
      Data * dp = data_pointer;
      PyObject* pyobj = dp->retrievePyFunc();
	
	if (pyobj==NULL){ERROR("PyFunc: pointer to Python Object is NULL." << ", name=" << dp->getName(true)); return;};

	char AttribStr[] = "hfcleanup";
	//we need to check if the attribute is present in the Python Object
	if (!PyObject_HasAttrString(pyobj, AttribStr)) {
	    ERROR("PyFunc: Object does not have Attribute " << AttribStr << ", name=" << dp->getName(true) << "."); return;};
	int ret=boost::python::call_method<int>(pyobj,AttribStr);
	if (ret!=0) {
	    ERROR("PyFunc - process method returned user-defined error code" << ret  << ", name=" << dp->getName(true));
	};
    }
 

    void process(F_PARAMETERS_NOVEC){
      PyObject* pyobj = dp->retrievePyFunc();

	DBG("DataFunc_Py_PyFunc.process: pyobj=" << pyobj << " name=" << dp->getName(true));
	if (pyobj==NULL){ERROR("PyFunc: pointer to Python Object is NULL."  << ", name=" << dp->getName(true)); return;};

	char AttribStr[] = "hfprocess";
	//we need to check if the process attribute is present in the Python Object
	if (!PyObject_HasAttrString(pyobj, AttribStr)) {
	    ERROR("PyFunc: Object does not have Attribute " << AttribStr << "."  << ", name=" << dp->getName(true)); return;};
	DBG("DataFunc_Py_PyFunc.process: Call Python Object");
	int ret=boost::python::call_method<int>(pyobj,AttribStr,boost::ref(*dp));
	if (ret!=0) {
	    ERROR("PyFunc - process method returned user-defined error code" << ret << ", name=" << dp->getName(true));
	};
    }
};
//The following macro has to come at the of the definiton. It defines
//a constructor function (no class) with a pointer that is called when
//an object is assigned this function.
DATAFUNC_CONSTRUCTOR(PyFunc,Py,"Function to call a user-defined python object of type hffunc",POINTER,false);



//------------------------------------------------------------------------
//Network functions
//------------------------------------------------------------------------

/* 
These functions can be called from each object to perform certain functions
 */



/*template <class T>
static void * addvecs(data_field * d) {
  vector<T> v0,vi;
  objectid i,j;
  objectid sizei, size0, sizej;
  vector<Data*>* inp;
  inp = reinterpret_cast<vector<Data*>*>(&in_v);

  v0 = (*inp).at(0)->get<T>();
  size0 = v0.size();
  sizei=(*inp).size();
  for (i=1; i<sizei; i++) {
    vi = (*inp).at(i)->get<T>();
    sizej = vi.size();
    for (j=0; j<min(size0,sizej); j++) {
      v0[j] = v0[j] + vi[j];
    };
  };
  return v0;
}

*/


int ReadTextFile(string filename)
{
  string line,name;
  int nfield=0,i,j;
  int nline=0;
  vector<string> fieldname;
  map<string,int> fieldnum;
  vector<vector<double> > fields;


  ifstream f(filename.c_str());
  if (!f)
    {
      cout << "ERROR (ReadText): File " << filename.c_str() << " not found!";
      return 1;
    }
  else
    {cout << "Opening data file " << filename << ".\n";}

  //Read first line and parse the field names
  getline(f,line);
  istringstream sline(line);
  while (sline >> name){
    fieldname.push_back(name);
    cout << "\n" << fieldname.size() << ": " << fieldname.at(fieldname.size()-1);
    fieldnum[name]=nfield;
    nfield++;
  };
  cout << "\n";

  vector<double> row(nfield);
  nline=0;
  //Read data lines
  while (getline(f,line)) {
    istringstream sline(line);
    i=0;
    while (sline >> row[i] && i<nfield){i++;};
    fields.push_back(row);
    nline++;
  };
  f.close();
  for (i=0;i<nline;i++) {
    cout << i << ": ";
    for (j=0;j<nfield;j++) {
      cout << fields[i][j]<<" ";
        };
    cout << "\n";
  };
}

//------------------------------------------------------------------------
//Publish the Libraries - used in Data object constructor
//------------------------------------------------------------------------

void DataFunc_Library_publish(DataFuncLibraryClass* library_ptr){
  //$PUBLISH Function
PUBLISH_OBJECT_FUNCTION(Sys,Unit);
PUBLISH_OBJECT_FUNCTION(Sys,Offset);
PUBLISH_OBJECT_FUNCTION(CR,dataReaderObject);

  PUBLISH_OBJECT_FUNCTION(Sys,Neighbours);
  PUBLISH_OBJECT_FUNCTION(Sys,Copy);
  PUBLISH_OBJECT_FUNCTION(Sys,Print);
  PUBLISH_OBJECT_FUNCTION(Sys,Square);
  PUBLISH_OBJECT_FUNCTION(Sys,Sqrt);
  //PUBLISH_OBJECT_FUNCTION(Sys,Offset);
  PUBLISH_OBJECT_FUNCTION(Sys,Range);
  //  PUBLISH_OBJECT_FUNCTION(Sys,Unit);
  //  PUBLISH_OBJECT_FUNCTION(CR,dataReaderObject);
  PUBLISH_OBJECT_FUNCTION(CR,dataRead);
  PUBLISH_OBJECT_FUNCTION(Py,PyFunc);
};
 
