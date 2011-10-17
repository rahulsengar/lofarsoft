nogui=True

import pycrtools as cr
import os


datalocation="/data/VHECR/LORAtriggered/results"
dirList=os.listdir(datalocation)

for fname in dirList:
    if "VHECR" in fname:
        Ev = str(fname)
        try:
            ldf=cr.trun("ldf",topdir=datalocation,events=Ev, plot_xmax=300,draw_global = False,Draw3D = False,save_images=True,generate_html=True)
        except:
            continue
            
        pollist=[0,1]
        filefilter=datalocation+"/"+Ev+"/"
        try:
            for polarization in pollist:
                crfootprint=cr.trun("plotfootprint",filefilter=filefilter,pol=polarization,plotlora=True,save_images=True,generate_html=True)
        except:
            continue