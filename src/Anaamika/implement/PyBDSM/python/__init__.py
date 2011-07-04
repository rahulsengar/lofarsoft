"""Initialize PyBDSM namespace.

Import all standard operations, define default chain of
operations and provide function 'execute', which can
execute chain of operations properly. Also define the
'process_image' convienence function which can take
options as arguments rather than as a dictionary as
required by 'execute'.
"""
from readimage import Op_readimage
from collapse import Op_collapse
from preprocess import Op_preprocess
from rmsimage import Op_rmsimage
from threshold import Op_threshold
from islands import Op_islands
from gausfit import Op_gausfit
from make_residimage import Op_make_residimage
from output import Op_outlist
from shapefit import Op_shapelets
from gaul2srl import Op_gaul2srl
from spectralindex import Op_spectralindex
from polarisation import Op_polarisation
from wavelet_atrous import Op_wavelet_atrous
from psf_vary import Op_psf_vary
from cleanup import Op_cleanup
import graphix
import mylogger
from interface import in_ipython
__version__ = '1.0' # placeholder version number
__revision__ = filter(str.isdigit, "$Revision$")

default_chain = [Op_readimage(),
                 Op_collapse(),
                 Op_preprocess(),
                 Op_rmsimage(),
                 Op_threshold(), 
                 Op_islands(),
                 Op_gausfit(), 
                 Op_shapelets(),
                 Op_make_residimage(), 
                 Op_gaul2srl(), 
                 Op_spectralindex(),
                 Op_polarisation(),
                 Op_wavelet_atrous(),
                 Op_psf_vary(),
                 Op_outlist(),
                 Op_cleanup()
                 ]
fits_chain = default_chain # for legacy scripts

def execute(chain, opts):
    """Execute chain.

    Create new Image with given options and apply chain of 
    operations to it. The opts input must be a dictionary.
    """
    from image import Image
    import mylogger 

    if opts.has_key('quiet'):
        quiet = opts['quiet']
    else:
        quiet = False
    if opts.has_key('debug'):
        debug = opts['debug']
    else:
        debug = False
    log_filename = opts["filename"] + '.pybdsm.log'
    mylogger.init_logger(log_filename, quiet=quiet, debug=debug)
    mylog = mylogger.logging.getLogger("PyBDSM.Init")
    mylog.info("Running PyBDSM on "+opts["filename"])

    try:
        img = Image(opts)
        img.log = log_filename
        _run_op_list(img, chain)
        return img
    except RuntimeError, err:
        # Catch and log, then re-raise if needed (e.g., for AstroWise)
        mylog.error(str(err))
        # raise
    except KeyboardInterrupt:
        mylogger.userinfo(mylog, "\n\033[31;1mAborted\033[0m")
        # raise


def _run_op_list(img, chain):
    """Runs an Image object through chain of op's.

    This is separate from execute() to allow other modules (such as
    interface.py) to use it as well.
    """
    from time import time
    from types import ClassType, TypeType
    from interface import raw_input_no_history
    from gausfit import Op_gausfit
    
    ops = []
    stopat = img.opts.stop_at
    # Make sure all op's are instances
    for op in chain:
        if isinstance(op, (ClassType, TypeType)):
            ops.append(op())
        else:
            ops.append(op)
        if stopat == 'read' and isinstance(op, Op_readimage): break
        if stopat == 'isl' and isinstance(op, Op_islands): break

    # Run all op's
    for op in ops:
        if isinstance(op, Op_gausfit) and img.opts.interactive:
            dc = '\033[34;1m'
            nc = '\033[0m'
            print dc + 'Displaying islands and rms image...' + nc
            img.show_fit(rms_image=True, mean_image=True,
                ch0_islands=True, gresid_image=False, sresid_image=False,
                gmodel_image=False, smodel_image=False, pyramid_srcs=False)
            prompt = dc + "Press enter to continue or 'q' to quit    : " + nc
            answ = raw_input_no_history(prompt)
            while answ != '':
                if answ == 'q':
                    return
                answ = raw_input_no_history(prompt)
        op.__start_time = time()
        op(img)
        op.__stop_time = time()

    if img.opts.interactive:
        print dc + 'Fitting complete. Displaying results...' + nc
        img.show_fit()

    if img.opts.print_timing:
        print "="*30
        for op in chain:
            print "%15s : %f" % (op.__class__.__name__, 
                                 (op.__stop_time - op.__start_time))
        print "="*30
        print "%15s : %f" % ("Total",
                             (chain[-1].__stop_time - chain[0].__start_time))


def process_image(input_file, **kwargs):
    """Run a standard analysis and returns the associated Image object.

    The input file can be a FITS or CASA image or a PyBDSM parameter save
    file. Partial names are allowed for the parameters as long as they are
    unique. Parameters are set to default values if par = ''.

    Examples:
        > img = bdsm.process_image('example.fits', thresh_isl=4)
          --> process FITS image names 'example.fits'
        > img_3C196 = bdsm.process_image('3C196.image', mea='map')
          --> process CASA image, 'mean_map' parameter is abbreviated
        > img_VirA = bdsm.process_image('VirA_im.pybdsm.sav')
          --> load parameter save file and process
    """
    from interface import load_pars
    from image import Image
    
    # Try to load input_file assuming it's a parameter save file.
    # load_pars returns None if this doesn't work.
    img = load_pars(input_file)

    # If load_pars fails, assume that input_file is an image
    if img == None:
        img = Image({'filename': input_file})

    # Now process it. Any kwargs specified by the user will
    # override those read in from the parameter save file.
    img.process(**kwargs)
    return img
    