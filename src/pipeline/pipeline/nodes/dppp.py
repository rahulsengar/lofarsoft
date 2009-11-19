# Python standard library
from __future__ import with_statement
from contextlib import closing
from subprocess import check_call, CalledProcessError
import os.path, tempfile, shutil

from pipeline.support.lofarnode import LOFARnode
from pipeline.support.utilities import patch_parset, create_directory, log_time
from pipeline.support.lofarexceptions import ExecutableMissing
import pipeline.support.utilities as utilities

log_prop = """
log4cplus.rootLogger=DEBUG, FILE
log4cplus.logger.TRC=DEBUG, FILE
log4cplus.additivity.TRC=FALSE

log4cplus.appender.FILE=log4cplus::FileAppender
log4cplus.appender.FILE.File=%(filename)s
log4cplus.appender.FILE.layout=log4cplus::PatternLayout
"""
log_format = "log4cplus.appender.FILE.layout.ConversionPattern=%x %D{%d-%m-%y %H:%M:%S} %-5p %c{3} - %m [%.25l]%n"


class dppp_node(LOFARnode):
    def run(self, infile, outfile, parset, log_location, executable, initscript):
        with log_time(self.logger):
            self.logger.info("Processing %s" % (infile,))

            # We need to patch the parset with the correct input/output MS names.
            temp_parset_filename = patch_parset(
                parset, {
                    'msin': infile,
                    'msout': outfile,
                }
            )

            create_directory(log_location)
            create_directory(os.path.dirname(outfile))

            env = utilities.read_initscript(initscript)

            try:
                if not os.access(executable, os.X_OK):
                    raise ExecutableMissing(executable)
                working_dir = tempfile.mkdtemp()
                log_prop_filename = os.path.join(working_dir, os.path.basename(executable) + ".log_prop")
                with open(log_prop_filename, 'w') as log_prop_file:
                    log_prop_file.write(log_prop % {'filename': os.path.join(
                            log_location,
                            os.path.basename(executable) + '.' + os.path.basename(infile) + '.log'
                        )}
                    )
                    log_prop_file.write(log_format)
                # What is the '1' for? Required by DP3...
                cmd = [executable, temp_parset_filename, '1']
                self.logger.debug("Running: %s" % (' '.join(cmd),))
                result = check_call(
                    cmd,
                    cwd=working_dir,
                    env=env
                )
                self.logger.debug(
                    "%s returned exit status %d" % (executable, result)
                )
            except ExecutableMissing, e:
                self.logger.error("%s not found" % (e.args[0]))
                raise
            except CalledProcessError, e:
                # For CalledProcessError isn't properly propagated by IPython
                # Temporary workaround...
                self.logger.error(str(e))
                raise Exception
            finally:
                os.unlink(temp_parset_filename)
                shutil.rmtree(working_dir)

            return result
