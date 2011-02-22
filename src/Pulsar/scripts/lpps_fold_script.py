#!/usr/bin/env python
'''
Script that folds pulsar candidates found by the PRESTO acceleration search.
(Works on old style lpps search output directories, where all the different
file types are mixed in 1 directory).

Note : still needs a lot better sifting algorithm, the prepfold commands are
generated like the prepfold command that the GBT drift scan produces.
'''

# Standard library imports:
import shutil
import os
import sys
import optparse
import glob
import stat
import subprocess
# PRESTO imports
import sifting
# LOFAR PILOT PULSAR SURVEY imports
from lpps_search import crawler
from lpps_search import inf
from lpps_search.fold import main
from lpps_search.sift import sift_accel_cands
from lpps_search.util import create_script, run_as_script
from lpps_search.util import get_command, run_command
from lpps_search.util import MissingOptionException
from lpps_search.util import MissingCommandLineOptionException
from lpps_search.util import DirectoryNotEmpty, WrongPermissions

REQUIRED_OPTIONS = ['searchoutdir', 'subbdir', 'workdir', 'basename']
OPTIONAL_OPTIONS = ['ncores']


if __name__ == '__main__':
    # Set up and run the commandline parser:
    # Note that here the commandline option and the name of it internally are
    # same (but without the --).
    parser = optparse.OptionParser()
    parser.add_option('--subbdir', help='Input directory for sub band files.',
        type='string', metavar='SUBB_DIR', dest='subbdir')
    parser.add_option('--searchoutdir', help='Search ouput directory.',
        type='string', dest='searchoutdir', metavar='SEARCH_OUT_DIR')
    parser.add_option('--workdir', help='Root dir for working directory.',
        type='string', dest='workdir', metavar='WORK_DIR')
    parser.add_option('--ncores', metavar='N_CORES', default=8, type='int', 
        help='Number of processor cores to use default is 8.', dest='ncores')
    parser.add_option('--basename', type='string', metavar='BASENAME', 
        help='Basename of PRESTO files to look for.', dest='basename')
    options, args = parser.parse_args()

    # Check that all the required options were supplied on the commandline:
    kwargs = {}
    for k in REQUIRED_OPTIONS:
        v = getattr(options, k)
        if v == None:
            print 'You need to supply the --%s option' % k
            raise MissingCommandLineOptionException(k)

    search_out_dir = os.path.abspath(options.searchoutdir)

    # prepare working directory (for both candidates and folds in the end)
    work_dir = os.path.abspath(options.workdir)
    if not os.access(work_dir, os.F_OK | os.R_OK | os.W_OK):
        raise WrongPermissions(work_dir)

    files = os.listdir(work_dir)
    if len(files) > 0:  
        raise DirectoryNotEmpty(work_dir)

    # make a directory to receive the pulsar folds in the working area
    folddir = os.path.join(work_dir, 'FOLDS')
    os.mkdir(folddir)

    # make a directory for the candidates and .inf files in the working area
    canddir = os.path.join(work_dir, 'CANDIDATES')
    os.mkdir(canddir)

    # copy the candidates and the .inf files to the working area
    search_accelsearch_dir = os.path.join(search_out_dir, 'ACCELSEARCH')
    search_inf_dir = os.path.join(search_out_dir, 'INF')
    for f in os.listdir(search_accelsearch_dir):
        shutil.copy(os.path.join(search_accelsearch_dir, f), canddir)
    for f in os.listdir(search_inf_dir): 
        shutil.copy(os.path.join(search_inf_dir, f), canddir)
    
    # prepare the keyword arguments dictionary to give to the folding module
    kwargs = {
        'canddir' : canddir,
        'folddir' : folddir,
        'subbdir' : os.path.abspath(options.subbdir),
        'ncores' : options.ncores,
        'basename' : options.basename,
    }
    main(**kwargs)

