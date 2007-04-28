%module dal
%include "std_string.i"
%include "cpointer.i"

/* Wrap a class interface around an "int *" */
%pointer_class(int, intp);
%pointer_class(double, doublep);

%{
/* Includes the header in the wrapper code */
#include "dal.h"
#include "dalAttribute.h"
#include "dalColumn.h"
#include "dalFilter.h"
#include "dalGroup.h"
#include "dalTable.h"
#include "dalDataset.h"
%}
/* Parse the header file to generate wrappers */
%include "dal.h"
%include "dalAttribute.h"
%include "dalColumn.h"
%include "dalFilter.h"
%include "dalGroup.h"
%include "dalTable.h"
%include "dalDataset.h"
