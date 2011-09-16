"""Configuration and settings.
"""

import os
from math import *
from hftools import *
import pytmf

deg=pi/180.
pi2=pi/2.

def convert(fromvalue,totype):
    """
    Basis of a conversion routine, e.g. for coordinate conversions

    Doesn't do much right now ...
    """
    if fromvalue.has_key("az"):
        fromtype="AZEL"
    if fromtype=="AZEL":
        if totype=="CARTESIAN":
            if fromvalue.has_key("r"):
                r=fromvalue["r"]
            else:
                r=1
            return pytmf.spherical2cartesian(r,pi2-fromvalue["el"],pi2-fromvalue["az"])


def revisionnr(filedir):
    """

    """
    import subprocess

    revision_nr = 0
    process = None

    lofarsoft = os.environ["LOFARSOFT"]
    uses_svn = os.path.exists(lofarsoft+"/.svn")
    uses_git = os.path.exists(lofarsoft+"/.git")

    print "filedir: %s" %(filedir)

    if (uses_svn):
        process = subprocess.Popen(["svn","info",filedir],stdout=subprocess.PIPE)
    elif(uses_git):
        process = subprocess.Popen(["git","svn","info"], stdout=subprocess.PIPE, cwd=filedir)

    if (process):
        lines = process.stdout.readlines()
        for line in lines:
            if "Revision" in line:
                revision_nr = int(line.split()[1])

    return revision_nr



LOFARSOFT=os.environ["LOFARSOFT"]
PYCRBIN=LOFARSOFT+"/release/bin/python/"
PYCR=LOFARSOFT+"/src/PyCRTools/scripts/"
PYP=LOFARSOFT+"/src/PyCRTools/"
PYCRREVNR=revisionnr(PYP)

filename_lofar_onesecond=LOFARSOFT+"/data/lofar/RS307C-readfullsecond.h5"
filename_lofar_onesecond=LOFARSOFT+"/data/lofar/RS307C-readfullsecondtbb1.h5"
filename_lopes_test=LOFARSOFT+"/data/lopes/test.event"
filename_lofar=LOFARSOFT+"/data/lofar/rw_20080701_162002_0109.h5"
filename_lopes=LOFARSOFT+"/data/lopes/example.event"

DEFAULTDATAFILE=filename_lopes

#Current tasks
#import pycrtools.tasks #as tasks
#import pycrtools.tasks.averagespectrum
#import pycrtools.tasks.imager

