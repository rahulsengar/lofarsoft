#                                                       LOFAR PIPELINE FRAMEWORK
#
#                                                          Parameterset Handling
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

import os
from tempfile import mkstemp

from lofar.parameterset import parameterset

class Parset(parameterset):
    """
    This wraps lofar.parameterset to provide a convenient means of iterating
    over the parameterset's keys.

    It should be replaced (eventually) by rolling this functionality directly
    into the C++ implementation.
    """
    def __init__(self, filename=None, caseInsensitive=False):
        super(Parset, self).__init__(filename, caseInsensitive)
        self.keys = []
        if filename:
            self._append_file(filename)

    def add(self, key, value):
        super(Parset, self).add(key, value)
        self.keys.append(key)

    def adoptFile(self, filename, prefix=''):
        super(Parset, self).adoptFile(filename, prefix)
        self._append_file(filename, prefix)

    def clear(self):
        super(Parset, self).clear()
        self.keys = []

    def remove(self, key):
        super(Parset, self).remove(key)
        self.keys.remove(key)

    def replace(self, key, value):
        super(Parset, self).replace(key, value)
        if not key in self.keys:
            self.keys.append(key)

    def subtractSubset(self, baseKey):
        super(Parset, self).subtractSubset(baseKey)
        self.keys = filter(
            lambda key: False if key[:len(baseKey)] == baseKey else True,
            self.keys
        )

    def addStringVector(self, key, vector):
        super(Parset, self).add(key, "[ %s ]" % ", ".join(vector))
        self.keys.append(key)

    def _append_file(self, filename, prefix=''):
        file = open(filename, 'r')
        for line in file:
            key = line.split("=")[0].strip()
            if key:
                self.keys.append(prefix + key)
        file.close()

    def __iter__(self):
        return iter(self.keys)

def get_parset(parset_filename):
    """
    Returns an instance of Parset with the given file loaded.
    """
    return Parset(parset_filename)

def patch_parset(parset, data, output_dir=None):
    """
    Generate a parset file by adding the contents of the data dictionary to
    the specified parset object. Write it to file, and return the filename.
    """
    temp_parset = get_parset(parset)
    for key, value in data.iteritems():
        temp_parset.replace(key, value)
    fd, output = mkstemp()
    temp_parset.writeFile(output)
    os.close(fd)
    return output
