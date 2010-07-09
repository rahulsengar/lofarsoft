#! /usr/bin/env python

import os
import numpy as np
import pycr as cr

LOFARSOFT=os.environ["LOFARSOFT"]
filename=LOFARSOFT+"/data/lofar/trigger-2010-04-15/triggered-pulse-2010-04-15-RS205.h5"

# Open file guessing filetype from extension
f = cr.open(filename)

# Or open file with explicit filetype
#f = cr.open(filename, 'LOFAR_TBB')

# Print list of allowed parameters
print f.keys()

# Get the sample offsets if possible
if 'sample_offset' in f:
    print f['sample_offset']

# Set parameters of the DataReader
f.blocksize = 512
f.block = 129

# Get parameters from the DataReader
ants = f['nofantennas']
blocksize = f['blocksize']
fftlength = f['fftlength']

# Read some data
time = np.empty(blocksize)
f.read("Time", time)

print 'Time:'
print time

freqs = np.empty(fftlength)
f.read("Frequency", freqs)

print 'Frequency:'
print freqs

efield = np.empty((ants, blocksize))
f.read("Fx", efield)

print 'Fx:'
print efield

