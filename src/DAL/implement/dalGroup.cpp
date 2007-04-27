/***************************************************************************
 *   Copyright (C) 2006 by Joseph Masters                                  *
 *   jmasters@science.uva.nl                                               *
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

#ifndef dalGROUP_H
#include "dalGroup.h"
#endif
herr_t attr_info(hid_t loc_id, const char *name, void *opdata);
dalGroup::dalGroup() {
}

dalGroup::dalGroup( void * voidfile, char* gname ) {


	hid_t * lclfile = (hid_t*)voidfile; // H5File object
	file = lclfile;
	file_id = *lclfile;  // get the file handle

	name = gname;
	string fullgroupname = "/" + stringify(gname);
	hid_t grp = H5Gcreate(*(hid_t*)file, fullgroupname.c_str(), 0);

}

bool dalGroup::setName ( string gname ) {
   if ( gname.length() > 0 )
   {
     name = gname;
     return SUCCESS;
   }
   else
   {
     cout << "Error:  Group name must not be empty." << endl;
     return FAIL;
   }
}

string dalGroup::getName () {
   return name;
}

int dalGroup::open( void * voidfile, string groupname ) {
	name = groupname;

	hid_t * lclfile = (hid_t*)voidfile; // H5File object
	file = lclfile;
	file_id = *lclfile;  // get the file handle
	
	string fullgroupname = "/" + groupname;
	group_id = H5Gopen( file_id, fullgroupname.c_str() );
	return( group_id );
}

void dalGroup::setAttribute_string( string attrname, string data ) {
	if ( H5LTset_attribute_string( file_id, name.c_str(),
					attrname.c_str(), data.c_str() ) < 0 ) {
		cout << "ERROR: could not set attribute " << attrname << endl;
	}
}

void dalGroup::setAttribute_int( string attrname, int * data, int size ) {
	if ( H5LTset_attribute_int( file_id, name.c_str(),
					attrname.c_str(), data, size ) < 0 ) {
		cout << "ERROR: could not set attribute " << attrname << endl;
	}
}

void dalGroup::setAttribute_uint( string attrname, unsigned int * data, int size ) {
	if ( H5LTset_attribute_uint( file_id, name.c_str(),
					attrname.c_str(), data, size ) < 0 ) {
		cout << "ERROR: could not set attribute " << attrname << endl;
	}
}

void dalGroup::setAttribute_double( string attrname, double * data, int size ) {
	if ( H5LTset_attribute_double( file_id, name.c_str(),
					attrname.c_str(), data, size ) < 0 ) {
		cout << "ERROR: could not set attribute " << attrname << endl;
	}
}

void dalGroup::getAttributes() {

   //status = H5Aget_num_attrs(group_id);
   //printf ("H5Aget_num_attrs returns: %i\n", status);

   status = H5Aiterate(group_id,NULL ,attr_info, NULL);
   //printf ("\nH5Aiterate returns: %i\n", status);

}

void dalGroup::printAttribute( string attrname ) {

	hsize_t * dims;
	H5T_class_t type_class;
	size_t type_size;

	// Check if attribute exists
	if ( H5LT_find_attribute(group_id, attrname.c_str()) <= 0 ) {
		cout << "Attribute " << attrname << " not found." << endl;
		return;
	}
	
	string fullname = "/" + name;

	int rank;
	H5LTget_attribute_ndims(file_id, fullname.c_str(), attrname.c_str(),
				&rank );

	dims = (hsize_t *)malloc(rank * sizeof(hsize_t));

	H5LTget_attribute_info( file_id, fullname.c_str(), attrname.c_str(),
				dims, &type_class, &type_size );

	if ( H5T_FLOAT == type_class ) {
		double data[*dims];
		H5LTget_attribute(file_id, fullname.c_str(), attrname.c_str(),
			 H5T_NATIVE_DOUBLE, data);
		cout << attrname << " = ";
		for (unsigned int ii=0; ii<*dims; ii++) {
		  cout << data[ii];
		  if (ii < (*dims)-1)
		    cout << ',';
		  else
		    cout << endl;
		}
	}
	else if ( H5T_INTEGER == type_class ) {
		int data[*dims];
		H5LTget_attribute(file_id, fullname.c_str(), attrname.c_str(),
			 H5T_NATIVE_INT, data);
		cout << attrname << " = ";
		for (unsigned int ii=0; ii<*dims; ii++) {
		  cout << data[ii];
		  if (ii < (*dims)-1)
		    cout << ',';
		  else
		    cout << endl;
		}
	}
	else if ( H5T_STRING == type_class ) {
		char* data;
		string fullname = "/" + name;
		data = (char *)malloc(rank * sizeof(char));
		H5LTget_attribute_string( file_id, fullname.c_str(), 
					  attrname.c_str(),data);
		cout << attrname << " = " << data << endl;
	}
	else {
		cout << "Attribute " << attrname << " type unknown." << endl;
	}
}

void * dalGroup::getAttribute( string attrname ) {

	hsize_t * dims;
	H5T_class_t type_class;
	size_t type_size;

	// Check if attribute exists
	if ( H5LT_find_attribute(group_id, attrname.c_str()) <= 0 ) {
		return NULL;
	}
	
	string fullname = "/" + name;

	int rank;
	H5LTget_attribute_ndims(file_id, fullname.c_str(), attrname.c_str(),
				&rank );

	dims = (hsize_t *)malloc(rank * sizeof(hsize_t));

	H5LTget_attribute_info( file_id, fullname.c_str(), attrname.c_str(),
				dims, &type_class, &type_size );

	if ( H5T_FLOAT == type_class ) {
		double data[*dims];
		if ( 0 < H5LTget_attribute(file_id, fullname.c_str(), attrname.c_str(),
			 H5T_NATIVE_DOUBLE, data) )
		  return NULL;
		else
		  return data;
	}
	else if ( H5T_INTEGER == type_class ) {
		int data[*dims];
		if ( 0 < H5LTget_attribute(file_id, fullname.c_str(), attrname.c_str(),
			 H5T_NATIVE_INT, data) )
		  return NULL;
		else
		  return data;
	}
	else if ( H5T_STRING == type_class ) {
		char* data;
		string fullname = "/" + name;
		data = (char *)malloc(rank * sizeof(char));
		if ( 0 < H5LTget_attribute_string( file_id, fullname.c_str(), 
			  attrname.c_str(),data) )
		  return NULL;
		else
		  return data;
	}
	else {
		return NULL;
	}
}

/*
 * Operator function.
 */
herr_t 
attr_info(hid_t loc_id, const char *name, void *opdata)
{
    hid_t attr, atype, aspace;  /* Attribute, datatype, dataspace identifiers */
    int   rank;
    hsize_t sdim[64]; 
    herr_t ret;
    int i;
    size_t size;
    size_t npoints;             /* Number of elements in the array attribute. */ 
    int point_out;    
    float *float_array;         /* Pointer to the array attribute. */
    H5S_class_t  type_class;

    /* avoid warnings */
    opdata = opdata;

    /*  Open the attribute using its name.  */    
    attr = H5Aopen_name(loc_id, name);

    /*  Display attribute name.  */
    //printf("\nName : ");
    //puts(name);

    /* Get attribute datatype, dataspace, rank, and dimensions.  */
    atype  = H5Aget_type(attr);
    aspace = H5Aget_space(attr);
    rank = H5Sget_simple_extent_ndims(aspace);
    ret = H5Sget_simple_extent_dims(aspace, sdim, NULL);

    /* Get dataspace type */
    type_class = H5Sget_simple_extent_type (aspace);
    //printf ("H5Sget_simple_extent_type (aspace) returns: %i\n", type_class);

    /* Display rank and dimension sizes for the array attribute.  */
    if(rank > 0) {
       //printf("Rank : %d \n", rank); 
       //printf("Dimension sizes : ");
       //for (i=0; i< rank; i++) printf("%d ", (int)sdim[i]);
       //printf("\n");
    }

    if (H5T_INTEGER == H5Tget_class(atype)) {
       //printf("Type : INTEGER \n");
       ret  = H5Aread(attr, atype, &point_out);
       //printf("The value of the attribute \"Integer attribute\" is %d \n", 
         //      point_out);
	cout << name << " = " << point_out << endl;
    }

    if (H5T_FLOAT == H5Tget_class(atype)) {
       //printf("Type : FLOAT \n"); 
       npoints = H5Sget_simple_extent_npoints(aspace);
       float_array = (float *)malloc(sizeof(float)*(int)npoints); 
       ret = H5Aread(attr, atype, float_array);
       //printf("Values : ");
	cout << name << " = ";
       for( i = 0; i < (int)npoints; i++) printf("%f ", float_array[i]); 
       printf("\n");
       free(float_array);
    }

    if (H5T_STRING == H5Tget_class (atype)) {
      //printf ("Type: STRING \n");
      size = H5Tget_size (atype);

	char* data;
	data = (char *)malloc(rank * sizeof(char));
      	ret = H5Aread(attr, atype, data);
	cout << name << " = " << data << endl;

    }

    ret = H5Tclose(atype);
    ret = H5Sclose(aspace);
    ret = H5Aclose(attr);

    return 0;
}

/*
void dalGroup::setAttribute( string attrname, void * data, string coltype )
{
	const int size = 1;
	if ( dal_INT == coltype ) {
			status = H5LTset_attribute_int( file_id, name.c_str(), attrname.c_str(), (const int*)data, size );
	}
	else if ( dal_FLOAT == coltype ) {
			status = H5LTset_attribute_float( file_id, name.c_str(), attrname.c_str(), (const float*)data, size );
	}
	else if ( dal_DOUBLE == coltype ) {
			status = H5LTset_attribute_double( file_id, name.c_str(), attrname.c_str(), (const double*)data, size );
	}
	else {
			cout << "ERROR: datatype " << coltype << " not supported for setAttribute." << endl;
	}			
}
*/
