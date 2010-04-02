#gdb --annotate=3 -quiet -f --args python -i /Users/falcke/LOFAR/usg/src/CR-Tools/implement/Pypeline/pycrtools.py -i /Users/falcke/LOFAR/usg/src/CR-Tools/implement/Pypeline/test.py

def p_(var): 
    if (type(var)==list): map(lambda x:p_(x),var)
    else: print " ",var,"=>",eval(var)

#from pycrtools import *

filename_sun=LOFARSOFT+"/data/lopes/example.event"
filename_biglofar=LOFARSOFT+"/data/lofar/rw_20080701_162002_0109.h5"
filename_lofar=LOFARSOFT+"/data/lofar/trigger-2010-02-11/triggered-pulse-2010-02-11-TBB1.h5"
figno=0

plt.clf()
#------------------------------------------------------------------------
filename_cr=LOFARSOFT+"/data/lopes/2004.01.12.00:28:11.577.event"
cr_direction=[41.9898208, 64.70544, 1750.]
cr_shower_core=[-25.44, 8.94, 0]
FarField=False

cr=crfile(filename_cr)
"""

Now we read in the raw time series data as well as the Frequencies and
Time value arrays.

"""
cr_time=cr["Time"].setUnit("\\mu","s")
cr_frequencies=cr["Frequency"]
cr_efield=cr["Fx"].setPar("xvalues",cr_time)
"""

As a next step we create an empty vector to hold the Fourierspectrum
of the data

"""
cr_fft=cr["emptyFFT"].setPar("xvalues",cr_frequencies)
"""

and then make the Fourier transform (noting that the data is in the
second Nyquist domain)

"""
cr_efield[...].fft(cr_fft[...],2)
"""

We first turn this into std vector and create a vector that is
supposed to hold the Cartesian coordinates. Note that the AzEL vector
is actually AzElRadius, where we need to set the radius to unity.

"""
azel=hArray(cr_direction,dimensions=[1,3])
cartesian=azel.new()
"""

We then do the conversion, using

"""
hCoordinateConvert(azel[...],CoordinateTypes.AzElRadius,cartesian[...],CoordinateTypes.Cartesian,FarField)
"""


(++) Coordinates
---------------

For doing actual beamforming we need to know the antenna
positions. For this we will use the method getCalData:

"""
antenna_positions=cr.getCalData("Position")
p_("antenna_positions")
"""

As a next step we need to put the antenna coordinates on a reference
frame that is relative to the phase center. Here we will choose the
location of the first antenna as our phase center (that makes life a
little easier for checking) and we simply subtract the reference
position from the antenna locations so that our phase center lies at
0,0,0.

"""
phase_center=hArray(cr_shower_core)
antenna_positions -= phase_center
""""

Now we read in instrumental delays for each antenna in a similar way
and store it for later use.

"""
cal_delays=cr.getCalData("Delay")
p_("cal_delays")
"""

We now convert the Azimuth-Elevation position into a vector in
Cartesian coordinates, which is what is used by the beamformer.
"""

hCoordinateConvert(azel,CoordinateTypes.AzElRadius,cartesian,CoordinateTypes.Cartesian,FarField)
"""

Then calculate geometric delays and add the instrumental delays.

"""
delays=hArray(float,dimensions=cal_delays)
hGeometricDelays(antenna_positions,cartesian,delays,FarField)
"""

To get the total delay we add the geometric and the calibration delays.

"""
delays += cal_delays
"""

The delays can be converted to phases of coplex weights (to be applied
in the Fourier domain).

"""
phases=hArray(float,dimensions=cr_fft,name="Phases",xvalues=cr_frequencies)
hGeometricPhases(cr_frequencies,antenna_positions,cartesian,phases,FarField)
"""

Similarly, the corresponding complex weights are calculated.

"""
weights=hArray(complex,dimensions=cr_fft,name="Complex Weights")
hGeometricWeights(cr_frequencies,antenna_positions,cartesian,weights,FarField)
"""

To shift the time series data (or rather the FFTed time series data)
we multiply the fft data with the complex weights from above.

"""
cr_calfft_shifted=hArray(copy=cr_fft)
cr_calfft_shifted *= weights
"""

Convert back into time domain

"""

cr_efield_shifted = cr["emptyFx"].setPar("xvalues",cr_time)
cr_calfft_shifted[...].invfft(cr_efield_shifted[...],2)
"""

To now "form a beam" we just need to add all time series data (or
their FFTs),

"""
cr_calfft_shifted_added=hArray(cr_calfft_shifted[0].vec())
cr_calfft_shifted_added.add(cr_calfft_shifted[1:,...])
"""

normalize by the number of antennas (which for some reason we don't
need to do after all), 
cr_calfft_shifted_added /= cr.nofSelectedAntennas

and then FFT back into the time domain:

"""
cr_efield_shifted_added=hArray(float,dimensions=cr_time,name="beamformed E-field",xvalues=cr_time)
cr_calfft_shifted_added.invfft(cr_efield_shifted_added,2)

"""
Finally, we take the absolute and smooth it with a running average
"""
cr_efield_shifted_added_abs=hArray(copy=cr_efield_shifted_added,xvalues=cr_time)
cr_efield_shifted_added_abs.abs()
cr_efield_shifted_added_smoothed=hArray(float,dimensions=[cr.blocksize],xvalues=cr_time,name="E-Field")
cr_efield_shifted_added_abs.runningaverage(cr_efield_shifted_added_smoothed,7,hWEIGHTS.GAUSSIAN)

"""
... and plot.
"""
cr_efield_shifted_added_smoothed.plot(xlim=(-3,-0.5),title=cr.filename)

