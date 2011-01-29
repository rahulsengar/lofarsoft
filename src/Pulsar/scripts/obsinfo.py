#!/usr/bin/env python
#
# Script to collect info about LOFAR observations and write out
# the list of obs with different parameters listed, both in ascii
# and Html formats
#
# NOTE: script was tested mainly on BASH, and it's known for
# sure that it fails on CSH, because of many specific cexec and find
# system calls that require shell-specific format. Try using
# another version obsinfo-shell.py
#
# Vlad, Aug 5, 2010 (c)
###################################################################
import os, sys, glob
import getopt
import numpy as np
import time
import cPickle
import re

# sorting type
sortkind="obsid"  # four kinds of sorting currently: by start time ("time"), 
                  # by volume ("size"), by source pointing ("source"), and default is by ObsID ("obsid")

# if True then will show only those observations newer than some date
is_from=False
fromdate=""
# if True then will show only those observations older than some date
is_to=False
todate=""
# if True then make a list in html format
is_html=False
htmlfile=""  # name of the html file in case is_html == True
# if True then create all sorted html lists linked together
is_linkedhtml = False
linkedhtmlstem=""   # filestem of linked html files if is_linkedhtml = True
# if True then delete dump of obs list (if exists) and recreate it from scratch
# if False then first read the dump, compare obsids with the current ones and process
# only those ObsIDs that do not exist in the dump. After the dump is updated
is_rebuild = False
# if True then updates only the records in the dumpfile without checking if new obs appeared
is_update = False
# if True then obs will be printed one by one (debug mode) (with # = 0 for all)
is_debug = False
debugcounter=0

# if True, then only db file will be read to calculate the statistics of observations
is_stats = False

# View of presenting info (usual (defaul), brief, plots, and mega)
viewtype="usual"

# Setting User name
username=os.environ['USER']
# where to copy profile plots (dop95)
webserver="%s@10.87.2.95" % (username, )
plotsdir="/home/%s/Lofar/plots" % (username, )
griddir="/home/%s/Lofar/grid" % (username, )
webplotsdir="public_html/lofar/plots"
webgriddir="public_html/lofar/grid"
# if False, then do not rsync plots to external webserver
is_torsync = True

# common large file with archive links in the GRID
archivefile="/home/leeuwen/grid/archived-at-sara.txt"
gridfiles = ()  # list of archive files

# file used in debug mode to write iteration number in main loop
debugfile="/home/%s/.obsinfo_debug" % (username, )

atnflink_start="http://www.atnf.csiro.au/research/pulsar/psrcat/proc_form.php?startUserDefined=true&c1_val=&c2_val=&c3_val=&c4_val=&sort_attr=jname&sort_order=asc&condition=&pulsar_names="
atnflink_end="&ephemeris=long&submit_ephemeris=Get+Ephemeris&coords_unit=raj%2Fdecj&radius=&coords_1=&coords_2=&style=Long+with+last+digit+error&no_value=*&fsize=3&x_axis=&x_scale=linear&y_axis=&y_scale=linear&state=query"
#nedlink_start="http://nedwww.ipac.caltech.edu/cgi-bin/nph-objsearch?objname="
#nedlink_end="&extend=no&hconst=73&omegam=0.27&omegav=0.73&corr_z=1&out_csys=Equatorial&out_equinox=J2000.0&obj_sort=RA+or+Longitude&of=pre_text&zv_breaker=30000.0&list_limit=5&img_stamp=YES"
simbadlinkregion_start="http://simbad.u-strasbg.fr/simbad/sim-sam?Criteria=region%28circle%2c%20icrs%2c%20" # %20 or %20%2b
simbadlinkregion_end="%2c%205d%29%20%26%0d%0a%28otype%20%3d%20%27PSR%27%20%7c%20otype%20%3d%20%27ISM%27%20%7c%20otype%20%3d%20%27mR%27%29&submit=submit%20query&maxObject=100&OutputMode=LIST"
simbadlink_identifier="http://simbad.u-strasbg.fr/simbad/sim-id?Radius=5&Radius.unit=deg&submit=submit%20id&NbIdent=1&Ident="

# storage nodes to collect info about Pulsar Observations
# we assume that even for the case of long observations when data were spreaded out
# across many other nodes, these three "pulsar" sub5 nodes were used to record a part
# of the data as well
storage_nodes=["lse013", "lse014", "lse015", "lse016", "lse017", "lse018"]
# list of directories withe the data
data_dirs=["/data1", "/data2", "/data3", "/data4"]

# cexec corresponding table
cexec_nodes={'lse013': 'sub5:9', 'lse014': 'sub5:10', 'lse015': 'sub5:11',
             'lse016': 'sub6:9', 'lse017': 'sub6:10', 'lse018': 'sub6:11',
             'lse019': 'sub7:9', 'lse020': 'sub7:10', 'lse021': 'sub7:11',
             'lse022': 'sub8:9', 'lse023': 'sub8:10', 'lse024': 'sub8:11',
             'lse001': 'sub1:9', 'lse002': 'sub1:10', 'lse003': 'sub1:11',
             'lse004': 'sub2:9', 'lse005': 'sub2:10', 'lse006': 'sub2:11',
             'lse007': 'sub3:9', 'lse008': 'sub3:10', 'lse009': 'sub3:11',
             'lse010': 'sub4:9', 'lse011': 'sub4:10', 'lse012': 'sub4:11'}
cexec_egrep_string="egrep -v \'\\*\\*\\*\\*\\*\' |egrep -v \'\\-\\-\\-\\-\\-\'"

# list of lse nodes names for use as keys in the dictionary that keeps the sizes of obs directories in outputInfo class
lsenames = ["lse%03d" % (n, ) for n in np.arange(1,25,1)]

# file to dump the obs table
dumpfile="/home/%s/Lofar/dump.b" % (username, )

# directories with parset files
parset_logdir="/globalhome/lofarsystem/log/"
parset_oldlogdir="/globalhome/lofarsystem/oldlog/"
parset_newlogdir="/globalhome/lofarsystem/production/lofar-trunk/bgfen/log/"
# name of the parset file
parset="RTCP.parset.0"

# list of obs ids
obsids=[]
# list of ObsIDs from the dump file 
dbobsids=[]





# Class obsinfo with info from the parset file
class obsinfo:
	def __init__(self, log):
		self.parset = log	
		# Getting the Date of observation
	        cmd="grep Observation.startTime %s | tr -d \\'" % (self.parset,)
        	self.starttime=os.popen(cmd).readlines()
        	if np.size(self.starttime) > 0:
                	# it means that this keyword exist and we can extract the info
                	self.starttime=os.popen(cmd).readlines()[0][:-1].split(" = ")[-1]
			# Getting the number of seconds from 1970. Can use this to sort obs out by date/time
			self.seconds=time.mktime(time.strptime(self.starttime, "%Y-%m-%d %H:%M:%S"))
                	smonth=self.starttime.split("-")[1]
                	sday=self.starttime.split("-")[2].split(" ")[0]
                	self.datestring=smonth+sday
        	else:
                	self.datestring="????"
			self.seconds = 0

		# Getting the Antenna info (HBA or LBA)
        	cmd="grep 'Observation.bandFilter' %s" % (self.parset,)
        	self.antenna=os.popen(cmd).readlines()
        	if np.size(self.antenna)>0:
                	# Antenna array setting exists in parset file
                	self.antenna=os.popen(cmd).readlines()[0][:-1].split(" = ")[-1].split("_")[0]
        	else:
                	self.antenna="?"

		# Getting the Filter setting
        	cmd="grep 'Observation.bandFilter' %s" % (self.parset,)
        	self.band=os.popen(cmd).readlines()
        	if np.size(self.band)>0:
                	# band filter setting exists in parset file
                	self.band=os.popen(cmd).readlines()[0][:-1].split(" = ")[-1].split("A_")[-1]
        	else:
                	self.band="?"

		# Getting the stations and their number (including separately the number of CS and RS)
#        	cmd="grep 'Observation.VirtualInstrument.stationList' %s" % (self.parset,)
        	cmd="grep 'OLAP.storageStationNames' %s" % (self.parset,)
        	self.stations=os.popen(cmd).readlines()
        	if np.size(self.stations)>0:
                	# Stations setting exists in parset file
                	self.stations=os.popen(cmd).readlines()[0][:-1].split(" = ")[-1].split("[")[1].split("]")[0]
			# removing LBA and HBA from station names, replacing HBA ears HBA0 to /0 and HBA1 to /1
			self.stations = re.sub("HBA0", "/0", self.stations)
			self.stations = re.sub("HBA1", "/1", self.stations)
			self.stations = re.sub("HBA", "", self.stations)
			self.stations = re.sub("LBA", "", self.stations)
			stations_array = self.stations.split(",")
			self.nstations = np.size(stations_array)
			self.ncorestations = self.stations.count("CS")
			self.nremotestations = self.stations.count("RS")
			self.stations_string = "%d [%dCS, %dRS]" % (self.nstations, self.ncorestations, self.nremotestations) 
			self.stations_html=""
			for n in np.arange(self.nstations-1):
				if n != 0 and n % 9 == 0: self.stations_html += "<br>"
				self.stations_html += stations_array[n] + ","
			self.stations_html += stations_array[-1]
        	else:
                	self.stations="?"
			self.stations_string="?"

	        # reading the parset file
	        # getting the info about StorageNodes. Note! For old parsets there seems to be no such a keyword Virtual...
        	# However, the old keyword OLAP.storageNodeList has "non-friendly" format, so I just ignore this by now
        	cmd="grep Observation.VirtualInstrument.storageNodeList %s | sed -e 's/lse//g'" % (self.parset,)
        	self.nodeslist=os.popen(cmd).readlines()
        	if np.size(self.nodeslist) > 0:
                	# it means that this keyword exist and we can extract the info
                	self.nodeslist=os.popen(cmd).readlines()[0][:-1].split(" = ")[-1]

		# getting the subcluster number where the raw data were taken (e.g. sub4, sub5, etc.)
		# if raw data were spread between several subclusters, then 'subA' is assigned
		# these data are checked if there are more than 3 compute nodes in nodeslist
		# also, even if there are 3 or less storage nodes used, we check if they belong to the
		# same subcluster, if not then 'subA' is assigned
		# is 'nodeslist' is empty, or has old strange node names like 'list...', then we 
		# assign 'sub?' to them. If the data is processed and present in the archive, then 'sub?'
		# will be further corrected
		try:
			snames = np.unique([cexec_nodes["lse%s" % (i)].split(":")[0] for i in self.nodeslist.split("[")[1].split("]")[0].split(",")])
			if np.size(snames) == 0:
				self.subcluster = 'sub?'
			elif np.size(snames) > 1:
				self.subcluster = "subA"
			else:
				self.subcluster = snames[0]
		except:
			self.subcluster = 'sub?'

		# After we figured the correct subcluster, then we cut the nodeslist string
        	# cut the string of nodes if it is too long
        	if len(self.nodeslist)>13:
                	self.nodeslist=self.nodeslist[:13] + "..."

	        # getting the name of /data? where the data are stored
		try:
        		cmd="grep Observation.MSNameMask %s" % (self.parset,)
        		self.datadir="/" + os.popen(cmd).readlines()[0][:-1].split(" = ")[-1].split("/")[1]
		# it seems that for some of observations, the field Observation.MSNameMask in the parset file is missing
		# instead, there is another field called "OLAP.Storage.targetDirectory"
		except:
        		cmd="grep OLAP.Storage.targetDirectory %s" % (self.parset,)
        		self.datadir="/" + os.popen(cmd).readlines()[0][:-1].split(" = ")[-1].split("/")[1]

	        # getting info about the Type of the data (BF, Imaging, etc.)
        	# check first if data are beamformed
        	cmd="grep outputBeamFormedData %s" % (self.parset,)
        	self.bftype=os.popen(cmd).readlines()
        	if np.size(self.bftype) > 0:
                	# this info exists in parset file
                	self.bftype=os.popen(cmd).readlines()[0][:-1].split(" = ")[-1].lower()[:1]
                	if self.bftype == 'f':
                        	self.bftype = "-"
                	else:
                        	self.bftype = "+"
        	else:
                	self.bftype = "?"

        	# check first if data are filtered
        	cmd="grep outputFilteredData %s" % (self.parset,)
        	self.fdtype=os.popen(cmd).readlines()
        	if np.size(self.fdtype) > 0:
                	# this info exists in parset file
                	self.fdtype=os.popen(cmd).readlines()[0][:-1].split(" = ")[-1].lower()[:1]
                	if self.fdtype == 'f':
                        	self.fdtype = "-"
                	else:
                        	self.fdtype = "+"
        	else:
                	self.fdtype = "?"

	        # check if data are imaging
        	cmd="grep outputCorrelatedData %s" % (self.parset,)
        	self.imtype=os.popen(cmd).readlines()
        	if np.size(self.imtype) > 0:
                	# this info exists in parset file
                	self.imtype=os.popen(cmd).readlines()[0][:-1].split(" = ")[-1].lower()[:1]
                	if self.imtype == 'f':
                        	self.imtype = "-"
                	else:
                        	self.imtype = "+"
        	else:
                	self.imtype = "?"

	        # check if data are incoherent stokes data
        	cmd="grep outputIncoherentStokes %s" % (self.parset,)
        	self.istype=os.popen(cmd).readlines()
        	if np.size(self.istype) > 0:
                	# this info exists in parset file
                	self.istype=os.popen(cmd).readlines()[0][:-1].split(" = ")[-1].lower()[:1]
                	if self.istype == 'f':
                        	self.istype = "-"
                	else:
                        	self.istype = "+"
        	else:
                	self.istype = "?"

	        # check if data are coherent stokes data
        	cmd="grep outputCoherentStokes %s" % (self.parset,)
        	self.cstype=os.popen(cmd).readlines()
        	if np.size(self.cstype) > 0:
                	# this info exists in parset file
                	self.cstype=os.popen(cmd).readlines()[0][:-1].split(" = ")[-1].lower()[:1]
                	if self.cstype == 'f':
                        	self.cstype = "-"
                	else:
                        	self.cstype = "+"
        	else:
                	self.cstype = "?"

	        # check if data are fly's eye mode data
        	cmd="grep PencilInfo.flysEye %s" % (self.parset,)
        	self.fetype=os.popen(cmd).readlines()
        	if np.size(self.fetype) > 0:
                	# this info exists in parset file
                	self.fetype=os.popen(cmd).readlines()[0][:-1].split(" = ")[-1].lower()[:1]
                	if self.fetype == 'f':
                        	self.fetype = "-"
                	else:
                        	self.fetype = "+"
        	else:
                	self.fetype = "?"

	        # getting info about the pointing
        	cmd="grep 'Beam\[0\].angle1' %s" % (self.parset,)
        	self.rarad=os.popen(cmd).readlines()
        	if np.size(self.rarad)>0:
                	# RA info exists in parset file
                	self.rarad=float(os.popen(cmd).readlines()[0][:-1].split(" = ")[-1])
                	rahours=self.rarad*12./3.1415926
                	rah=int(rahours)
                	ram=int((rahours-rah)*60.)
                	self.rastring="%02d%02d" % (rah, ram)
        	else:
                	self.rastring="????"

        	cmd="grep 'Beam\[0\].angle2' %s" % (self.parset,)
        	self.decrad=os.popen(cmd).readlines()
        	if np.size(self.decrad)>0:
                	# DEC info exists in parset file
                	self.decrad=float(os.popen(cmd).readlines()[0][:-1].split(" = ")[-1])
                	decdeg=self.decrad*180./3.1415926
                	if decdeg>0:
                        	decsign="+"
                	else:
                        	decsign="-"
                	decdeg=abs(decdeg)
                	decd=int(decdeg)
                	decm=int((decdeg-decd)*60.)
                	self.decstring="%c%02d%02d" % (decsign, decd, decm)
        	else:
                	self.decstring="_????"
        	self.pointing="%s%s" % (self.rastring, self.decstring)

	        # getting info about Source name (new addition to the Parset files)
        	cmd="grep 'Observation.Beam\[0\].target' %s" % (self.parset,)
        	self.source=os.popen(cmd).readlines()
        	if np.size(self.source)>0:
                	# Source name exists in parset file
                	self.source=os.popen(cmd).readlines()[0][:-1].split(" = ")[-1]
			if self.source != "":
				if self.source[0] == "'":
					self.source=self.source.split("'")[1]
				if self.source[0] == "\"":
					self.source=self.source.split("\"")[1]
        	else:
                	self.source=""

        	# Getting the Duration
        	cmd="grep Observation.stopTime %s | tr -d \\'" % (self.parset,)
        	self.stoptime=os.popen(cmd).readlines()
        	if np.size(self.starttime) > 0 and np.size(self.stoptime) > 0:
                	# it means that both start and stop Times exist in parset file
                	self.stoptime=os.popen(cmd).readlines()[0][:-1].split(" = ")[-1]
                	c1 = time.strptime(self.starttime, "%Y-%m-%d %H:%M:%S")
                	c2 = time.strptime(self.stoptime, "%Y-%m-%d %H:%M:%S")
                	self.dur=time.mktime(c2)-time.mktime(c1)  # difference in seconds
                	if float(self.dur/3600.0) > 1.:
                        	self.duration="%.1fh" % (self.dur/3600.)
                	else:
                        	self.duration="%.1fm" % (self.dur/60.)
        	else:
                	self.duration="?"







# Class (structure) with all fields to output
class outputInfo:
	def __init__(self, id):
		self.id = id
		self.obsyear = self.id.split("_")[0][1:]
		try:
			self.seconds=time.mktime(time.strptime(self.obsyear, "%Y"))
		except StandardError:
			print "Network problems or non-standard obs dir name: %s" % (self.id, )
			sys.exit(1)
		self.storage = {}
		for key in lsenames:
			self.storage[key] = ["x", "0.0"]

	def get_link (self):
		# if source name starts on "B" or "J" -> it is pulsar
		if self.oi.source[0] == "B" or self.oi.source[0] == "J":
			link = atnflink_start + self.oi.source.replace("+", "%2B") + atnflink_end
		# if source name starts on "Pos" -> it's survey pointing -> coordinate search of Simbad
		elif self.oi.source[:3] == "Pos":
			rad=self.oi.source.split(" ")[1]
			decd=self.oi.source.split(" ")[2].split("+")[-1]
			link = simbadlinkregion_start + rad
			if float(decd) > 0:
				link = link + "%20%2b" + decd + simbadlinkregion_end   # "%20%2b" is " +"
			else:
				link = link + "%20" + decd + simbadlinkregion_end      # "%20" is " "
		# everything else -> search in NED database (deprecated, Simbad is now)
		else:
			link = simbadlink_identifier + self.oi.source.split("(")[0].replace(" ", "%20")
#			link = nedlink_start + self.oi.source.replace("+", "%2B") + nedlink_end
		return link
	
	def setcomment (self, id, storage_nodes, comment):
		self.id = id
		self.obsyear = self.id.split("_")[0][1:]
		self.seconds=time.mktime(time.strptime(self.obsyear, "%Y"))
		self.comment = comment
		self.subcluster = "sub?"
		self.totsize = 0.0
		self.processed_dirsize = 0.0
		self.pointing = "????_????"
		self.cs = len(storage_nodes)
		if viewtype == "brief":
			self.colspan = 13
		elif viewtype == "plots":
			self.colspan = 20
		elif viewtype == "mega":
			self.colspan = 24 + self.cs
		else:
			self.colspan = 14 + self.cs

		if self.comment != "":
			self.info = self.comment
			self.infohtml = "<td>%s</td>\n <td colspan=%d align=left>%s</td>" % (self.id, self.colspan, self.comment,)

	def Init(self, id, oi, storage_nodes, dirsizes, statusline, reduced_node, redlocation, processed_dirsize, comment, filestem_array, chi_array, combined_plot, archivestatus):
		self.id = id
		self.obsyear = self.id.split("_")[0][1:]
		self.oi = oi
		if self.oi.seconds != 0:
			self.seconds = self.oi.seconds
		else:
			self.seconds=time.mktime(time.strptime(self.obsyear, "%Y"))
		self.subcluster = self.oi.subcluster
		self.pointing = self.oi.pointing
		self.statusline = statusline
		self.reduced_node = reduced_node
#		if self.reduced_node != "" and self.oi.subcluster == 'sub?':
		if self.reduced_node != "":
			self.oi.subcluster = cexec_nodes[self.reduced_node]
		self.redlocation = redlocation
		self.processed_dirsize = processed_dirsize
		self.comment = comment
		self.cs = len(storage_nodes)
		if viewtype == "brief":
			self.colspan = 13
		elif viewtype == "plots":
			self.colspan = 20
		elif viewtype == "mega":
			self.colspan = 24 + self.cs
		else:
			self.colspan = 14 + self.cs
		self.filestem_array = filestem_array
		self.chi_array = chi_array
		self.combined_plot = combined_plot
		self.archivestatus = archivestatus

		# checking if the datadir exists in all lse nodes and if it does, gets the size of directory
		self.totsize=0.0
		self.dirsize_string=""
		for l in storage_nodes:
			self.storage[l]=dirsizes[l]
		for l in storage_nodes:
			self.totsize = self.totsize + float(self.storage[l][1])
			self.dirsize_string = self.dirsize_string + self.storage[l][0] + "\t"
		# converting total size to GB
		self.totsize = "%.1f" % (self.totsize / 1024. / 1024. / 1024.,)
		self.dirsize_string_html = "</td>\n <td align=center>".join(self.dirsize_string.split("\t")[:-1])

		if self.comment == "":
			if viewtype == "brief":
				self.info = "%s	%s	%s	%s	%s	%s	   %-15s  %c  %c  %c  %c  %c  %c	%s		%-27s" % (self.id, self.oi.source != "" and self.oi.source or self.oi.pointing, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype, self.oi.fdtype, self.oi.imtype, self.oi.istype, self.oi.cstype, self.oi.fetype, self.redlocation, self.statusline)
			elif viewtype == "plots":
				self.info = "%s	%s	%s	%s	%s	%s	   %-15s  %c  %c  %c  %c  %c  %c	%s		%-27s   %s" % (self.id, self.oi.source != "" and self.oi.source or self.oi.pointing, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype, self.oi.fdtype, self.oi.imtype, self.oi.istype, self.oi.cstype, self.oi.fetype, self.redlocation, self.statusline, self.archivestatus)
			elif viewtype == "mega":
				self.info = "%s	%s	%s	%s	%s	%s	   %-15s  %c  %c  %c  %c  %c  %c	%-16s %s	%s%-9s	%s	%s		%-27s   %s" % (self.id, self.oi.source != "" and self.oi.source or self.oi.pointing, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype, self.oi.fdtype, self.oi.imtype, self.oi.istype, self.oi.cstype, self.oi.fetype, self.oi.nodeslist, self.oi.datadir, self.dirsize_string, self.totsize, self.oi.stations, self.redlocation, self.statusline, self.archivestatus)
			else: # usual
				self.info = "%s	%s	%s	%-16s %s	%s%s		%c  %c  %c  %c  %c  %c	%-27s	%s   %s" % (self.id, self.oi.datestring, self.oi.duration, self.oi.nodeslist, self.oi.datadir, self.dirsize_string, self.totsize, self.oi.bftype, self.oi.fdtype, self.oi.imtype, self.oi.istype, self.oi.cstype, self.oi.fetype, self.statusline, self.oi.pointing, self.oi.source)
			if viewtype == "brief":
				if self.oi.source == "":
					self.infohtml="<td>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=left>%s</td>" % (self.id, self.oi.pointing, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype == "-" and "&#8211;" or self.oi.bftype, self.oi.fdtype == "-" and "&#8211;" or self.oi.fdtype, self.oi.imtype == "-" and "&#8211;" or self.oi.imtype, self.oi.istype == "-" and "&#8211;" or self.oi.istype, self.oi.cstype == "-" and "&#8211;" or self.oi.cstype, self.oi.fetype == "-" and "&#8211;" or self.oi.fetype, self.redlocation)
				else:
					self.infohtml="<td>%s</td>\n <td align=center><a href=\"%s\">%s</a></td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=left>%s</td>" % (self.id, self.get_link(), self.oi.source, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype == "-" and "&#8211;" or self.oi.bftype, self.oi.fdtype == "-" and "&#8211;" or self.oi.fdtype, self.oi.imtype == "-" and "&#8211;" or self.oi.imtype, self.oi.istype == "-" and "&#8211;" or self.oi.istype, self.oi.cstype == "-" and "&#8211;" or self.oi.cstype, self.oi.fetype == "-" and "&#8211;" or self.oi.fetype, self.redlocation)
			elif viewtype == "plots" or viewtype == "mega":
				if self.oi.source == "":
					self.infohtml="<td>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>" % (self.id, self.oi.pointing, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype == "-" and "&#8211;" or self.oi.bftype, self.oi.fdtype == "-" and "&#8211;" or self.oi.fdtype, self.oi.imtype == "-" and "&#8211;" or self.oi.imtype, self.oi.istype == "-" and "&#8211;" or self.oi.istype, self.oi.cstype == "-" and "&#8211;" or self.oi.cstype, self.oi.fetype == "-" and "&#8211;" or self.oi.fetype)
				else:
					self.infohtml="<td>%s</td>\n <td align=center><a href=\"%s\">%s</a></td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>" % (self.id, self.get_link(), self.oi.source, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype == "-" and "&#8211;" or self.oi.bftype, self.oi.fdtype == "-" and "&#8211;" or self.oi.fdtype, self.oi.imtype == "-" and "&#8211;" or self.oi.imtype, self.oi.istype == "-" and "&#8211;" or self.oi.istype, self.oi.cstype == "-" and "&#8211;" or self.oi.cstype, self.oi.fetype == "-" and "&#8211;" or self.oi.fetype)
				# adding RSP0 chi-square and profile
				self.infohtml = self.infohtml + "\n <td align=center>%s</td>" % (self.chi_array[0])
				if self.filestem_array[0] == "":
					self.infohtml = self.infohtml + "\n <td align=center></td>"
				else:
					self.infohtml = self.infohtml + "\n <td align=center><a href=\"plots/%s/%s.png\"><img width=200 height=140 src=\"plots/%s/%s.th.png\"></a></td>" % (self.id, self.filestem_array[0], self.id, self.filestem_array[0])
				# adding RSPA chi-square and profile
				self.infohtml = self.infohtml + "\n <td align=center>%s</td>" % (self.chi_array[1])
				if self.filestem_array[1] == "":
					self.infohtml = self.infohtml + "\n <td align=center></td>"
				else:
					self.infohtml = self.infohtml + "\n <td align=center><a href=\"plots/%s/%s.png\"><img width=200 height=140 src=\"plots/%s/%s.th.png\"></a></td>" % (self.id, self.filestem_array[1], self.id, self.filestem_array[1])

				# adding combined_plot column
				if self.combined_plot != "":
					self.infohtml = self.infohtml + "\n <td align=center><a href=\"plots/%s/%s.png\"><img width=200 height=140 src=\"plots/%s/%s.th.png\"></a></td>" % (self.id, self.combined_plot, self.id, self.combined_plot)
				else:
					self.infohtml = self.infohtml + "\n <td align=center></td>"
				# adding the rest (columns) of the table
				if viewtype == "plots": self.infohtml = self.infohtml + "\n <td align=left>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>" % (self.redlocation, self.statusline.replace("-", "&#8211;"), self.archivestatus == "x" and self.archivestatus or "<a href=\"grid/%s.txt\">%s</a>" % (self.id, self.archivestatus))
				if viewtype == "mega":
					self.infohtml = self.infohtml + "\n <td>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=left style=\"white-space: nowrap;\">%s</td>\n <td align=left>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>" % (self.oi.nodeslist, self.oi.datadir, self.dirsize_string_html, self.totsize, self.oi.stations_html, self.redlocation, self.statusline.replace("-", "&#8211;"), self.archivestatus == "x" and self.archivestatus or "<a href=\"grid/%s.txt\">%s</a>" % (self.id, self.archivestatus))
			else: # usual
				self.infohtml="<td>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>" % (self.id, self.oi.datestring, self.oi.duration, self.oi.nodeslist, self.oi.datadir, self.dirsize_string_html, self.totsize, self.oi.bftype == "-" and "&#8211;" or self.oi.bftype, self.oi.fdtype == "-" and "&#8211;" or self.oi.fdtype, self.oi.imtype == "-" and "&#8211;" or self.oi.imtype, self.oi.istype == "-" and "&#8211;" or self.oi.istype, self.oi.cstype == "-" and "&#8211;" or self.oi.cstype, self.oi.fetype == "-" and "&#8211;" or self.oi.fetype, self.statusline.replace("-", "&#8211;"), self.oi.pointing, self.oi.source)
		else:
			self.info = self.comment
			self.infohtml = "<td>%s</td>\n <td colspan=%d align=left>%s</td>" % (self.id, self.colspan, self.comment,)

	def update(self, storage_nodes):
		self.cs = len(storage_nodes)
		# checking if the datadir exists in all lse nodes and if it does, gets the size of directory
		self.totsize=0.0
		self.dirsize_string=""
		for l in storage_nodes:
			self.totsize = self.totsize + float(self.storage[l][1])
			self.dirsize_string = self.dirsize_string + self.storage[l][0] + "\t"
		# converting total size to GB
		self.totsize = "%.1f" % (self.totsize / 1024. / 1024. / 1024.,)
		self.dirsize_string_html = "</td>\n <td align=center>".join(self.dirsize_string.split("\t")[:-1])

		if viewtype == "brief":
			self.colspan = 13
		elif viewtype == "plots":
			self.colspan = 20
		elif viewtype == "mega":
			self.colspan = 24 + self.cs
		else:
			self.colspan = 14 + self.cs

		if self.comment == "":
			if viewtype == "brief":
				self.info = "%s	%s	%s	%s	%s	%s	   %-15s  %c  %c  %c  %c  %c  %c	%s		%-27s" % (self.id, self.oi.source != "" and self.oi.source or self.oi.pointing, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype, self.oi.fdtype, self.oi.imtype, self.oi.istype, self.oi.cstype, self.oi.fetype, self.redlocation, self.statusline)
			elif viewtype == "plots":
				self.info = "%s	%s	%s	%s	%s	%s	   %-15s  %c  %c  %c  %c  %c  %c	%s		%-27s   %s" % (self.id, self.oi.source != "" and self.oi.source or self.oi.pointing, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype, self.oi.fdtype, self.oi.imtype, self.oi.istype, self.oi.cstype, self.oi.fetype, self.redlocation, self.statusline, self.archivestatus)
			elif viewtype == "mega":
				self.info = "%s	%s	%s	%s	%s	%s	   %-15s  %c  %c  %c  %c  %c  %c	%-16s %s	%s%-9s	%s	%s		%-27s   %s" % (self.id, self.oi.source != "" and self.oi.source or self.oi.pointing, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype, self.oi.fdtype, self.oi.imtype, self.oi.istype, self.oi.cstype, self.oi.fetype, self.oi.nodeslist, self.oi.datadir, self.dirsize_string, self.totsize, self.oi.stations, self.redlocation, self.statusline, self.archivestatus)
			else: #usual
				self.info = "%s	%s	%s	%-16s %s	%s%s		%c  %c  %c  %c  %c  %c	%-27s	%s   %s" % (self.id, self.oi.datestring, self.oi.duration, self.oi.nodeslist, self.oi.datadir, self.dirsize_string, self.totsize, self.oi.bftype, self.oi.fdtype, self.oi.imtype, self.oi.istype, self.oi.cstype, self.oi.fetype, self.statusline, self.oi.pointing, self.oi.source)

			if viewtype == "brief":
				if self.oi.source == "":
					self.infohtml="<td>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=left>%s</td>" % (self.id, self.oi.pointing, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype == "-" and "&#8211;" or self.oi.bftype, self.oi.fdtype == "-" and "&#8211;" or self.oi.fdtype, self.oi.imtype == "-" and "&#8211;" or self.oi.imtype, self.oi.istype == "-" and "&#8211;" or self.oi.istype, self.oi.cstype == "-" and "&#8211;" or self.oi.cstype, self.oi.fetype == "-" and "&#8211;" or self.oi.fetype, self.redlocation)
				else:
					self.infohtml="<td>%s</td>\n <td align=center><a href=\"%s\">%s</a></td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=left>%s</td>" % (self.id, self.get_link(), self.oi.source, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype == "-" and "&#8211;" or self.oi.bftype, self.oi.fdtype == "-" and "&#8211;" or self.oi.fdtype, self.oi.imtype == "-" and "&#8211;" or self.oi.imtype, self.oi.istype == "-" and "&#8211;" or self.oi.istype, self.oi.cstype == "-" and "&#8211;" or self.oi.cstype, self.oi.fetype == "-" and "&#8211;" or self.oi.fetype, self.redlocation)
			elif viewtype == "plots" or viewtype == "mega":
				if self.oi.source == "":
					self.infohtml="<td>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>" % (self.id, self.oi.pointing, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype == "-" and "&#8211;" or self.oi.bftype, self.oi.fdtype == "-" and "&#8211;" or self.oi.fdtype, self.oi.imtype == "-" and "&#8211;" or self.oi.imtype, self.oi.istype == "-" and "&#8211;" or self.oi.istype, self.oi.cstype == "-" and "&#8211;" or self.oi.cstype, self.oi.fetype == "-" and "&#8211;" or self.oi.fetype)
				else:
					self.infohtml="<td>%s</td>\n <td align=center><a href=\"%s\">%s</a></td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>" % (self.id, self.get_link(), self.oi.source, self.oi.datestring, self.oi.duration, self.oi.antenna, self.oi.band, self.oi.stations_string, self.oi.bftype == "-" and "&#8211;" or self.oi.bftype, self.oi.fdtype == "-" and "&#8211;" or self.oi.fdtype, self.oi.imtype == "-" and "&#8211;" or self.oi.imtype, self.oi.istype == "-" and "&#8211;" or self.oi.istype, self.oi.cstype == "-" and "&#8211;" or self.oi.cstype, self.oi.fetype == "-" and "&#8211;" or self.oi.fetype)
				# adding RSP0 chi-square and profile
				self.infohtml = self.infohtml + "\n <td align=center>%s</td>" % (self.chi_array[0])
				if self.filestem_array[0] == "":
					self.infohtml = self.infohtml + "\n <td align=center></td>"
				else:
					self.infohtml = self.infohtml + "\n <td align=center><a href=\"plots/%s/%s.png\"><img width=200 height=140 src=\"plots/%s/%s.th.png\"></a></td>" % (self.id, self.filestem_array[0], self.id, self.filestem_array[0])
				# adding RSPA chi-square and profile
				self.infohtml = self.infohtml + "\n <td align=center>%s</td>" % (self.chi_array[1])
				if self.filestem_array[1] == "":
					self.infohtml = self.infohtml + "\n <td align=center></td>"
				else:
					self.infohtml = self.infohtml + "\n <td align=center><a href=\"plots/%s/%s.png\"><img width=200 height=140 src=\"plots/%s/%s.th.png\"></a></td>" % (self.id, self.filestem_array[1], self.id, self.filestem_array[1])

				# adding combined_plot column
				if self.combined_plot != "":
					self.infohtml = self.infohtml + "\n <td align=center><a href=\"plots/%s/%s.png\"><img width=200 height=140 src=\"plots/%s/%s.th.png\"></a></td>" % (self.id, self.combined_plot, self.id, self.combined_plot)
				else:
					self.infohtml = self.infohtml + "\n <td align=center></td>"
				# adding the rest (columns) of the table
				if viewtype == "plots": self.infohtml = self.infohtml + "\n <td align=left>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>" % (self.redlocation, self.statusline.replace("-", "&#8211;"), self.archivestatus == "x" and self.archivestatus or "<a href=\"grid/%s.txt\">%s</a>" % (self.id, self.archivestatus))
				if viewtype == "mega":
					self.infohtml = self.infohtml + "\n <td>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=left style=\"white-space: nowrap;\">%s</td>\n <td align=left>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>" % (self.oi.nodeslist, self.oi.datadir, self.dirsize_string_html, self.totsize, self.oi.stations_html, self.redlocation, self.statusline.replace("-", "&#8211;"), self.archivestatus == "x" and self.archivestatus or "<a href=\"grid/%s.txt\">%s</a>" % (self.id, self.archivestatus))
			else:
				self.infohtml="<td>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>\n <td align=center>%s</td>" % (self.id, self.oi.datestring, self.oi.duration, self.oi.nodeslist, self.oi.datadir, self.dirsize_string_html, self.totsize, self.oi.bftype == "-" and "&#8211;" or self.oi.bftype, self.oi.fdtype == "-" and "&#8211;" or self.oi.fdtype, self.oi.imtype == "-" and "&#8211;" or self.oi.imtype, self.oi.istype == "-" and "&#8211;" or self.oi.istype, self.oi.cstype == "-" and "&#8211;" or self.oi.cstype, self.oi.fetype == "-" and "&#8211;" or self.oi.fetype, self.statusline.replace("-", "&#8211;"), self.oi.pointing, self.oi.source)
		else:
			self.info = self.comment
			self.infohtml = "<td>%s</td>\n <td colspan=%d align=left>%s</td>" % (self.id, self.colspan, self.comment,)





# Class with functions to write Html obs list
class writeHtmlList:
	def __init__(self, file, stem, fd, td):
		self.htmlfile = file
		self.linkedhtmlstem = stem
		self.fd = fd
		self.td = td
		self.lupd = ""

	def reInit (self, file):
		self.htmlfile = file

	def open(self):
		self.htmlptr = open(self.htmlfile, 'w')
		self.htmlptr.write ("<html>\n\
                	         <head>\n\
                                  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
                          	  <meta name=\"Classification\" content=\"public HTML\">\n\
                                  <meta name=\"robots\" content=\"noindex, nofollow\">\n\
				  <base href=\"http://www.astron.nl/~kondratiev/lofar/\" />\n\
                          	  <title>LOFAR pulsar observations</title>\n\
                         	</head>\n\n\
                         	<style type='text/css'>\n\
                          	 tr.d0 td { background-color: #ccffff; color: black; font-size: 80% }\n\
                          	 tr.d1 td { background-color: #99cccc; color: black; font-size: 80% }\n\
                          	 tr.d th { background-color: #99cccc; color: black;}\n\
                         	</style>\n\n\
                         	<body bgcolor='white'>\n\
                          	<h2 align=left>LOFAR pulsar observations</h2>\n\
                        	\n")

	def obsnumber (self, storage_nodes, subclusters, ndbnodes, nnodes):
		self.nodes_string = ", ".join(storage_nodes)
		self.subs_string = ", ".join(subclusters)
		self.htmlptr.write("Number of observations in %s: <b>%d</b><br>\n" % (self.subs_string, ndbnodes, ))
		self.htmlptr.write("Number of new observations found in %s: <b>%d</b><br>\n" % (self.nodes_string, nnodes))

	def datesrange (self):
		if self.fd == "":
			if self.td != "":
				self.htmlptr.write ("List only observations till %s<br>\n" % (self.td, ))
		else:
			if self.td == "":
				self.htmlptr.write ("List only observations since %s<br>\n" % (self.fd, ))
			else:
				self.htmlptr.write ("List only observations since %s till %s<br>\n" % (self.fd, self.td))

	def statistics (self, link):
		self.htmlptr.write ("<a href=\"%s\">Statistics</a>\n" % (link.split("/")[-1]))

	def header (self, viewtype, storage_nodes_string_html):
		self.htmlptr.write ("\n<p align=left>\n<table border=0 cellspacing=0 cellpadding=3>\n")
		if viewtype == "brief":
			self.htmlptr.write ("\n<tr class='d' align=left>\n <th>No.</th>\n <th>ObsID</th>\n <th align=center>Source</th>\n <th align=center>MMDD</th>\n <th align=center>Duration</th>\n <th align=center>Antenna</th>\n <th align=center>Band</th>\n <th align=center>#Stations</th>\n <th align=center>BF</th>\n <th align=center>FD</th>\n <th align=center>IM</th>\n <th align=center>IS</th>\n <th align=center>CS</th>\n <th align=center>FE</th>\n <th align=left>Location</th>\n</tr>\n")
		elif viewtype == "plots":
			self.htmlptr.write ("\n<tr class='d' align=left>\n <th>No.</th>\n <th>ObsID</th>\n <th align=center>Source</th>\n <th align=center>MMDD</th>\n <th align=center>Duration</th>\n <th align=center>Antenna</th>\n <th align=center>Band</th>\n <th align=center>#Stations</th>\n <th align=center>BF</th>\n <th align=center>FD</th>\n <th align=center>IM</th>\n <th align=center>IS</th>\n <th align=center>CS</th>\n <th align=center>FE</th>\n <th align=center>Chi-squared (RSP0)</th>\n <th align=center>Profile (RSP0)</th>\n <th align=center>Chi-squared (RSPA)</th>\n <th align=center>Profile (RSPA)</th>\n <th align=center>Combined</th>\n <th align=left>Location</th>\n <th align=center>Status</th>\n <th align=center>Archive</th>\n</tr>\n")
		elif viewtype == "mega":
			self.htmlptr.write ("\n<tr class='d' align=left>\n <th>No.</th>\n <th>ObsID</th>\n <th align=center>Source</th>\n <th align=center>MMDD</th>\n <th align=center>Duration</th>\n <th align=center>Antenna</th>\n <th align=center>Band</th>\n <th align=center>#Stations</th>\n <th align=center>BF</th>\n <th align=center>FD</th>\n <th align=center>IM</th>\n <th align=center>IS</th>\n <th align=center>CS</th>\n <th align=center>FE</th>\n <th align=center>Chi-squared (RSP0)</th>\n <th align=center>Profile (RSP0)</th>\n <th align=center>Chi-squared (RSPA)</th>\n <th align=center>Profile (RSPA)</th>\n <th align=center>Combined</th>\n <th align=center>NodesList (lse)</th>\n <th align=center>Raw Datadir</th>\n <th align=center>%s</th>\n <th align=center>Total (GB)</th>\n <th align=center style=\"white-space: nowrap;\">Stations</th>\n <th align=left>Location</th>\n <th align=center>Status</th>\n <th align=center>Archive</th>\n</tr>\n" % (storage_nodes_string_html,))
		else:
			self.htmlptr.write ("\n<tr class='d' align=left>\n <th>No.</th>\n <th>ObsID</th>\n <th align=center>MMDD</th>\n <th align=center>Duration</th>\n <th align=center>NodesList (lse)</th>\n <th align=center>Raw Datadir</th>\n <th align=center>%s</th>\n <th align=center>Total (GB)</th>\n <th align=center>BF</th>\n <th align=center>FD</th>\n <th align=center>IM</th>\n <th align=center>IS</th>\n <th align=center>CS</th>\n <th align=center>FE</th>\n <th align=center>Status</th>\n <th align=center>Pointing</th>\n <th align=center>Source</th>\n</tr>\n" % (storage_nodes_string_html,))

	def linkedheader (self, viewtype, storage_nodes_string_html):
		sf=["-obsid.html", "-time.html", "-size.html", "-source.html"]
		sf=["%s%s" % (self.linkedhtmlstem.split("/")[-1], i) for i in sf]
		self.htmlptr.write ("\n<p align=left>\n<table border=0 cellspacing=0 cellpadding=3>\n")
		if viewtype == "brief":
			self.htmlptr.write ("\n<tr class='d' align=left>\n <th>No.</th>\n <th><a href=\"%s\">ObsID</a></th>\n <th align=center><a href=\"%s\">Source</a></th>\n <th align=center><a href=\"%s\">MMDD</a></th>\n <th align=center>Duration</th>\n <th align=center>Antenna</th>\n <th align=center>Band</th>\n <th align=center>#Stations</th>\n <th align=center>BF</th>\n <th align=center>FD</th>\n <th align=center>IM</th>\n <th align=center>IS</th>\n <th align=center>CS</th>\n <th align=center>FE</th>\n <th align=left>Location</th>\n</tr>\n" % (sf[0], sf[3], sf[1]))
		elif viewtype == "plots":
			self.htmlptr.write ("\n<tr class='d' align=left>\n <th>No.</th>\n <th><a href=\"%s\">ObsID</a></th>\n <th align=center><a href=\"%s\">Source</a></th>\n <th align=center><a href=\"%s\">MMDD</a></th>\n <th align=center>Duration</th>\n <th align=center>Antenna</th>\n <th align=center>Band</th>\n <th align=center>#Stations</th>\n <th align=center>BF</th>\n <th align=center>FD</th>\n <th align=center>IM</th>\n <th align=center>IS</th>\n <th align=center>CS</th>\n <th align=center>FE</th>\n <th align=center>Chi-squared (RSP0)</th>\n <th align=center>Profile (RSP0)</th>\n <th align=center>Chi-squared (RSPA)</th>\n <th align=center>Profile (RSPA)</th>\n <th align=center>Combined</th>\n <th align=left>Location</th>\n <th align=center>Status</th>\n <th align=center>Archive</th>\n</tr>\n " % (sf[0], sf[3], sf[1]))
		elif viewtype == "mega":
			self.htmlptr.write ("\n<tr class='d' align=left>\n <th>No.</th>\n <th><a href=\"%s\">ObsID</a></th>\n <th align=center><a href=\"%s\">Source</a></th>\n <th align=center><a href=\"%s\">MMDD</a></th>\n <th align=center>Duration</th>\n <th align=center>Antenna</th>\n <th align=center>Band</th>\n <th align=center>#Stations</th>\n <th align=center>BF</th>\n <th align=center>FD</th>\n <th align=center>IM</th>\n <th align=center>IS</th>\n <th align=center>CS</th>\n <th align=center>FE</th>\n <th align=center>Chi-squared (RSP0)</th>\n <th align=center>Profile (RSP0)</th>\n <th align=center>Chi-squared (RSPA)</th>\n <th align=center>Profile (RSPA)</th>\n <th align=center>Combined</th>\n <th align=center>NodesList (lse)</th>\n <th align=center>Raw Datadir</th>\n <th align=center>%s</th>\n <th align=center><a href=\"%s\">Total (GB)</a></th>\n <th align=center style=\"white-space: nowrap;\">Stations</th>\n <th align=left>Location</th>\n <th align=center>Status</th>\n <th align=center>Archive</th>\n</tr>\n" % (sf[0], sf[3], sf[1], storage_nodes_string_html, sf[2]))
		else:
			self.htmlptr.write ("\n<tr class='d' align=left>\n <th>No.</th>\n <th><a href=\"%s\">ObsID</a></th>\n <th align=center><a href=\"%s\">MMDD</a></th>\n <th align=center>Duration</th>\n <th align=center>NodesList (lse)</th>\n <th align=center>Raw Datadir</th>\n <th align=center>%s</th>\n <th align=center><a href=\"%s\">Total (GB)</a></th>\n <th align=center>BF</th>\n <th align=center>FD</th>\n <th align=center>IM</th>\n <th align=center>IS</th>\n <th align=center>CS</th>\n <th align=center>FE</th>\n <th align=center>Status</th>\n <th align=center><a href=\"%s\">Pointing</a></th>\n <th align=center><a href=\"%s\">Source</a></th>\n</tr>\n" % (sf[0], sf[1], storage_nodes_string_html, sf[2], sf[3], sf[3]))

	def record (self, lineclass, index, line):
		self.htmlptr.write ("\n<tr class='%s' align=left>\n <td>%d</td>\n %s\n</tr>" % (lineclass, index, line))

	def legend (self):
		# getting date & time of last update
		if self.lupd == "":
			cmd="date +'%b %d, %Y %H:%M:%S'"
			self.lupd=os.popen(cmd).readlines()[0][:-1]
		self.htmlptr.write ("\n</table>\n\n<hr width=100%%>\n<address>\nLast Updated: %s\n</address>\n" % (self.lupd, ))
		self.htmlptr.write ("\n</body>\n</html>")

	def close (self):
		self.htmlptr.close()


# Class that keeps the statistics of all data
class obsstat:
	def __init__(self, ids, fd, td, storage_nodes):
		self.ids = ids
		self.fd = fd
		self.td = td
		self.storage_nodes = storage_nodes
		self.lupd = ""

		self.subclusters = np.append(np.unique([cexec_nodes[s].split(":")[0] for s in self.storage_nodes]), ["subA", "sub?"])
		self.dbinfo = {}
		for sub in np.append(self.subclusters, ["Total"]):
			self.dbinfo[sub] = {"totDuration": 0.0, "processedDuration": 0.0, "IMonlyDuration": 0.0,
                                            "Ntotal": 0, "Nprocessed": 0, "Nistype": 0, "Nistype_only": 0, "Ncstype": 0, "Ncstype_only": 0,
                                            "Nfetype": 0, "Nfetype_only": 0, "Nimtype": 0, "Nimtype_only": 0,
                                            "Nbftype": 0, "Nbftype_only": 0, "Nfdtype": 0, "Nfdtype_only": 0,
                                            "Niscsim": 0, "Nisim": 0, "Niscs": 0, "Ncsim": 0, "Ncsfe": 0, "Nimfe": 0, "Nisfe": 0, "Niscsfe": 0, 
                                            "Nbfis": 0, "Nbffe": 0, "Nbfisfe": 0, "Nbfiscsfe": 0,
                                            "totRawsize": 0.0, "IMonlyRawsize": 0.0, "totProcessedsize": 0.0 }	 # size in TB

		for sub in self.subclusters:
			self.subkeys=np.compress(np.array([obstable[r].subcluster for r in self.ids]) == sub, self.ids)
			self.dbinfo[sub]["Ntotal"] += np.size(self.subkeys)
			for r in self.subkeys:
				# getting the numbers and duration
				if obstable[r].comment == "" and obstable[r].oi.duration != "?":
					self.dbinfo[sub]["totDuration"] += obstable[r].oi.dur
				if obstable[r].comment == "" and obstable[r].statusline != "x":
					self.dbinfo[sub]["Nprocessed"] += 1
					if obstable[r].oi.duration != "?":
						self.dbinfo[sub]["processedDuration"] += obstable[r].oi.dur
				# getting the number of obs of different type
				if obstable[r].comment == "" and obstable[r].oi.istype == "+":
					self.dbinfo[sub]["Nistype"] += 1
					if obstable[r].oi.cstype != "+" and obstable[r].oi.fetype != "+" and obstable[r].oi.imtype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.bftype != "+":
						self.dbinfo[sub]["Nistype_only"] += 1
				if obstable[r].comment == "" and obstable[r].oi.cstype == "+":
					self.dbinfo[sub]["Ncstype"] += 1
					if obstable[r].oi.istype != "+" and obstable[r].oi.fetype != "+" and obstable[r].oi.imtype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.bftype != "+":
						self.dbinfo[sub]["Ncstype_only"] += 1
				if obstable[r].comment == "" and obstable[r].oi.fetype == "+":
					self.dbinfo[sub]["Nfetype"] += 1
					if obstable[r].oi.istype != "+" and obstable[r].oi.cstype != "+" and obstable[r].oi.imtype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.bftype != "+":
						self.dbinfo[sub]["Nfetype_only"] += 1
				if obstable[r].comment == "" and obstable[r].oi.imtype == "+":
					self.dbinfo[sub]["Nimtype"] += 1
					if obstable[r].oi.cstype != "+" and obstable[r].oi.fetype != "+" and obstable[r].oi.istype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.bftype != "+":
						self.dbinfo[sub]["Nimtype_only"] += 1
						self.dbinfo[sub]["IMonlyRawsize"] += float(obstable[r].totsize)
						if obstable[r].oi.duration != "?":
							self.dbinfo[sub]["IMonlyDuration"] += obstable[r].oi.dur
				if obstable[r].comment == "" and obstable[r].oi.bftype == "+":
					self.dbinfo[sub]["Nbftype"] += 1
					if obstable[r].oi.cstype != "+" and obstable[r].oi.fetype != "+" and obstable[r].oi.istype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.imtype != "+":
						self.dbinfo[sub]["Nbftype_only"] += 1
				if obstable[r].comment == "" and obstable[r].oi.fdtype == "+":
					self.dbinfo[sub]["Nfdtype"] += 1
					if obstable[r].oi.cstype != "+" and obstable[r].oi.fetype != "+" and obstable[r].oi.istype != "+" and obstable[r].oi.bftype != "+" and obstable[r].oi.imtype != "+":
						self.dbinfo[sub]["Nfdtype_only"] += 1
				# getting the number of some observing types' mixtures
				if obstable[r].comment == "" and obstable[r].oi.istype == "+" and obstable[r].oi.cstype == "+" and obstable[r].oi.imtype == "+" and obstable[r].oi.fetype != "+" and obstable[r].oi.bftype != "+" and obstable[r].oi.fdtype != "+":
					self.dbinfo[sub]["Niscsim"] += 1
				if obstable[r].comment == "" and obstable[r].oi.istype == "+" and obstable[r].oi.imtype == "+" and obstable[r].oi.fetype != "+" and obstable[r].oi.bftype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.cstype != "+":
					self.dbinfo[sub]["Nisim"] += 1
				if obstable[r].comment == "" and obstable[r].oi.istype == "+" and obstable[r].oi.cstype == "+" and obstable[r].oi.fetype != "+" and obstable[r].oi.bftype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.imtype != "+":
					self.dbinfo[sub]["Niscs"] += 1
				if obstable[r].comment == "" and obstable[r].oi.cstype == "+" and obstable[r].oi.imtype == "+" and obstable[r].oi.fetype != "+" and obstable[r].oi.bftype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.istype != "+":
					self.dbinfo[sub]["Ncsim"] += 1
				if obstable[r].comment == "" and obstable[r].oi.cstype == "+" and obstable[r].oi.fetype == "+" and obstable[r].oi.imtype != "+" and obstable[r].oi.bftype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.istype != "+":
					self.dbinfo[sub]["Ncsfe"] += 1
				if obstable[r].comment == "" and obstable[r].oi.istype == "+" and obstable[r].oi.fetype == "+" and obstable[r].oi.imtype != "+" and obstable[r].oi.bftype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.cstype != "+":
					self.dbinfo[sub]["Nisfe"] += 1
				if obstable[r].comment == "" and obstable[r].oi.imtype == "+" and obstable[r].oi.fetype == "+" and obstable[r].oi.istype != "+" and obstable[r].oi.bftype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.cstype != "+":
					self.dbinfo[sub]["Nimfe"] += 1
				if obstable[r].comment == "" and obstable[r].oi.istype == "+" and obstable[r].oi.cstype == "+" and obstable[r].oi.fetype == "+" and obstable[r].oi.imtype != "+" and obstable[r].oi.bftype != "+" and obstable[r].oi.fdtype != "+":
					self.dbinfo[sub]["Niscsfe"] += 1
				if obstable[r].comment == "" and obstable[r].oi.bftype == "+" and obstable[r].oi.istype == "+" and obstable[r].oi.imtype != "+" and obstable[r].oi.cstype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.fetype != "+":
					self.dbinfo[sub]["Nbfis"] += 1
				if obstable[r].comment == "" and obstable[r].oi.bftype == "+" and obstable[r].oi.fetype == "+" and obstable[r].oi.istype != "+" and obstable[r].oi.cstype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.imtype != "+":
					self.dbinfo[sub]["Nbffe"] += 1
				if obstable[r].comment == "" and obstable[r].oi.bftype == "+" and obstable[r].oi.istype == "+" and obstable[r].oi.fetype == "+" and obstable[r].oi.cstype != "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.imtype != "+":
					self.dbinfo[sub]["Nbfisfe"] += 1
				if obstable[r].comment == "" and obstable[r].oi.bftype == "+" and obstable[r].oi.istype == "+" and obstable[r].oi.fetype == "+" and obstable[r].oi.cstype == "+" and obstable[r].oi.fdtype != "+" and obstable[r].oi.imtype != "+":
					self.dbinfo[sub]["Nbfiscsfe"] += 1
				# getting the sizes
				if obstable[r].comment == "":
					self.dbinfo[sub]["totRawsize"] += float(obstable[r].totsize)
				if obstable[r].comment == "":
					self.dbinfo[sub]["totProcessedsize"] += float(obstable[r].processed_dirsize)

			self.dbinfo[sub]["totDuration"] /= 3600.
			self.dbinfo[sub]["processedDuration"] /= 3600.
			self.dbinfo[sub]["IMonlyDuration"] /= 3600.
			self.dbinfo[sub]["totRawsize"] /= 1024.
			self.dbinfo[sub]["totProcessedsize"] /= 1024.
			self.dbinfo[sub]["IMonlyRawsize"] /= 1024.

		for sub in self.subclusters:
			self.dbinfo["Total"]["totDuration"] += self.dbinfo[sub]["totDuration"]
			self.dbinfo["Total"]["processedDuration"] += self.dbinfo[sub]["processedDuration"]
			self.dbinfo["Total"]["IMonlyDuration"] += self.dbinfo[sub]["IMonlyDuration"]
			self.dbinfo["Total"]["totRawsize"] += self.dbinfo[sub]["totRawsize"]
			self.dbinfo["Total"]["totProcessedsize"] += self.dbinfo[sub]["totProcessedsize"]
			self.dbinfo["Total"]["IMonlyRawsize"] += self.dbinfo[sub]["IMonlyRawsize"]
			self.dbinfo["Total"]["Ntotal"] += self.dbinfo[sub]["Ntotal"]
			self.dbinfo["Total"]["Nprocessed"] += self.dbinfo[sub]["Nprocessed"]
			self.dbinfo["Total"]["Nistype"] += self.dbinfo[sub]["Nistype"]
			self.dbinfo["Total"]["Nistype_only"] += self.dbinfo[sub]["Nistype_only"]
			self.dbinfo["Total"]["Ncstype"] += self.dbinfo[sub]["Ncstype"]
			self.dbinfo["Total"]["Ncstype_only"] += self.dbinfo[sub]["Ncstype_only"]
			self.dbinfo["Total"]["Nfetype"] += self.dbinfo[sub]["Nfetype"]
			self.dbinfo["Total"]["Nfetype_only"] += self.dbinfo[sub]["Nfetype_only"]
			self.dbinfo["Total"]["Nimtype"] += self.dbinfo[sub]["Nimtype"]
			self.dbinfo["Total"]["Nimtype_only"] += self.dbinfo[sub]["Nimtype_only"]
			self.dbinfo["Total"]["Nbftype"] += self.dbinfo[sub]["Nbftype"]
			self.dbinfo["Total"]["Nbftype_only"] += self.dbinfo[sub]["Nbftype_only"]
			self.dbinfo["Total"]["Nfdtype"] += self.dbinfo[sub]["Nfdtype"]
			self.dbinfo["Total"]["Nfdtype_only"] += self.dbinfo[sub]["Nfdtype_only"]
			self.dbinfo["Total"]["Niscsim"] += self.dbinfo[sub]["Niscsim"]
			self.dbinfo["Total"]["Nisim"] += self.dbinfo[sub]["Nisim"]
			self.dbinfo["Total"]["Niscs"] += self.dbinfo[sub]["Niscs"]
			self.dbinfo["Total"]["Ncsim"] += self.dbinfo[sub]["Ncsim"]
			self.dbinfo["Total"]["Ncsfe"] += self.dbinfo[sub]["Ncsfe"]
			self.dbinfo["Total"]["Nimfe"] += self.dbinfo[sub]["Nimfe"]
			self.dbinfo["Total"]["Nisfe"] += self.dbinfo[sub]["Nisfe"]
			self.dbinfo["Total"]["Niscsfe"] += self.dbinfo[sub]["Niscsfe"]
			self.dbinfo["Total"]["Nbfis"] += self.dbinfo[sub]["Nbfis"]
			self.dbinfo["Total"]["Nbffe"] += self.dbinfo[sub]["Nbffe"]
			self.dbinfo["Total"]["Nbfisfe"] += self.dbinfo[sub]["Nbfisfe"]
			self.dbinfo["Total"]["Nbfiscsfe"] += self.dbinfo[sub]["Nbfiscsfe"]
			

	def printstat (self):
		print
		print "Pulsar observations' statistics:"
		if self.fd != "" or self.td != "":
			print "[%s%s]" % (self.fd != "" and "from " + self.fd or (self.td != "" and " till " + self.td or ""), 
                                                                    self.td != "" and (self.fd != "" and " till " + self.td or "") or "")
		ruler_size=53+23*(1 + np.size(self.subclusters))
		rulers=[]
		for r in np.arange(ruler_size): rulers = np.append(rulers, "-")
		rulerstring="".join(rulers) + "-"
		print rulerstring
		header_string="                                                     "
		for sub in np.append("Total", self.subclusters):
			symbols=[]
			for r in np.arange(23-len(sub)): symbols = np.append(symbols, " ")
			header_string += "%s%s" % (sub, "".join(symbols))
		print header_string
		print rulerstring

		line="Total number of observations [hours / days]:         "
		for sub in np.append("Total", self.subclusters):
			field = "%d [%.1f / %.1f]" % (self.dbinfo[sub]["Ntotal"], self.dbinfo[sub]["totDuration"], self.dbinfo[sub]["totDuration"]/24.)
			line += "%-23s" % (field)
		print line
		line="Number of observations w/o IM-only [hours / days]:   "
		for sub in np.append("Total", self.subclusters):
			field = "%d [%.1f / %.1f]" % (self.dbinfo[sub]["Ntotal"]-self.dbinfo[sub]["Nimtype_only"], self.dbinfo[sub]["totDuration"]-self.dbinfo[sub]["IMonlyDuration"], (self.dbinfo[sub]["totDuration"]-self.dbinfo[sub]["IMonlyDuration"])/24.)
			line += "%-23s" % (field)
		print line
		line="Number of processed observations [hours / days]:     "
		for sub in np.append("Total", self.subclusters):
			field = "%d [%.1f / %.1f]" % (self.dbinfo[sub]["Nprocessed"], self.dbinfo[sub]["processedDuration"], self.dbinfo[sub]["processedDuration"]/24.)
			line += "%-23s" % (field)
		print line

		print

		line="Number of IS observations [only IS]:                 "
		for sub in np.append("Total", self.subclusters):
			field = "%d [%d]" % (self.dbinfo[sub]["Nistype"], self.dbinfo[sub]["Nistype_only"])
			line += "%-23s" % (field)
		print line
		line="Number of IS+CS observations only:                   "
		for sub in np.append("Total", self.subclusters):
			field = "%d" % (self.dbinfo[sub]["Niscs"])
			line += "%-23s" % (field)
		print line
		line="Number of IS+IM observations only:                   "
		for sub in np.append("Total", self.subclusters):
			field = "%d" % (self.dbinfo[sub]["Nisim"])
			line += "%-23s" % (field)
		print line
		line="Number of IS+CS+IM observations only:                "
		for sub in np.append("Total", self.subclusters):
			field = "%d" % (self.dbinfo[sub]["Niscsim"])
			line += "%-23s" % (field)
		print line
		line="Number of CS observations [only CS]:                 "
		for sub in np.append("Total", self.subclusters):
			field = "%d [%d]" % (self.dbinfo[sub]["Ncstype"], self.dbinfo[sub]["Ncstype_only"])
			line += "%-23s" % (field)
		print line
		line="Number of CS+IM observations only:                   "
		for sub in np.append("Total", self.subclusters):
			field = "%d" % (self.dbinfo[sub]["Ncsim"])
			line += "%-23s" % (field)
		print line
		line="Number of FE observations [only FE]:                 "
		for sub in np.append("Total", self.subclusters):
			field = "%d [%d]" % (self.dbinfo[sub]["Nfetype"], self.dbinfo[sub]["Nfetype_only"])
			line += "%-23s" % (field)
		print line
		line="Number of FE+CS observations only:                   "
		for sub in np.append("Total", self.subclusters):
			field = "%d" % (self.dbinfo[sub]["Ncsfe"])
			line += "%-23s" % (field)
		print line
		line="Number of FE+IS observations only:                   "
		for sub in np.append("Total", self.subclusters):
			field = "%d" % (self.dbinfo[sub]["Nisfe"])
			line += "%-23s" % (field)
		print line
		line="Number of FE+IM observations only:                   "
		for sub in np.append("Total", self.subclusters):
			field = "%d" % (self.dbinfo[sub]["Nimfe"])
			line += "%-23s" % (field)
		print line
		line="Number of FE+IS+CS observations only:                "
		for sub in np.append("Total", self.subclusters):
			field = "%d" % (self.dbinfo[sub]["Niscsfe"])
			line += "%-23s" % (field)
		print line
		line="Number of IM observations [only IM]:                 "
		for sub in np.append("Total", self.subclusters):
			field = "%d [%d]" % (self.dbinfo[sub]["Nimtype"], self.dbinfo[sub]["Nimtype_only"])
			line += "%-23s" % (field)
		print line
		line="Number of BF observations [only BF]:                 "
		for sub in np.append("Total", self.subclusters):
			field = "%d [%d]" % (self.dbinfo[sub]["Nbftype"], self.dbinfo[sub]["Nbftype_only"])
			line += "%-23s" % (field)
		print line
		line="Number of BF+IS observations only:                   "
		for sub in np.append("Total", self.subclusters):
			field = "%d" % (self.dbinfo[sub]["Nbfis"])
			line += "%-23s" % (field)
		print line
		line="Number of BF+FE observations only:                   "
		for sub in np.append("Total", self.subclusters):
			field = "%d" % (self.dbinfo[sub]["Nbffe"])
			line += "%-23s" % (field)
		print line
		line="Number of BF+IS+FE observations only:                "
		for sub in np.append("Total", self.subclusters):
			field = "%d" % (self.dbinfo[sub]["Nbfisfe"])
			line += "%-23s" % (field)
		print line
		line="Number of BF+IS+CS+FE observations only:             "
		for sub in np.append("Total", self.subclusters):
			field = "%d" % (self.dbinfo[sub]["Nbfiscsfe"])
			line += "%-23s" % (field)
		print line
		line="Number of FD observations [only FD]:                 "
		for sub in np.append("Total", self.subclusters):
			field = "%d [%d]" % (self.dbinfo[sub]["Nfdtype"], self.dbinfo[sub]["Nfdtype_only"])
			line += "%-23s" % (field)
		print line

		print

		line="Total size of raw data (TB):                         "
		for sub in np.append("Total", self.subclusters):
			field = "%.1f" % (self.dbinfo[sub]["totRawsize"])
			line += "%-23s" % (field)
		print line
		line="Total size of raw data w/o IM-only (TB):             "
		for sub in np.append("Total", self.subclusters):
			field = "%.1f" % (self.dbinfo[sub]["totRawsize"]-self.dbinfo[sub]["IMonlyRawsize"])
			line += "%-23s" % (field)
		print line
		line="Total size of processed data (TB):                   "
		for sub in np.append("Total", self.subclusters):
			field = "%.1f" % (self.dbinfo[sub]["totProcessedsize"])
			line += "%-23s" % (field)
		print line
		print

	def printhtml (self, htmlfile):
		self.htmlptr = open(htmlfile, 'w')
		self.htmlptr.write ("<html>\n\
                	         <head>\n\
                                  <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\">\n\
                          	  <meta name=\"Classification\" content=\"public HTML\">\n\
                                  <meta name=\"robots\" content=\"noindex, nofollow\">\n\
				  <base href=\"http://www.astron.nl/~kondratiev/lofar/\" />\n\
                          	  <title>Pulsar observations statistics</title>\n\
                         	</head>\n\n\
                         	<style type='text/css'>\n\
                          	 tr.d0 td { background-color: #ccffff; color: black; font-size: 100% }\n\
                          	 tr.d1 td { background-color: #99cccc; color: black; font-size: 100% }\n\
                          	 tr.d th { background-color: #99cccc; color: black;}\n\
                         	</style>\n\n\
                         	<body bgcolor='white'>\n\
                          	<h2 align=left>Pulsar observations statistics</h2>\n\n\
				<a href=\"archive/number.png\">Number of observations over time</a><br>\n\
				<a href=\"archive/volume.png\">Data volume over time</a><br>\n\
                        	\n")
		if self.fd != "" or self.td != "":
			self.htmlptr.write ("\n<h3>%s%s</h3>" % (self.fd != "" and "From " + self.fd or (self.td != "" and " Till " + self.td or ""), 
                                                                    self.td != "" and (self.fd != "" and " Till " + self.td or "") or ""))

		self.htmlptr.write ("\n<p align=left>\n<table border=0 cellspacing=0 cellpadding=5>")
		self.htmlptr.write ("\n<tr class='d1' align=left>\n <td align=left width=400></td>")
		for sub in np.append("Total", self.subclusters):	
			self.htmlptr.write ("\n <td align=left><b>%s</b></td>" % (sub))	
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d0' align=left>\n <td align=left>%s [<font color=\"brown\"><b>%s</b></font> / <font color=\"green\"><b>%s</b></font>]</td>" % ("Total number of observations", "hours", "days"))
		for sub in np.append("Total", self.subclusters):	
			self.htmlptr.write ("\n <td align=left><b>%d</b> [<font color=\"brown\"><b>%.1f</b></font> / <font color=\"green\"><b>%.1f</b></font>]</td>" % (self.dbinfo[sub]["Ntotal"], self.dbinfo[sub]["totDuration"], self.dbinfo[sub]["totDuration"]/24.))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d1' align=left>\n <td align=left>%s [<font color=\"brown\"><b>%s</b></font> / <font color=\"green\"><b>%s</b></font>]</td>" % ("Number of observations w/o IM-only", "hours", "days"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b> [<font color=\"brown\"><b>%.1f</b></font> / <font color=\"green\"><b>%.1f</b></font>]</td>" % (self.dbinfo[sub]["Ntotal"]-self.dbinfo[sub]["Nimtype_only"], self.dbinfo[sub]["totDuration"]-self.dbinfo[sub]["IMonlyDuration"], (self.dbinfo[sub]["totDuration"]-self.dbinfo[sub]["IMonlyDuration"])/24.))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d0' align=left>\n <td align=left>%s [<font color=\"brown\"><b>%s</b></font> / <font color=\"green\"><b>%s</b></font>]</td>" % ("Number of processed observations", "hours", "days"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b> [<font color=\"brown\"><b>%.1f</b></font> / <font color=\"green\"><b>%.1f</b></font>]</td>" % (self.dbinfo[sub]["Nprocessed"], self.dbinfo[sub]["processedDuration"], self.dbinfo[sub]["processedDuration"]/24.))
		self.htmlptr.write ("\n</tr>")

		self.htmlptr.write ("\n<tr align=left height=25>\n <td align=left></td>")
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left></td>")
		self.htmlptr.write ("\n</tr>")

		self.htmlptr.write ("\n<tr class='d1' align=left>\n <td align=left>%s [<font color=\"brown\"><b>%s</b></font>]</td>" % ("Number of IS observations", "only IS"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b> [<font color=\"brown\"><b>%d</b></font>]</td>" % (self.dbinfo[sub]["Nistype"], self.dbinfo[sub]["Nistype_only"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d0' align=left>\n <td align=left>%s</td>" % ("Number of IS+CS observations only"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b></td>" % (self.dbinfo[sub]["Niscs"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d1' align=left>\n <td align=left>%s</td>" % ("Number of IS+IM observations only"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b></td>" % (self.dbinfo[sub]["Nisim"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d0' align=left>\n <td align=left>%s</td>" % ("Number of IS+CS+IM observations only"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b></td>" % (self.dbinfo[sub]["Niscsim"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d1' align=left>\n <td align=left>%s [<font color=\"brown\"><b>%s</b></font>]</td>" % ("Number of CS observations", "only CS"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b> [<font color=\"brown\"><b>%d<b></font>]</td>" % (self.dbinfo[sub]["Ncstype"], self.dbinfo[sub]["Ncstype_only"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d0' align=left>\n <td align=left>%s</td>" % ("Number of CS+IM observations only"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b></td>" % (self.dbinfo[sub]["Ncsim"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d1' align=left>\n <td align=left>%s [<font color=\"brown\"><b>%s</b></font>]</td>" % ("Number of FE observations", "only FE"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b> [<font color=\"brown\"><b>%d</b></font>]</td>" % (self.dbinfo[sub]["Nfetype"], self.dbinfo[sub]["Nfetype_only"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d0' align=left>\n <td align=left>%s</td>" % ("Number of FE+CS observations only"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b></td>" % (self.dbinfo[sub]["Ncsfe"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d1' align=left>\n <td align=left>%s</td>" % ("Number of FE+IS observations only"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b></td>" % (self.dbinfo[sub]["Nisfe"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d0' align=left>\n <td align=left>%s</td>" % ("Number of FE+IM observations only"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b></td>" % (self.dbinfo[sub]["Nimfe"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d1' align=left>\n <td align=left>%s</td>" % ("Number of FE+IS+CS observations only"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b></td>" % (self.dbinfo[sub]["Niscsfe"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d0' align=left>\n <td align=left>%s [<font color=\"brown\"><b>%s</b></font>]</td>" % ("Number of IM observations", "only IM"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b> [<font color=\"brown\"><b>%d</b></font>]</td>" % (self.dbinfo[sub]["Nimtype"], self.dbinfo[sub]["Nimtype_only"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d1' align=left>\n <td align=left>%s [<font color=\"brown\"><b>%s</b></font>]</td>" % ("Number of BF observations", "only BF"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b> [<font color=\"brown\"><b>%d</b></font>]</td>" % (self.dbinfo[sub]["Nbftype"], self.dbinfo[sub]["Nbftype_only"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d0' align=left>\n <td align=left>%s</td>" % ("Number of BF+IS observations only"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b></td>" % (self.dbinfo[sub]["Nbfis"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d1' align=left>\n <td align=left>%s</td>" % ("Number of BF+FE observations only"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b></td>" % (self.dbinfo[sub]["Nbffe"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d0' align=left>\n <td align=left>%s</td>" % ("Number of BF+IS+FE observations only"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b></td>" % (self.dbinfo[sub]["Nbfisfe"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d1' align=left>\n <td align=left>%s</td>" % ("Number of BF+IS+CS+FE observations only"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d</b></td>" % (self.dbinfo[sub]["Nbfiscsfe"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d0' align=left>\n <td align=left>%s [<font color=\"brown\"><b>%s</b></font>]</td>" % ("Number of FD observations", "only FD"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%d<b> [<font color=\"brown\"><b>%d</b></font>]</td>" % (self.dbinfo[sub]["Nfdtype"], self.dbinfo[sub]["Nfdtype_only"]))
		self.htmlptr.write ("\n</tr>")

		self.htmlptr.write ("\n<tr align=left height=25>\n <td align=left></td>")
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left></td>")
		self.htmlptr.write ("\n</tr>")

		self.htmlptr.write ("\n<tr class='d1' align=left>\n <td align=left>%s</td>" % ("Total size of raw data (TB)"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%.1f</b></td>" % (self.dbinfo[sub]["totRawsize"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d0' align=left>\n <td align=left>%s</td>" % ("Total size of raw data w/o IM-only (TB)"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%.1f</b></td>" % (self.dbinfo[sub]["totRawsize"]-self.dbinfo[sub]["IMonlyRawsize"]))
		self.htmlptr.write ("\n</tr>")
		self.htmlptr.write ("\n<tr class='d1' align=left>\n <td align=left>%s</td>" % ("Total size of processed data (TB)"))
		for sub in np.append("Total", self.subclusters):
			self.htmlptr.write ("\n <td align=left><b>%.1f</b></td>" % (self.dbinfo[sub]["totProcessedsize"]))
		self.htmlptr.write ("\n</tr>")

		self.htmlptr.write ("\n</table>")
                # getting date & time of last update
                if self.lupd == "":
                        cmd="date +'%b %d, %Y %H:%M:%S'"
                        self.lupd=os.popen(cmd).readlines()[0][:-1]
                self.htmlptr.write ("\n\n<hr width=100%%>\n<address>\nLast Updated: %s\n</address>\n" % (self.lupd, ))
		self.htmlptr.write ("\n</body>\n</html>")
		self.htmlptr.close()


# help
def usage (prg):
        """ Prints info how to use the script.
        """
        print "Program %s lists info about observations" % (prg, )
	print "Usage: %s [options]\n\
          -f, --from <date>          - list obs only _since_ <date> (inclusive), <date> in format YYYY-MM-DD\n\
          -t, --to <date>            - list obs only _till_ <date> (inclusive), <date> in format YYYY-MM-DD\n\
          --sort <mode>              - sort obs list. Default list is sorted by ObsID. Possible <mode>\n\
                                       is \"time\" to sort by start obs time, \"size\" to sort by total\n\
                                       disk space occupied by _raw_ data, \"source\" to sort by\n\
                                       the pointing coords of the source, and \"obsid\" is to sort by ObsID (default)\n\
                                       All sorting modes are: \"time\", \"size\", \"source\", \"obsid\"\n\
          --lse <lsenodes>           - set lse nodes to search for raw and processed data. Default are lse\n\
                                       nodes in sub5 and sub6, i.e. from lse013 to lse018, or <lsenodes> = 13-18\n\
                                       <lsenodes> should not have any spaces, nodes are specified just by number\n\
                                       without 'lse' prefix; comma and hyphen are allowed to list nodes and their ranges\n\
          --html <file>              - also write the list of obs in html-format to the file <file>\n\
          --linkedhtml <filestem>    - several lists of obs in html format will be created for for all sorted modes,\n\
                                       with basename <filestem>, and they will be linked together\n\
          -v, --view <mode>          - set the format of output data, namely the type of info to be listed.\n\
                                       Possible <mode> are \"usual\" (default) to list the data volume in every specified\n\
                                       lse nodes, total disk volume of raw data, datadir, if data reduced or not\n\
                                       in addition to start time, duration, source, etc. fiels that present in all formats.\n\
                                       Second <mode> is \"brief\" that lists antenna array, band filter, number of stations\n\
                                       used. Third <mode> is \"plots\" which is the same as \"brief\" mode in ascii output,\n\
                                       but in html-format it also provides the profiles (if existed) for RSP0 split and\n\
                                       in the full band (RSPA) together with chi-squared values of profiles, and with\n\
                                       the combined profiles from all RSPs together. Fourth <mode> is \"mega\" which is huge\n\
                                       table like for \"plots\" but also with all other info from \"usual\" mode together with\n\
                                       list of all stations. All view modes are: \"usual\" (default), \"brief\", \"plots\", \"mega\"\n\
          -r, --rebuild              - reprocess all observations from scratch (can take a while) rather than to read\n\
                                       the existent database, process obs that do not exist there, and add them to the database\n\
          -u, --update               - update db file only, new observations in /data? won't be added\n\
                                       This option can be used together with --from and --to to update only some observations\n\
          --stats                    - to calculate the statistics of existent observations in the database\n\
                                       can be used together with --from and --to options, and with --html option\n\
          --norsync                  - don't rsync plots and grid links to external webserver when \"mega\" or \"plots\" view mode is used\n\
          --dbfile <dbfile>          - database file with stored info about the observations\n\
          --debug                    - debug mode\n\
          -h, --help                 - print this message\n" % (prg, )




# Parse the command line
def parsecmd(prg, argv):
        """ Parsing the command line
        """
	try:
		opts, args = getopt.getopt (argv, "hf:t:v:ru", ["help", "sort=", "from=", "html=", "to=", "lse=", "view=", "linkedhtml=", "rebuild", "update", "debug", "stats", "dbfile=", "norsync"])
		for opt, arg in opts:
			if opt in ("-h", "--help"):
				usage(prg)
				sys.exit()
			if opt in ("--sort"):
				global sortkind
				sortkind = arg
				if sortkind != "time" and sortkind != "size" and sortkind != "source" and sortkind != "obsid":
					print "Arg for sort option should either be 'time' or 'size' or 'source' or 'obsid'\n"
					sys.exit()
			if opt in ("--html"):
				global is_html
				is_html = True
				global htmlfile
				htmlfile = arg
			if opt in ("--linkedhtml"):
				global is_linkedhtml
				is_linkedhtml = True
				global linkedhtmlstem
				linkedhtmlstem = arg
			if opt in ("--lse"):
				if arg.isspace() == True:
					print "--lse option has spaces that is not allowed"
					sys.exit()
				lsenodes = []
				for s in arg.split(","):
					if s.count("-") == 0:
						lsenodes = np.append(lsenodes, "lse%03d" % (int(s),))
					else:
						for l in np.arange(int(s.split("-")[0]), int(s.split("-")[1])+1):
							lsenodes = np.append(lsenodes, "lse%03d" % (l,))	
				global storage_nodes
				storage_nodes = lsenodes
			if opt in ("-f", "--from"):
				global is_from
				is_from = True
				global fromdate
				fromdate = arg
			if opt in ("-t", "--to"):
				global is_to
				is_to = True
				global todate
				todate = arg
			if opt in ("-v", "--view"):
				global viewtype
				viewtype = arg
			if opt in ("-r", "--rebuild"):
				global is_rebuild
				is_rebuild = True
			if opt in ("-u", "--update"):
				global is_update
				is_update = True
			if opt in ("--debug"):
				global is_debug
				is_debug = True
			if opt in ("--stats"):
				global is_stats
				is_stats = True
			if opt in ("--dbfile"):
				global dumpfile
				dumpfile = arg
			if opt in ("--norsync"):
				global is_torsync
				is_torsync = False

	except getopt.GetoptError:
		print "Wrong option!"
		usage(prg)
		sys.exit(2)

###################################################################################################################
#          M A I N                                                                                                #
###################################################################################################################
if __name__ == "__main__":

	# parsing command line
	parsecmd (sys.argv[0].split("/")[-1], sys.argv[1:])

	if is_rebuild and is_update:
		print "Choose what you want to do: rebuild, update or add new observations if any"
		sys.exit()

	# list of subclusters
	subclusters = np.unique([cexec_nodes[s].split(":")[0] for s in storage_nodes])

	# table with obs info
	# key is the ObsId, and value is outputInfo class instance
	obstable={}

	# creating plotsdir directory if it does not exist
	cmd="mkdir -p %s" % (plotsdir, )
	os.system(cmd)
	# creating griddir directory if it does not exist
	cmd="mkdir -p %s" % (griddir, )
	os.system(cmd)

	# reading the grid links from common file to the list
	if os.path.exists(archivefile):
		gridfiles, griddates, gridtimes, gridlinks = np.loadtxt(archivefile, usecols=(0,1,2,3), dtype=str, unpack=True, comments='#')

	# just make sure that rebuild and update switches are off
	if is_stats:
		is_rebuild = False
		is_update = False	

	if not is_rebuild:
		if not os.path.exists(dumpfile):
			print "Dumpfile \'%s\' does not exist! Use -r option to rebuild the database." % (dumpfile, )
			sys.exit()
		else:
			dfdescr = open(dumpfile, "r")
			obstable=cPickle.load(dfdescr)
			dfdescr.close()
			# update info and infohtml depending on current command line options and make the sorted list of Db obsids
			dbobsids = np.flipud(np.sort(obstable.keys(), kind='mergesort'))
			for r in dbobsids:
				obstable[r].update(storage_nodes)
			#
			# calculate statistics
			#
			if is_stats:
				obsids = dbobsids
				# we also have to choose only those IDs within the desired time range
				# if --from and/or --to are specified
				if is_from:
					fromsecs=time.mktime(time.strptime(fromdate, "%Y-%m-%d"))
					obsids=list(np.compress(np.array([obstable[r].seconds for r in obsids]) >= fromsecs, obsids))
				if is_to:
					tosecs=time.mktime(time.strptime(todate, "%Y-%m-%d")) + 86399
					obsids=list(np.compress(np.array([obstable[r].seconds for r in obsids]) <= tosecs, obsids))

				# initializing statistics' class
				stat = obsstat(obsids, fromdate, todate, storage_nodes)
				stat.printstat()
				if is_html:
					stat.printhtml(htmlfile)
				sys.exit(0)
				########## end of statistics #############

	if not is_update:
		# loop over the storage nodes and directories to get the list of all IDs
		for s in storage_nodes:
			for d in data_dirs:
				cmd="cexec %s 'find %s -maxdepth 1 -type d -name \"%s\" -print 2>/dev/null' 2>/dev/null | grep -v Permission | grep -v such | %s" % (cexec_nodes[s], d, "?20??_*", cexec_egrep_string)
				indlist=[i.split("/")[-1][:-1] for i in os.popen(cmd).readlines()]
				obsids = np.append(obsids, indlist)

		# also checking the archive directories to extend the list of ObsIDs in case the raw data was removed
		for s in storage_nodes:
			cmd="cexec %s 'ls -d %s 2>/dev/null' 2>/dev/null | grep -v such | %s" % (cexec_nodes[s], "/data4/LOFAR_PULSAR_ARCHIVE_" + s, cexec_egrep_string)
			if np.size(os.popen(cmd).readlines()) == 0:
				continue
			cmd="cexec %s 'find %s -type d -name \"%s\" -print 2>/dev/null' 2>/dev/null | grep -v Permission | grep -v such | %s" % (cexec_nodes[s], "/data4/LOFAR_PULSAR_ARCHIVE_" + s, "?20??_*_red", cexec_egrep_string)
			indlist=[i.split("/")[-1].split("_red")[0] for i in os.popen(cmd).readlines()]
			obsids = np.append(obsids, indlist)

		# getting the unique list of IDs (some of IDs can have entries in many /data? and nodes)
		# and sort in reverse order (most recent obs go first)
		# more recent obs is the obs with higher ID (as it should be)
		obsids = np.flipud(np.sort(np.unique(obsids), kind='mergesort'))

		# if is_rebuild == False then excluding ObsIDs from obsids list that are already in the database, i.e. in dbobsids list
		# only new ObsIDs will be processed and added to database
		if not is_rebuild:
			# now obsids have only those IDs that are not in the dump file
			obsids=list(set(obsids)-set(obsids).intersection(set(dbobsids)))
	else:   ## --update is set
		obsids = dbobsids
		# for the db update we also have to choose only those IDs within the desired time range
		# if --from and/or --to are specified
		if is_from:
			fromsecs=time.mktime(time.strptime(fromdate, "%Y-%m-%d"))
			obsids=list(np.compress(np.array([obstable[r].seconds for r in obsids]) >= fromsecs, obsids))

		if is_to:
			tosecs=time.mktime(time.strptime(todate, "%Y-%m-%d")) + 86399
			obsids=list(np.compress(np.array([obstable[r].seconds for r in obsids]) <= tosecs, obsids))


	if is_update:
		# also db can have obsids from all subclusters. However, we need not update all of them, only those
		# from the same subclusters as for lse in the command line option. This is because, the info from the parset
		# files is always the same, and do not need to be updated, and update of the processed data status can only
		# have sense for the same subclusters as in the command line option
		# also, we add 'subA' and 'sub?' as well
		newobsids=[]
		for sub in subclusters:
			newobsids=np.append(newobsids, list(np.compress(np.array([obstable[r].subcluster for r in obsids]) == sub, obsids)))
		newobsids=np.append(newobsids, list(np.compress(np.array([obstable[r].subcluster for r in obsids]) == "subA", obsids)))
		newobsids=np.append(newobsids, list(np.compress(np.array([obstable[r].subcluster for r in obsids]) == "sub?", obsids)))
		newobsids = np.flipud(np.sort(np.unique(newobsids), kind='mergesort'))
		obsids = newobsids

	# Number of ObsIDs
	Nobsids = np.size(obsids)

	# loop for every observation
	for id in obsids:
	
		# class instance with output Info
		out=outputInfo(id)	

		# prefix of ID, like L2010 or L2009
        	id_prefix=id.split("_")[0]   
		# suffix of ID, the sequence number of observation
        	id_suffix=id.split("_")[1]   

		#
		# checking first if the directory with the parset file exists
		#
		# There were several changes in the location of the parset files and its name, so we have to
		# check all of them. There were:
		# (0) OLDEST parset file: /globalhome/lofarsystem/oldlog/id/RTCP.parset.0   <-- parset_oldlogdir + id + parset  
		# (1) OLD parset was here: /globalhome/lofarsystem/log/id/RTCP.parset.0     <-- parset_logdir + id + parset
		# (2) NEW parset as of May 10, 2010 is here: /globalhome/lofarsystem/log/L2010-MM-DD-DATE/RTCP-ID.parset
		# (3) 2nd transpose parset as of Aug 20, 2010 is here: 
		#          /globalhome/lofarsystem/production/lofar-trunk/bgfen/log/L2010-MM-DD-DATE/RTCP-ID.parset
		#
		logdir=parset_logdir + id + "/"
		if not os.path.exists(logdir):
			# checking in the oldlog directory
			logdir=parset_oldlogdir + id + "/"
			if not os.path.exists(logdir):
				# Due to new naming convention and location of the parset files, also looking for the parset file
				# in any L2010-??-??_?????? directories	
				cmd="find %s -type f -name '*%s.parset' -print 2>/dev/null | grep -v Permission | grep -v such" % (parset_logdir, id_suffix)
				logdir=os.popen(cmd).readlines()
				if np.size(logdir) > 0:
					# it means we found the directory with parset file
					logdir=os.popen("dirname %s" % (logdir[0][:-1],)).readlines()[0][:-1]
					logdir = logdir + "/"
				else:
					# now checking the new parset directory
					cmd="find %s -type f -name '*%s.parset' -print 2>/dev/null | grep -v Permission | grep -v such" % (parset_newlogdir, id_suffix)
					logdir=os.popen(cmd).readlines()
					if np.size(logdir) > 0:
						# it means we found the directory with parset file
						logdir=os.popen("dirname %s" % (logdir[0][:-1],)).readlines()[0][:-1]
						logdir = logdir + "/"
					else:
						# no directory found
						comment = "Oops!.. The log directory or parset file in new naming convention does not exist!"
						out.setcomment(id, storage_nodes, comment)
						obstable[id] = out
						continue

		# get the full path for the parset file for the current ID
		log=logdir + parset
		if not os.path.exists(log):
			# also checking that maybe the parset file has name the same as Obs ID
			log=logdir + id + ".parset"
			if not os.path.exists(log):
				# also checking that maybe the name of parset file has new naming convention, like "RTCP-<id_suffix>.parset"
				# OR like L<id_suffix>.parset
				log=logdir + "RTCP-" + id_suffix + ".parset"
				if not os.path.exists(log):
					log=logdir + "L" + id_suffix + ".parset"
					if not os.path.exists(log):
						comment = "Oops!.. The parset file '%s' does not exist in any possible location!" % (parset,)
						out.setcomment(id, storage_nodes, comment)
						obstable[id] = out
						continue

		# initializing the obsinfo class
		oi=obsinfo(log)

		# checking if the datadir exists in all lse nodes and if it does, gets the size of directory
		dirsizes = {}
		for lse in storage_nodes:
			ddir=oi.datadir + "/" + id
			dirsizes[lse] = ["x", "0.0"]
			cmd="cexec %s 'du -sh %s 2>/dev/null | cut -f 1' 2>/dev/null | grep -v such | %s" % (cexec_nodes[lse], ddir, cexec_egrep_string)
			dirout=os.popen(cmd).readlines()
			if np.size(dirout) > 0:
				dirsizes[lse][0]=dirout[0][:-1]
				cmd="cexec %s 'du -s -B 1 %s 2>/dev/null | cut -f 1' 2>/dev/null | grep -v such | %s" % (cexec_nodes[lse], ddir, cexec_egrep_string)
				status=os.popen(cmd).readlines()
				if np.size(status) > 0:
					status=status[0][:-1]
					if status.isdigit() == True:
							dirsizes[lse][1] = status

		# checking if this specific observation was already reduced. Checking for both existence of the *_red directory
		# in LOFAR_PULSAR_ARCHIVE and the existence of *_plots.tar.gz file
		statusline="x"
		redlocation="x"
		processed_dirsize=0.0
		reduced_node = ""
		for lse in storage_nodes:
			cmd="cexec %s 'ls -d %s 2>/dev/null' 2>/dev/null | grep -v such | %s" % (cexec_nodes[lse], "/data4/LOFAR_PULSAR_ARCHIVE_" + lse, cexec_egrep_string)
			if np.size(os.popen(cmd).readlines()) == 0:
				continue

			reduced_node = lse # saving the lse node with the reduced data for further use (to increase the performance)
			reddir = ""

			cmd="cexec %s 'find %s -type d -name \"%s\" -print 2>/dev/null' 2>/dev/null | grep -v Permission | grep -v such | %s" % (cexec_nodes[lse], "/data4/LOFAR_PULSAR_ARCHIVE_" + lse, id + "_red", cexec_egrep_string)
			redout=os.popen(cmd).readlines()
			if np.size(redout) > 0:
				reddir=redout[0][:-1]
				statusline=lse
				redlocation="%s/%s/%s%s" % ("/net", cexec_nodes[lse].split(":")[0], lse, reddir)
				# getting the size of the processed data
				cmd="cexec %s 'du -s -B 1 %s 2>/dev/null | cut -f 1' 2>/dev/null | grep -v such | %s" % (cexec_nodes[lse], reddir, cexec_egrep_string)
				status=os.popen(cmd).readlines()
				if np.size(status) > 0:
					status=status[0][:-1]
					if status.isdigit() == True:
							processed_dirsize = float(status) / 1024. / 1024. / 1024.
				# checking if final tar.gz file exists
				cmd="cexec %s 'find %s -name \"%s\" -print 2>/dev/null' 2>/dev/null | grep -v Permission | grep -v such | %s" % (cexec_nodes[lse], reddir, "*_plots.tar.gz", cexec_egrep_string)
				if np.size(os.popen(cmd).readlines()) > 0:
					# tarfile exists
					statusline=statusline+" +tar"	
				else:
					statusline=statusline+" -tar"
				# checking if RSPA exists
				cmd="cexec %s 'ls -d %s 2>/dev/null' 2>/dev/null | %s" % (cexec_nodes[lse], reddir + "/incoherentstokes/RSPA", cexec_egrep_string)
                                if np.size(os.popen(cmd).readlines()) > 0:
					statusline=statusline+" +all"
				else:
					statusline=statusline+" -all"
				# checking if rfirep file exists in RSP0
				cmd="cexec %s 'find %s -name \"%s\" -print 2>/dev/null' 2>/dev/null | grep -v Permission | grep -v such | %s" % (cexec_nodes[lse], reddir + "/incoherentstokes/RSP0", "*.rfirep", cexec_egrep_string)
                                if np.size(os.popen(cmd).readlines()) > 0:
					statusline=statusline+" +rfi"
				else:
					statusline=statusline+" -rfi"
				# checking if summary rfirep file exists
				cmd="cexec %s 'find %s -name \"%s\" -print 2>/dev/null' 2>/dev/null | grep -v Permission | grep -v such | %s" % (cexec_nodes[lse], reddir + "/incoherentstokes", "*.rfirep", cexec_egrep_string)
                                if np.size(os.popen(cmd).readlines()) > 0:
					statusline=statusline+" +rfiA"
				else:
					statusline=statusline+" -rfiA"
				# checking if "search" directory exists
				cmd="cexec %s 'ls -1 -d %s 2>/dev/null' 2>/dev/null | %s" % (cexec_nodes[lse], reddir + "/search*", cexec_egrep_string)
                                if np.size(os.popen(cmd).readlines()) > 0:
					statusline=statusline+" +search"
				break


		# Collecting info about chi-squared and profile png-files
		profiles_array=["", ""]
		chi_array=["", ""]
		combined_plot=""  # name of the combined plot (always the same), or empty string if it does not exist

		if reduced_node != "":
			if reddir != "":
				# getting first the name of the main pulsar (first in the command-line option)
				cmd="cexec %s 'find %s -name \"%s\" -print 2>/dev/null' 2>/dev/null | grep -v Permission | grep -v such | %s" % (cexec_nodes[reduced_node], reddir, "*.log", cexec_egrep_string)
				mainlog=os.popen(cmd).readlines()
				mainpsr="undefined"
				if np.size(mainlog) > 0:
					cmd="cexec %s 'cat %s 2>/dev/null | grep id | head -n 1' 2>/dev/null | grep -v Permission | grep -v such | %s" % (cexec_nodes[reduced_node], mainlog[0][:-1], cexec_egrep_string)
					cmdline=os.popen(cmd).readlines()
					if np.size(cmdline) > 0:
						param=cmdline[0][:-1].split(" ")
						for k in np.arange(len(param)):
        						if param[k] == "-p":
                						mainpsr=param[k+1].split(",")[0]
                						break
				# it's possible to specify the pulsar name by "position". In this case profiles will be folded for 3 pulsars closest
				# to this position
				if mainpsr == "position":
					mainpsr = "undefined"
				# RSP0
				cmd="cexec %s 'ls -d %s 2>/dev/null' 2>/dev/null | %s" % (cexec_nodes[reduced_node], reddir + "/incoherentstokes/RSP0", cexec_egrep_string)
				if np.size(os.popen(cmd).readlines()) > 0:
					if mainpsr == "undefined":
						cmd="cexec %s 'find %s -name \"%s\" -print 2>/dev/null' 2>/dev/null | grep -v Permission | grep -v such | %s" % (cexec_nodes[reduced_node], reddir + "/incoherentstokes/RSP0", "*.pfd*png", cexec_egrep_string)
					else:
						cmd="cexec %s 'find %s -name \"%s\" -print 2>/dev/null' 2>/dev/null | grep -v Permission | grep -v such | %s" % (cexec_nodes[reduced_node], reddir + "/incoherentstokes/RSP0", mainpsr + "*.pfd*png", cexec_egrep_string)
					status=os.popen(cmd).readlines()
					if np.size(status) > 0:
						# copying png files to local directory
						cmd="mkdir -p %s/%s ; cexec %s 'cp -f %s %s/%s' 2>&1 1>/dev/null" % (plotsdir, id, cexec_nodes[reduced_node], " ".join([ss[:-1] for ss in status]), plotsdir, id)
						os.system(cmd)
						profiles_array[0] = status[0][:-1].split("/")[-1].split(".pfd")[0] + ".pfd"
					# getting chi-squared
					if mainpsr == "undefined":
						cmd="cexec %s 'find %s -name \"%s\" -print -exec cat {} \; 2>/dev/null | grep chi-squared' 2>/dev/null | grep -v Permission | grep -v such | %s | awk '{print $6}' -" % (cexec_nodes[reduced_node], reddir + "/incoherentstokes/RSP0", "*.prepout", cexec_egrep_string)
					else:
						cmd="cexec %s 'find %s -name \"%s\" -print -exec cat {} \; 2>/dev/null | grep chi-squared' 2>/dev/null | grep -v Permission | grep -v such | %s | awk '{print $6}' -" % (cexec_nodes[reduced_node], reddir + "/incoherentstokes/RSP0", mainpsr + "*.prepout", cexec_egrep_string)
					status=os.popen(cmd).readlines()
					if np.size(status) > 0:
						chi_array[0] = status[0][:-1]
				# RSPA
				cmd="cexec %s 'ls -d %s 2>/dev/null' 2>/dev/null | grep -v such | %s" % (cexec_nodes[reduced_node], reddir + "/incoherentstokes/RSPA", cexec_egrep_string)
				if np.size(os.popen(cmd).readlines()) > 0:
					if mainpsr == "undefined":
						cmd="cexec %s 'find %s -name \"%s\" -print 2>/dev/null' 2>/dev/null | grep -v Permission | grep -v such | %s" % (cexec_nodes[reduced_node], reddir + "/incoherentstokes/RSPA", "*.pfd*png", cexec_egrep_string)
					else:
						cmd="cexec %s 'find %s -name \"%s\" -print 2>/dev/null' 2>/dev/null | grep -v Permission | grep -v such | %s" % (cexec_nodes[reduced_node], reddir + "/incoherentstokes/RSPA", mainpsr + "*.pfd*png", cexec_egrep_string)
					status=os.popen(cmd).readlines()
					if np.size(status) > 0:
						# copying png files to local directory
						cmd="mkdir -p %s/%s ; cexec %s 'cp -f %s %s/%s' 2>&1 1>/dev/null" % (plotsdir, id, cexec_nodes[reduced_node], " ".join([ss[:-1] for ss in status]), plotsdir, id)
						os.system(cmd)
						profiles_array[1] = status[0][:-1].split("/")[-1].split(".pfd")[0] + ".pfd"
					# getting chi-squared
					if mainpsr == "undefined":
						cmd="cexec %s 'find %s -name \"%s\" -print -exec cat {} \; 2>/dev/null | grep chi-squared' 2>/dev/null | grep -v Permission | grep -v such | %s | awk '{print $6}' -" % (cexec_nodes[reduced_node], reddir + "/incoherentstokes/RSPA", "*.prepout", cexec_egrep_string)
					else:
						cmd="cexec %s 'find %s -name \"%s\" -print -exec cat {} \; 2>/dev/null | grep chi-squared' 2>/dev/null | grep -v Permission | grep -v such | %s | awk '{print $6}' -" % (cexec_nodes[reduced_node], reddir + "/incoherentstokes/RSPA", mainpsr + "*.prepout", cexec_egrep_string)
					status=os.popen(cmd).readlines()
					if np.size(status) > 0:
						chi_array[1] = status[0][:-1]

				# checking if combined plot exists and rsync it if it does exist
				cmd="cexec %s 'ls -1 %s/%s 2>/dev/null' 2>/dev/null | grep -v such | %s" % (cexec_nodes[reduced_node], reddir, "combined.th.png", cexec_egrep_string)
				if np.size(os.popen(cmd).readlines()) != 0:
					combined_plot="combined"
					# copying combined plot
					cmd="mkdir -p %s/%s ; cexec %s 'cp -f %s/%s %s/%s %s/%s' 2>&1 1>/dev/null" % (plotsdir, id, cexec_nodes[reduced_node], reddir, combined_plot + ".png", reddir, combined_plot + ".th.png", plotsdir, id)
					os.system(cmd)

		# getting the grid links for the current ObsID and putting them to separate ascii file
		# and making the archive status line for the table
		archivestatus = "x"
		if len(gridfiles) != 0:
			grid_obsid_indices = [i for i in np.arange(len(gridfiles)) if re.search(id, gridfiles[i])]
			if len(grid_obsid_indices) != 0:
				archivestatus = ""
				grid_obsid_files = [gridfiles[i] for i in grid_obsid_indices]
				grid_obsid_dates = [griddates[i] for i in grid_obsid_indices]
				grid_obsid_times = [gridtimes[i] for i in grid_obsid_indices]
				grid_obsid_links = [gridlinks[i] for i in grid_obsid_indices]
				# saving the grid links to the separate ascii file
				np.savetxt(griddir + "/%s.txt" % (id), np.transpose((grid_obsid_files, grid_obsid_dates, grid_obsid_times, grid_obsid_links)), fmt="%s", delimiter="   ")
				rawfiles = [file for file in grid_obsid_files if re.search("raw", file)]
				if len(rawfiles) != 0: archivestatus = archivestatus+" raw"
				subbandfiles = [file for file in grid_obsid_files if re.search("sub", file)]
				if len(subbandfiles) != 0: archivestatus = archivestatus+" sub"
				metafiles = [file for file in grid_obsid_files if re.search("meta", file)]
				if len(metafiles) != 0: archivestatus = archivestatus+" meta"
				archivestatus = " ".join(archivestatus.split(" ")[1:])

		# combining info
		out.Init(id, oi, storage_nodes, dirsizes, statusline, reduced_node, redlocation, processed_dirsize, "", profiles_array, chi_array, combined_plot, archivestatus)
		obstable[id] = out
		# printing the info line by line in debug mode
		if is_debug:
			print "%d       %s" % (debugcounter, out.info)
			cmd="echo \"%d (%d)\" > %s" % (debugcounter, Nobsids, debugfile)
			os.system(cmd)
			debugcounter += 1

	# dump obs table to the file
	dfdescr = open (dumpfile, "w")
	cPickle.dump(obstable, dfdescr, True)
	dfdescr.close()

	# uploading the png files to webserver
	if is_torsync:
		if viewtype == 'plots' or viewtype == 'mega':
			cmd="ssh %s mkdir -p %s" % (webserver, webplotsdir)	
			os.system(cmd)
			cmd="rsync -a %s/ %s:%s 2>&1 1>/dev/null" % (plotsdir, webserver, webplotsdir)
			os.system(cmd)
			cmd="ssh %s mkdir -p %s" % (webserver, webgriddir)	
			os.system(cmd)
			cmd="rsync -a %s/ %s:%s 2>&1 1>/dev/null" % (griddir, webserver, webgriddir)
			os.system(cmd)

	# copying to another list to keep the old one
	obskeys = np.flipud(np.sort(obstable.keys(), kind='mergesort'))

	# if is_from and/or is_to are set, then we have to exclude those records
	# from obstable that do not obey the conditions
	if is_from:
		fromsecs=time.mktime(time.strptime(fromdate, "%Y-%m-%d"))
		obskeys=list(np.compress(np.array([obstable[r].seconds for r in obskeys]) >= fromsecs, obskeys))

	if is_to:
		tosecs=time.mktime(time.strptime(todate, "%Y-%m-%d")) + 86399
		obskeys=list(np.compress(np.array([obstable[r].seconds for r in obskeys]) <= tosecs, obskeys))

	# similar to what we do when we are updating db records, here we also only want to show the observations from
	# selected (from the command line) subclusters, because db can have data for all subclusters
	newobskeys=[]
	for sub in subclusters:
		newobskeys=np.append(newobskeys, list(np.compress(np.array([obstable[r].subcluster for r in obskeys]) == sub, obskeys)))
	newobskeys=np.append(newobskeys, list(np.compress(np.array([obstable[r].subcluster for r in obskeys]) == "subA", obskeys)))
	newobskeys=np.append(newobskeys, list(np.compress(np.array([obstable[r].subcluster for r in obskeys]) == "sub?", obskeys)))
	newobskeys = np.flipud(np.sort(np.unique(newobskeys), kind='mergesort'))
	obskeys = newobskeys
	
	#
	# Printing the table
	#
	print "Number of observations in %s: %d" % (", ".join(subclusters), np.size(obskeys))
	if not is_rebuild and not is_update:
		print "Number of new observations found in %s: %d" % (", ".join(storage_nodes), np.size(obsids))

	if is_from == True or is_to == True:
		print "List only observations%s%s" % (is_from and " since " + fromdate or (is_to and " till " + todate or ""), 
                                                      is_to and (is_from and " till " + todate or "") or "")
	print

	# number of storage nodes
	Nnodes=np.size(storage_nodes)

	# printing out the header of the table
	storage_nodes_string=""
	for i in np.arange(Nnodes-1):
		storage_nodes_string=storage_nodes_string+storage_nodes[i]+"\t"
	storage_nodes_string=storage_nodes_string+storage_nodes[-1]
	storage_nodes_string_html="</th>\n <th align=center>".join(storage_nodes_string.split("\t"))

	equalstrs=[]
	if viewtype == "brief":
		equalstring_size=163
		for e in np.arange(equalstring_size):
			equalstrs = np.append(equalstrs, "=")
		equalstring="#" + "".join(equalstrs)
		
		print equalstring
		print "# No.	ObsID		Source		MMDD	Dur	Ant	Band	   #Stations	    BF FD IM IS CS FE	Location		Status"
		print equalstring
	elif viewtype == "plots":
		equalstring_size=172
		for e in np.arange(equalstring_size):
			equalstrs = np.append(equalstrs, "=")
		equalstring="#" + "".join(equalstrs)
		
		print equalstring
		print "# No.	ObsID		Source		MMDD	Dur	Ant	Band	   #Stations	    BF FD IM IS CS FE	Location		Status      Archive"
		print equalstring
	elif viewtype == "mega":
		equalstring_size=245+8*Nnodes
		for e in np.arange(equalstring_size):
			equalstrs = np.append(equalstrs, "=")
		equalstring="#" + "".join(equalstrs)
		
		print equalstring
		print "# No.	ObsID		Source		MMDD	Dur	Ant	Band	   #Stations	    BF FD IM IS CS FE	NodesList (lse) Datadir	%s	Total(GB)	Stations		Location                Status      Archive" % (storage_nodes_string,)
		print equalstring
	else: # usual
		equalstring_size=159+8*Nnodes
		for e in np.arange(equalstring_size):
			equalstrs = np.append(equalstrs, "=")
		equalstring="#" + "".join(equalstrs)
		
		print equalstring
		print "# No.	ObsID		MMDD	Dur	NodesList (lse)	Datadir	%s	Total(GB)	BF FD IM IS CS FE	Status				Pointing    Source" % (storage_nodes_string,)
		print equalstring
		

	# printing the sorted list
	if sortkind == "size":
		sorted_indices=np.flipud(np.argsort([float(obstable[r].totsize) for r in obskeys], kind='mergesort'))
	elif sortkind == "time":
		sorted_indices=np.flipud(np.argsort([obstable[r].seconds for r in obskeys], kind='mergesort'))
	# sorting by source (pointing coords)
	elif sortkind == "source":
		sorted_indices=np.argsort([obstable[r].pointing for r in obskeys], kind='mergesort')
	# unsorted (i.e. by default sorted by ObsId)
	else:
		sorted_indices=np.arange(np.size(obskeys))

	counter=0
	for i in sorted_indices:
		print "%d	%s" % (counter, obstable[obskeys[i]].info)
		counter += 1


	# writing the html code if chosen
	if is_html:
		htmlrep=writeHtmlList(htmlfile, linkedhtmlstem, fromdate, todate)
		htmlrep.open()
		htmlrep.obsnumber(storage_nodes, subclusters, np.size(obskeys), np.size(obsids))
		htmlrep.datesrange()
		htmlrep.header(viewtype, storage_nodes_string_html)
		counter = 0
		for i in sorted_indices:
			htmlrep.record(counter%2 == 0 and "d0" or "d1", counter, obstable[obskeys[i]].infohtml)
			counter += 1
		htmlrep.legend()
		htmlrep.close()

	# create html files sorted for different sorting modes, linked together
	if is_linkedhtml:
		sf={"obsid": "-obsid.html", "time": "-time.html", "size": "-size.html", "source": "-source.html"}
		for key in sf.keys():
			sf[key] = "%s%s" % (linkedhtmlstem, sf[key])
		# create html-file with statistics
		htmlstatfile = "%s-stat.html" % (linkedhtmlstem) 
		stat = obsstat(obskeys, fromdate, todate, storage_nodes)
		stat.printhtml(htmlstatfile)

		htmlrep=writeHtmlList(sf["obsid"], linkedhtmlstem, fromdate, todate)
		for key in sf.keys():
			htmlrep.reInit(sf[key])
			htmlrep.open()
			htmlrep.obsnumber(storage_nodes, subclusters, np.size(obskeys), np.size(obsids))
			htmlrep.datesrange()
			htmlrep.statistics(htmlstatfile)
			htmlrep.linkedheader(viewtype, storage_nodes_string_html)
			if key == "size":
				sorted_indices=np.flipud(np.argsort([float(obstable[r].totsize) for r in obskeys], kind='mergesort'))
			elif key == "time":
				sorted_indices=np.flipud(np.argsort([obstable[r].seconds for r in obskeys], kind='mergesort'))
			elif key == "source":
				sorted_indices=np.argsort([obstable[r].pointing for r in obskeys], kind='mergesort')
			# unsorted (i.e. by default sorted by ObsId)
			else:
				sorted_indices=np.arange(np.size(obskeys))
			counter = 0
			for i in sorted_indices:
				htmlrep.record(counter%2 == 0 and "d0" or "d1", counter, obstable[obskeys[i]].infohtml)
				counter += 1
			htmlrep.legend()
			htmlrep.close()

	if is_debug:
		cmd="rm -f %s" % (debugfile,) 
		os.system(cmd)
