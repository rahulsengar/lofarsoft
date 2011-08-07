"""A tool to plot the footprint of a cosmic ray event with the superterp as background.

"""

import pycrtools as cr
import pycrtools.tasks as tasks
import pytmf
import os
from pycrtools.tasks.shortcuts import *
from pycrtools import lora
from math import *
import pycrtools.rftools as rf

deg=pi/180.
pi2=pi/2.

def gatherresults(filefilter,pol):
    """This function returns a dictionary with selected results from file in the subdirectories (/pol?/*/) of the filedir that are needed for the plotfootprint task."""
    if not filefilter:
        return None

    eventdirs=cr.listFiles(filefilter)

    for eventdir in eventdirs:
        print "Processing event in directory:",eventdir
        par={}
        antid={0:[],1:[]}
        signal={0:[],1:[]}
        positions={0:[],1:[]}
        positions2={0:[],1:[]}
        ndipoles={0:0,1:0}
        timelags={0:[],1:[]}

        datadirs=cr.listFiles(os.path.join(os.path.join(eventdir,"pol?"),"*"))



        for datadir in datadirs:
            if not os.path.isfile(os.path.join(datadir,"results.py")):
                continue
            resfile=open(os.path.join(datadir,"results.py"))
            if "nan" in resfile.read():
                print "WARNING nan found. skipping file", resfile.name
                continue
            res={}
            execfile(os.path.join(datadir,"results.py"),res)
            res=res["results"]
            antid[res["polarization"]].extend([int(v) for v in res["antennas"]])
            positions2[res["polarization"]].extend(res["antenna_positions_ITRF_m"])

            signal[res["polarization"]].extend(res["pulses_maxima_y"])
            ndipoles[res["polarization"]]+=res["ndipoles"]
            pulseoffset=res["SAMPLE_NUMBER"]+res["pulse_start_sample"]+res["BLOCKSIZE"]
            pulseoffset/=res["SAMPLE_FREQUENCY"]
            
            stationtimelags=[l+pulseoffset for l in res["pulses_timelags_ns"]]
            timelags[res["polarization"]].extend(stationtimelags)
            #timelags[res["polarization"]].extend(res["pulses_timelags_ns"])

    if "res" not in dir():
        return None
    antset=res["ANTENNA_SET"]
    if "TIME" not in res.keys():
        assert False
#res["TIME"]=1307920753 if "event-1" in res["FILENAME"] else None

    lorainfo=lora.loraInfo(res["TIME"])
    for k in ["core","direction","energy"]:
        par["lora"+k]=lorainfo[k]

    par["loraarrivaltimes"]=cr.hArray(float,dimensions=len(lorainfo["10*nsec"]),fill=lorainfo["10*nsec"])/1e10

    par["lorapositions"]=[]
    for k in ["posX","posY","posZ"]:
        par["lorapositions"].extend(lorainfo[k])
    par["lorapositions"]=cr.hArray(par["lorapositions"],dimensions=[3,len(lorainfo["posX"])])
    par["lorapower"]=cr.hArray(lorainfo["particle_density(/m2)"],dimensions=[len(lorainfo["posX"])])


    clockcorrection=cr.metadata.get("ClockCorrection",antid[pol],antset,return_as_hArray=True)
    par["positions"]=cr.metadata.get("AntennaPositions",antid[pol],antset,return_as_hArray=True)
    par["power"]=cr.hArray(signal[pol])
#indexvec=cr.hArray(int,dimensions=par["power"])
#nrgrt=cr.hFindGreaterThan(indexvec,par["power"],1.0e15)
#zerovec=cr.hArray(float,dimensions=nrgrt.val(),fill=1e-5)
#par["power"][indexvec[0:3]]=par["power"].min()
    par["arrivaltime"]=cr.hArray(timelags[pol])+clockcorrection
#par["loracolor"]="time"
    par["title"]="Footprint of CR event "+res["FILENAME"].split('-')[1]
    par["names"]=[str(a)[:-6]+","+str(a)[-3:] for a in antid]
    return par

def obtainvalue(par,key):
    """This function returns the value from a parameter dict or a default value if the key does not exist"""
    defaultvalues=dict(
        power=20,
        title=False,
        loracore=None,
        positions=None,
        loraarivaltimes=None,
        loraenergy=None,
        lorapower=100,
        lorapositions=None,
        arrivaltime='b',
        loradirection=None,
        names=False
    )

    
    
    if not par:
        if key in defaultvalues.keys():
            return defaultvalues[key]
        else: 
            print "please add ",key,"to defaults in plotfootprint.py / obtainvalue "
            return None
    else:
        if key in par.keys():
            return par[key]
        else:
            if key in defaultvalues.keys():
                return defaultvalues[key]
            else: 
                print "please add ",key,"to defaults in plotfootprint.py / obtainvalue "
                return None

class plotfootprint(tasks.Task):
    """
    **Description:**

    Plot the layout of the current dataset on the ground.

    **Usage:**

    **See also:**
    :class:`DirectionFitTriangles`

    **Example:**

    ::
        polarization=0
        filefilter="/Volumes/Data/sandertv/VHECR/LORAtriggered/results/VHECR_LORA-20110716T094509.665Z/"
        crfootprint=cr.trun("plotfootprint",filefilter=filefilter,pol=polarization)
   """
    parameters=dict(
        filefilter={default:None,doc:"Obtains results from subdirectories of these files (from results.py)"},
        pol={default:0,doc:"0 or 1 for even or odd polarization"},
        results=p_(lambda self:gatherresults(self.filefilter,self.pol),"hArray with BLAAT transposed Cartesian coordinates of the antenna positions (x0,x1,...,y0,y1...,z0,z1,....)",unit="m",workarray=True),
        positions=p_(lambda self:obtainvalue(self.results,"positions"),doc="hArray of dimension [NAnt,3] with Cartesian coordinates of the antenna positions (x0,y0,z0,...)"),
        size={default:300,doc:"Size of largest point."},
        sizes_min={default:None,doc:"If set, then use this as the minimum scale for the sizes, when normalizing and plotting."},
        sizes_max={default:None,doc:"If set, then use this as the maximum scale for the sizes, when normalizing."},
        normalize_sizes={default:True,doc:"Normalize the sizes to run from 0-1."},
        normalize_colors={default:False,doc:"Normalize the colors to run from 0-1."},
        power=p_(lambda self:obtainvalue(self.results,"power"),doc="hArray of dimension [NAnt] with the values for the power of each lofar antenna. This gives the sizes of the plot"),
        arrivaltime=p_(lambda self:obtainvalue(self.results,"arrivaltime"),doc="hArray of dimension [NAnt] with the values of relative peak arrival time. This gives the colors of the plot"),
        names=p_(lambda self:obtainvalue(self.results,"names"),doc="hArray of dimension [NAnt] with the names or IDs of the antennas"),
        plotnames={default:False,doc:"plot names of dipoles"},
        title=p_(lambda self:obtainvalue(self.results,"title"),doc="Title for the plot (e.g., event or filename)"),
        newfigure=p_(True,"Create a new figure for plotting for each new instance of the task."),
        plot_finish={default: lambda self:cr.plotfinish(doplot=True,plotpause=False),doc:"Function to be called after each plot to determine whether to pause or not (see ::func::plotfinish)"},
        plot_name={default:"footprint",doc:"Extra name to be added to plot filename."},
        plotlegend={default:False,doc:"Plot a legend"},
        positionsT=p_(lambda self:cr.hArray_transpose(self.positions),"hArray with transposed Cartesian coordinates of the antenna positions (x0,x1,...,y0,y1...,z0,z1,....)",unit="m",workarray=True),
        NAnt=p_(lambda self: self.positions.shape()[-2],"Number of antennas.",output=True),
        figure={default:None,doc:"No startplot"},
        plotlora={default:True,doc:"Plot the LORA data when positions are present?"},
        loracore=p_(lambda self:obtainvalue(self.results,"loracore"),doc="Shower core position in hArray(float(X,Y,0))",unit="m"),
        loradirection=p_(lambda self:obtainvalue(self.results,"loradirection"),doc="Shower direction hArray(float,(Azimuth,Elevation)). Azimuth defined from North Eastwards. Elevation defined from horizon up",unit="degrees"),
        lorapositions=p_(lambda self:obtainvalue(self.results,"lorapositions"),doc="3-dim hArray with transposed Cartesian coordinates of the antenna positions (x0,x1,...,y0,y1...,z0,z1,....)",unit="m"),
        lorapower=p_(lambda self:obtainvalue(self.results,"lorapower"),doc="hArray with power of the LORA events"),
        loraarrivaltimes=p_(lambda self:obtainvalue(self.results,"loraarrivaltimes"),doc="hArray with arrival time of LORA events"),
        lorashape={default:"p",doc:"Shape of LORA detectors. e.g. 's' for square, 'p' for pentagram, 'h' for hexagon"},
        lofarshape={default:"o",doc:"Shape of LOFAR antennas. e.g. 'o' for circle, '^' for triangle"},
        loracolor={default:"#BE311A",doc:"Color used for LORA plots. If set to 'time' uses the arrival time" },
        plotlayout={default:True,doc:"Plot the LOFAR layout of the stations as the background"}
        
        )
        

    def call(self):
        pass

    def run(self):

        #Calculate scaled sizes
        #import pdb; pdb.set_trace()
        #if not filefilter and not positions and not topdir:
        #    raise ValueError("PlotFootPrint: specify at least filefilter or positions.")
        
        if isinstance(self.power,(list)):
            self.power=cr.hArray(self.power)
        if isinstance(self.power, (int, long, float)):
            self.ssizes=self.power
        elif isinstance(self.power,tuple(cr.hRealContainerTypes)):
            self.ssizes=cr.hArray(copy=self.power)
            if "Int" in repr(type(self.ssizes)):
                self.ssizes=cr.hArray(float,dimensions=self.ssizes,fill=self.ssizes)
            if self.normalize_sizes:
                self.ssizes /= self.ssizes.max().val() if self.sizes_max==None else self.sizes_max
                self.ssizes *= self.size
                if self.sizes_min:
                    minindex=cr.hArray(int,dimensions=self.ssizes)
                    nrless=cr.hFindLessThan(minindex,self.ssizes,self.sizes_min).val()
                    cr.hCopy(self.ssizes,cr.hArray(float,dimensions=[nrless],fill=self.sizes_min),minindex,nrless)
        else:
            raise TypeError("PlotAntennaLayout: parameter 'sizes' needs to be a number or an hArray of numbers.")

        if isinstance(self.arrivaltime, (str, int, long, float)):
            self.scolors=self.arrivaltime
        elif isinstance(self.arrivaltime,tuple(cr.hAllContainerTypes)):
            self.scolors=cr.hArray(copy=self.arrivaltime)
            if self.normalize_colors:
                self.scolors -= self.scolors.min().val()
                self.scolors /= self.scolors.max().val()
        else:
            raise TypeError("PlotAntennaLayout: parameter 'colors' needs to be string or an hArray thereof.")

        if self.loracolor is "time": 
            if self.loraarrivaltimes:
                self.loracolor=self.loraarrivaltimes
            else:
                self.loracolor="#BE311A"

        if self.newfigure and not self.figure:
            self.figure=cr.plt.figure()
        cr.plt.clf()
        if self.plotlayout:
            from os import environ
            from os.path import isfile
            if "LOFARSOFT" in environ.keys():
                bgimname=environ["LOFARSOFT"]+"/src/PyCRTools/extras/LORA_layout_background.png"
                if isfile(bgimname):
                    bgim=cr.plt.imread(bgimname)
                    cr.plt.imshow(bgim,origin='upper',extent=[-375/2,375/2,-375/2-6*120/227,375/2-6*120/227],alpha=0.2)
                else:
                    print "WARNING Cannot plot layout. Image file not found. Run an svn update?"
            else:
                print "WARNING Cannot plot layout. Environment variable LOFARSOFT not found."
        if self.title:
            cr.plt.title(self.title)
        if self.positions:
            cr.plt.scatter(self.positionsT[0].vec(),self.positionsT[1].vec(),s=self.ssizes,c=self.scolors,marker=self.lofarshape)
        cbar=cr.plt.colorbar()
        if self.plotlora:
            if isinstance(self.lorapower,(list)):
                self.lorapower=cr.hArray(self.lorapower)
            if isinstance(self.lorapower, (int, long, float)):
                self.lsizes=self.lorapower
                if self.lsizes < self.sizes_min :
                    self.lsizes= self.sizes_min
            elif isinstance(self.lorapower,(list)):
                self.lorapower=cr.hArray(self.lorapower)
            elif isinstance(self.lorapower,tuple(cr.hRealContainerTypes)):
                self.lsizes=cr.hArray(copy=self.lorapower)
                if "Int" in repr(type(self.lsizes)):
                    self.lsizes=cr.hArray(float,dimensions=self.lsizes,fill=self.lsizes)
                if self.normalize_sizes:
                    self.lsizes /= self.lsizes.max().val() if self.sizes_max==None else self.sizes_max
                    self.lsizes *= self.size
                    if self.sizes_min:
                        minindex=cr.hArray(int,dimensions=self.lsizes)
                        nrless=cr.hFindLessThan(minindex,self.lsizes,self.sizes_min).val()
                        cr.hCopy(self.lsizes,cr.hArray(float,dimensions=[nrless],fill=self.sizes_min),minindex,nrless)
            else:
                raise TypeError("PlotAntennaLayout: parameter 'sizes' needs to be a number or an hArray of numbers.")
            if self.lorapositions:
                cr.plt.scatter(self.lorapositions[0].vec(),self.lorapositions[1].vec(),s=self.lsizes,c=self.loracolor,marker=self.lorashape,)
            if self.loracore:
                cr.plt.scatter(self.loracore[0],self.loracore[1],marker='x',s=200,color='red',linewidth=3)
                if self.loradirection:
                    dcos=cr.cos(cr.radians(self.loradirection[0]))
                    dsin=cr.sin(cr.radians(self.loradirection[0]))
                    elev=self.loradirection[1]
                    cr.plt.arrow(self.loracore[0]+elev*dsin,self.loracore[1]+elev*dcos,-elev*dsin,-elev*dcos,lw=3,color='red',ls='dashed',hatch='\\')
        if self.names and self.plotnames:
            for label,x,y in zip(self.names,self.positionsT[0].vec(),self.positionsT[1].vec()):
                cr.plt.annotate(str(label),xy=(x,y), xytext=(-3,3),textcoords='offset points', ha='right', va='bottom')
        if self.plotlegend:
            cr.plt.colorbar()
        cr.plt.xlabel("meters East")
        cr.plt.ylabel("meters North")
        cr.plt.text(100,-220,"Size denotes signal power")
        if self.filefilter:
            self.plot_name=self.filefilter+"/"+"pol"+str(self.pol)+"/"+self.plot_name
        self.plot_finish(filename=self.plot_name)