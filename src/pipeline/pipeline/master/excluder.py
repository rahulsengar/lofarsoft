import sys
from pipeline.support.pyraprunner import pyraprunner

class excluder(pyraprunner):
    @staticmethod
    def remote_function(input, output, station):
        from pipeline.nodes.excluder import excluder_node
        return excluder_node(loghost=loghost, logport=logport).run(input, output, station)

    def __init__(self):
        super(excluder, self).__init__()
        self.optionparser.add_option(
            '--station',
            dest="station",
            help="Name of stations to exclude (e.g. DE001LBA)"
        )

    def _generate_arguments(self):
        return "\"%s\"" % ('\", \"'.join(string_to_list(self.inputs['station'])))

if __name__ == '__main__':
    sys.exit(excluder().main())
