import shlex, subprocess, threading, logging
from contextlib import contextmanager
from lofarpipe.support.clusterdesc import ClusterDesc
from lofarpipe.support.clusterdesc import get_compute_nodes, get_head_node

class ClusterHandler(object):
    def __init__(self, config, logger=None):
        if not logger:
            logging.basicConfig()
            self.logger = logging.getLogger()
        else:
            self.logger = logger
        clusterdesc = ClusterDesc(config.get('cluster', 'clusterdesc'))
        self.head_node = get_head_node(clusterdesc)[0]
        self.compute_nodes = get_compute_nodes(clusterdesc)
        self.script_path = config.get('deploy', 'script_path')
        self.config = config

    def start_cluster(self, nproc=""):
        # Optional nproc argument specifies number of engines per node
        self.__start_controller()
        self.__start_engines(nproc)

    def stop_cluster(self):
        self.__stop_controller()
        self.__stop_engines()

    def __execute_ssh(self, host, command):
        ssh_cmd = shlex.split("ssh -x %s -- %s" % (host, command))
        subprocess.check_call(ssh_cmd)
        self.logger.info("  * %s" % (host))

    def __multinode_ssh(self, nodes, command):
        ssh_connections = [
            threading.Thread(
                target = self.__execute_ssh,
                args = (node, command)
            ) for node in nodes
        ]
        [thread.start() for thread in ssh_connections]
        [thread.join() for thread in ssh_connections]

    def __start_controller(self):
        self.logger.info("Starting controller:")
        controlpath = self.config.get('DEFAULT', 'runtime_directory')
        controller_ppath = self.config.get('deploy', 'controller_ppath')
        self.__execute_ssh(self.head_node, "bash %s/ipcontroller.sh %s start %s" % (self.script_path, controlpath, controller_ppath))
        self.logger.info("done.")

    def __stop_controller(self):
        self.logger.info("Stopping controller:")
        controlpath = self.config.get('DEFAULT', 'runtime_directory')
        controller_ppath = self.config.get('deploy', 'controller_ppath')
        self.__execute_ssh(self.head_node, "bash %s/ipcontroller.sh %s stop %s" % (self.script_path, controlpath, controller_ppath))
        self.logger.info("done.")

    def __start_engines(self, nproc):
        self.logger.info("Starting engines:")
        controlpath = self.config.get('DEFAULT', 'runtime_directory')
        engine_ppath = self.config.get('deploy', 'engine_ppath')
        engine_lpath = self.config.get('deploy', 'engine_lpath')
        command = "bash %s/ipengine.sh %s start %s %s %s" % (self.script_path, controlpath, engine_ppath, engine_lpath, str(nproc))
        self.__multinode_ssh(self.compute_nodes, command)
        self.logger.info("done.")

    def __stop_engines(self):
        self.logger.info("Stopping engines:")
        controlpath = self.config.get('DEFAULT', 'runtime_directory')
        command= "bash %s/ipengine.sh %s stop" % (self.script_path, controlpath)
        self.__multinode_ssh(self.compute_nodes, command)
        self.logger.info("done.")

@contextmanager
def ipython_cluster(config, logger, nproc=""):
    cluster_handler = ClusterHandler(config, logger)
    cluster_handler.start_cluster(nproc)
    yield
    cluster_handler.stop_cluster()
