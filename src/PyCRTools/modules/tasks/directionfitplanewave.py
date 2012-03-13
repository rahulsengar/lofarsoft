import pycrtools as cr
from pycrtools import tasks
from pycrtools.tasks import shortcuts as sc
from pycrtools import rftools as rf
import matplotlib as mpl
from mpl_toolkits.mplot3d import Axes3D
import pytmf
import math
import numpy as np

deg=math.pi/180.
pi2=math.pi/2.



class DirectionFitPlaneWave(tasks.Task):
    """ Fit a plane wave through arrival times and positions.
        Taken from module srcfind.
        Parameters taken from DirectionFitTriangles.
        
        Outlier removal and iteration to be implemented.
    """
        
    parameters=dict(
    # INPUT
        positions = dict(doc="hArray with Cartesian coordinates of the antenna positions",
                         unit="m"),

        timelags = dict(doc="hArray with the measured time lags for each event and each antenna",
                        unit="s"),
                        
        good_antennas = dict(doc="List with indices of antennas to be used for the plane fit. This should aready exclude stations that were flagged or do not show a reliable pulse"),                

        doplot=sc.p_(False,
                     "Plot results."),

        verbose=sc.p_(True,
                      "Print progress information."),

        rmsfactor=sc.p_(2.0,
                        "How many sigma (times RMS) above the average can a delay deviate from the expected timelag (from latest fit iteration) before it is considered bad and removed as outlier."),
                        

    # OUTPUT
        residual_delays=sc.p_(lambda self:cr.hArray(float,[self.NAnt],name="Residual Delays"),
                              "Residual delays needed to calibrate the array after correction for known cable delays. Will be subtracted from the measured lags (correced for cable delays) or added to the expected. The array will be updated during iteration",
                              unit="s"),

        delta_delays=sc.p_(lambda self:cr.hArray(float,[self.NAnt],name="Delta Delays"),
                           "Additional instrumental delays needed to calibrate array will be added to timelags and will be updated during iteration",
                           unit="s"),

        total_delays=sc.p_(lambda self:cr.hArray(float,[self.NAnt],name="Total Delays"),
                           "Total instrumental (residual+cable) delays needed to calibrate the array. Will be subtracted from the measured lags or added to the expected. The array will be updated during iteration",
                           unit="s"),
                           
        NAnt=sc.p_(lambda self: self.timelags.shape()[-1],
                   "Number of antennas and time lags. If set explicitly, take only the first NAnt antennas from ``Task.positions`` and ``Task.timelags``."),

        refant=sc.p_(0,
                     "Reference antenna for which geometric delay is zero."),

        meandirection=sc.p_(lambda self:cr.hArray(float,[3]),
                            "Cartesian coordinates of mean direction from all good triangles",
                            output=True),

        meandirection_spherical = sc.p_(lambda self:pytmf.cartesian2spherical(self.meandirection[0],self.meandirection[1],self.meandirection[2]),
                                        "Mean direction in spherical coordinates."),

        meandirection_azel=sc.p_(lambda self:(math.pi-(self.meandirection_spherical[2]+pi2),pi2-(self.meandirection_spherical[1])),
                                 "Mean direction as Azimuth (``N->E``), Elevation tuple."),

        meandirection_azel_deg = sc.p_(lambda self:(180-(self.meandirection_spherical[2]+pi2)/deg,90-(self.meandirection_spherical[1])/deg),
                                       "Mean direction as Azimuth (``N->E``), Elevation tuple in degrees."),
        fit_failed = sc.p_(lambda self:False, "Indication if fit failed",output = True)

#        plot_finish = dict(default=lambda self:plotfinish(dopause=False,plotpause=False),
#                           doc="Function to be called after each plot to determine whether to pause or not (see :func:`plotfinish`)"),
#        plot_name = dict(default="",
#                         doc="Extra name to be added to plot filename.")
        )

    def call(self):
        pass

    def run(self):
        from pycrtools import srcfind 
        from numpy import cos, sin
        print "Running PlaneWaveFit ..."
        self.farfield=True
        c = 299792458.0 # speed of light in m/s
        rad2deg = 180.0 / math.pi

        positions = self.positions.toNumpy()
        times = self.timelags.toNumpy()
        times -= times[0]
        
        if self.good_antennas:
            indicesOfGoodAntennas = np.array(self.good_antennas)
            goodSubset = np.array(self.good_antennas) # start with all 'good'

        else:    
            indicesOfGoodAntennas = np.arange(len(times))
            goodSubset = np.arange(len(times)) # start with all 'good'
        
        goodcount = len(goodSubset)
        niter = 0
        while True:
            niter += 1
            goodpositions = positions[indicesOfGoodAntennas].ravel()
            goodtimes = times[indicesOfGoodAntennas]
            (az, el) = srcfind.directionForHorizontalArray(goodpositions, goodtimes)
            if np.isnan(el) or np.isnan(az):
                print 'WARNING: plane wave fit returns NaN. Setting elevation to 0.0'
                el = 0.0 # need to propagate the warning...!
                self.fit_failed = True
                break
            # get residuals
            expectedDelays = srcfind.timeDelaysFromDirection(goodpositions, (az, el)) 
            expectedDelays -= expectedDelays[0]
            self.delta_delays = goodtimes - expectedDelays
            # remove > k-sigma outliers and iterate
            spread = np.std(self.delta_delays)
            k = self.rmsfactor 
            goodSubset = np.where(abs(self.delta_delays - np.mean(self.delta_delays)) < k * spread)
            # gives subset of 'indicesOfGoodAntennas' that is 'good' after this iteration
            if self.verbose:
                print 'iteration # %d' %niter
                print 'az = %f, el = %f' % (az*rad2deg, el*rad2deg)
                print 'good count = %d' % goodcount
            if len(goodSubset[0]) == goodcount:
                break
            else:                
                goodcount = len(goodSubset[0])
                indicesOfGoodAntennas = indicesOfGoodAntennas[goodSubset] 
                
                
        
        cartesianDirection = [cos(el)*sin(az), cos(el)*cos(az), sin(el)]
        self.meandirection.copy(cr.hArray(cartesianDirection))
        # NB! Have to update the dependent parameters, or the previous value will come out.
        self.ws.updateParameter("meandirection_spherical",forced=True)
        self.ws.updateParameter("meandirection_azel_deg",forced=True)
        self.ws.updateParameter("meandirection_azel",forced=True)
           
        # now redo arrays for full antenna set
        expectedDelays = srcfind.timeDelaysFromDirection(positions.ravel(), (az, el)) # need positions as flat 1-D array
        expectedDelays -= expectedDelays[0] # subtract ref ant
        self.delta_delays = times - expectedDelays
        self.delta_delays_mean_history = [np.mean(self.delta_delays)] # comply with DirectionFitTriangles, return as list...
        self.delta_delays_rms_history = [np.std(self.delta_delays)]
        self.delta_delays = cr.hArray(self.delta_delays)
        
        self.residual_delays = self.delta_delays
        if self.doplot:
            import matplotlib.pyplot as plt
            plt.figure()
            plt.plot(self.delta_delays)
            plt.figure()
    #        cr.trerun("PlotAntennaLayout","Delsdays",positions = cr.hArray(goodpositions), colors=cr.hArray(self.delta_delays), sizes=100,title="Residuals for plane wave fit",plotlegend=True)            
    #        plt.figure()