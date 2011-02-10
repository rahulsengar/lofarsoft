#
#  pulsefit.py - fit the direction of the triggered pulse based on the full data
#  
#
#  Created by Andreas Horneffer on Aug. 1, 2010.
#  Copyright (c) 2010, Andreas Horneffer. All rights reserved.
#  Ported to pycrtools on Nov. 15, 2010 (AC)
"""
This module supplies different functions to fit the direction of a triggered
pulse

"""
import os
from pycrtools import *
from pycrtools import IO
import matching as match
import srcfind as sfind
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import fmin, brute
import beamformer as bf
import pdb

rad2deg = 180./np.pi
#-------------------------------------------------------------------------------

#------------------------------------------------------------ simplexPositionFit
def simplexPositionFit(crfile, cr_fft, antenna_positions, start_position, ant_indices, 
                       cr_freqs, FarField=True, blocksize=-1, doPlot = False):
#  print 'ANT INDICES: '
#  print ant_indices
#  print ' ' 
  thisBF = bf.Beamformer(crfile, cr_fft) # initialize object. Isn't this an ugly memory leak?
  #import pdb; pdb.set_trace()
  optimum = fmin(thisBF.pulseMaximizer, start_position, (cr_fft, antenna_positions, ant_indices, FarField), xtol=1e-2, ftol=1e-4, full_output=1)
  
  if doPlot and not FarField:
      distResult = np.zeros(100)
      distances = np.zeros(100)
      R = 10.0
      for i in range(100):
          distances[i] = R
          R *= 1.12
#      print distances
      for i in range(100):
          pos = [optimum[0][0], optimum[0][1], 2000.0 / distances[i]]
          distResult[i] = - thisBF.pulseMaximizer(pos, cr_fft, antenna_positions, ant_indices, False)
      
      optR = distances[np.argmax(distResult)]
      pos[2] = 2000.0 / optR
      plt.clf()
      plt.semilogx(distances, distResult)
      raw_input("--- Plotted bf pulse height versus distance - press Enter to continue...")
      
      optBeam = thisBF.getTiedArrayBeam(pos, cr_fft, antenna_positions, ant_indices, False)
  else:
      optBeam = thisBF.getTiedArrayBeam(optimum[0], cr_fft, antenna_positions, ant_indices, FarField)
  
#  bruteRanges = ((start_position[0] - 20.0, start_position[0] + 20.0), (start_position[1] - 10.0, start_position[1] + 10.0))
  #optErg = brute(beamform_function, bruteRanges, Ns = 50, full_output = 1)
  return (optimum, optBeam)

#------------------------------------------------------------- triggerMessageFit
def triggerMessageFit(crfile, triggers, fittype='bruteForce'):  # crfile has to be imported using IO module!
  #Get the trigger-message information
  result = dict(success=False, action = 'Trigger message fit')
  fileDate = crfile["Date"]
  fileSamplenum = crfile["SAMPLE_NUMBER"][0]                  # assuming start sample nr. is the same for all antennas
  ddate = fileDate + fileSamplenum / 200.0e6             
  (mIDs, mdDates, mTdiffs, mTriggerDates, mSampleNums) = match.matchTriggerfileToTime((ddate+0.00033024),triggers)
  if len(mIDs) == 0:
      print 'NO TRIGGERS FOUND'
      result.update(reason = 'No matching triggers found!')
      return result

  #get the position for that 
  nofAntennas = crfile["nofAntennas"]
  # now trim the matched trigger IDs to include only antennas which are actually in the data!
  validChannels = np.argwhere(mIDs < nofAntennas) # get indices in array
#  print mTdiffs
#  print len(mTdiffs)
  mTdiffs = mTdiffs[validChannels].ravel()
#  print mTdiffs
#  print len(mTdiffs)
#  print ' '
  mIDs = mIDs[validChannels].ravel()
#  print len(mIDs)
  match_positions = crfile["RelativeAntennaPositions"].toNumpy()[mIDs].reshape(3 * len(mIDs))
  #print mTdiffs
  #print len(mTdiffs)
#  print match_positions
#  print match_positions
#  print len(match_positions)

  if (fittype=='linearFit'):
    (radaz, radel) = sfind.directionForHorizontalArray(match_positions, mTdiffs)
    if np.isnan(radel) or np.isnan(radaz):
        (radaz, radel, bftime) = sfind.directionBruteForceSearch(match_positions, mTdiffs)
        result.update(warning = 'Linear fit failed (produced NaN). Falling back to brute force method.')
  elif (fittype=='bruteForce'):
    (radaz, radel, bftime) = sfind.directionBruteForceSearch(match_positions, mTdiffs)
  else : 
    radaz = 0.
    radel = 0.
  mse = sfind.mse(radaz, radel, match_positions, mTdiffs)
  degaz = radaz*rad2deg
  degel = radel*rad2deg
  if (degaz < 0.):
    degaz += 360.
  #get the average time-offset
  toffset = np.mean((mTriggerDates - fileDate) + (mSampleNums - fileSamplenum) / 200.0e6) # matching.py returns np array, so keep it (doing mean anyway)
  
  result.update(success = True, az = degaz, el = degel, mse = mse, avgToffset = toffset, nofAntennasUsed = len(mIDs) )
  return result

#------------------------------------------------------------------ fullPulseFit
def fullDirectionFit(crfile, triggerFitResult, blocksize, flaggedList = [], FarField=True, doPlot = False): 
  #Set the parameters
  samplefreq = 200.0e6 # must be
  crfile.set("blocksize", blocksize) # proper way, apparently
#  crfile.set("shift", 512)
  print 'File size is %d' % crfile["Filesize"]
  print 'Block size is now: %d' % crfile["blocksize"]
  print 'So there are: %d blocks' % int((crfile["Filesize"]) / int(crfile["blocksize"]))
  blockNo = int((triggerFitResult["avgToffset"] * samplefreq) / blocksize)
  print "fullPulseFit: set block-number to:", blockNo
#  crfile.set("block", blockNo)
  nofAntennas = crfile["nofAntennas"]
# Apply flagged antenna list - select those out
#  selectedList = range(nofAntennas)
#  for entry in flaggedList:
#      selectedList.remove(entry)  
#  crfile.setAntennaSelection(selectedList)
  
  fftlength = crfile["fftLength"]
  #Get the data
  cr_time = hArray(float, dimensions=[blocksize])
  cr_freqs = hArray(float, dimensions=[blocksize/2 + 1])
  
  cr_efield = crfile["emptyFx"]
  crfile.getTimeseriesData(cr_efield, blockNo) # crfile["Fx"] crashes on invalid block number ???? While it was set to a valid value...

  # temp: plot to see if pulse is in this block...
#  efield = cr_efield.toNumpy()
#  plt.plot(efield.T)
#  plt.show()

  cr_fft = crfile["emptyFFT"]
  print 'Number of antennas = %d' % cr_fft.shape()[0]
  print 'Block length fft = %d' % cr_fft.shape()[1]

  antenna_positions = crfile["RelativeAntennaPositions"]
#  print 'ANTENNA POSITIONS: '
#  print antenna_positions
#  print ' '
  #Now calculate the Fourier transform of the data
  
  cr_fft[...].fftw(cr_efield[...]) 
  for i in range(cr_fft.shape()[0]):
      cr_fft[i, 0] = 0.0 # kill DC offsets
       
  if (FarField):
    start_position = [triggerFitResult["az"], triggerFitResult["el"]]
  else:
    start_position = [triggerFitResult["az"], triggerFitResult["el"], 1.0]
  
  # HACK 
#  start_position = [255.0, 25.0]
  ant_indices = range(0, nofAntennas, 2)
  # for the time being: apply flags here instead of using crfile.setAntennaSelection (avoiding complications)
  for entry in flaggedList:
      if entry in ant_indices:
          ant_indices.remove(entry) # or [ant_indices.remove(x) for x in flaggedList if x in ant_indices] ...
  
  (fitDataEven, optBeamEven) = simplexPositionFit(crfile, cr_fft, antenna_positions, start_position, ant_indices, cr_freqs, FarField=FarField,blocksize=blocksize, doPlot=doPlot)
                                     
  ant_indices = range(1, nofAntennas, 2)
  for entry in flaggedList:
      if entry in ant_indices:
          ant_indices.remove(entry)
          
  (fitDataOdd, optBeamOdd) = simplexPositionFit(crfile, cr_fft, antenna_positions, start_position, ant_indices, 
                                  cr_freqs, FarField=FarField,blocksize=blocksize, doPlot=doPlot)

  result = dict(success = True, action = 'Full direction fit',
                even = dict(az = fitDataEven[0][0], el = fitDataEven[0][1], optValue = - fitDataEven[1], optBeam = optBeamEven),
                odd = dict(az = fitDataOdd[0][0], el = fitDataOdd[0][1], optValue = - fitDataOdd[1], optBeam = optBeamOdd) )
  
  return result

## Executing a module should run doctests.
#  This examines the docstrings of a module and runs the examples
#  as a selftest for the module.
#  See http://docs.python.org/library/doctest.html for more information.
if __name__=='__main__':
    import doctest
    doctest.testmod()
