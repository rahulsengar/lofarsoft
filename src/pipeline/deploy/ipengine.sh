CONTROLPATH=$1
export PYTHONPATH=$3
export LD_LIBRARY_PATH=$4
PIDPATH=$CONTROLPATH/engines/`hostname`

mkdir -p $PIDPATH

case "$2" in
  start) 
         NCPU=`cat /proc/cpuinfo | grep ^processor | wc -l`
         for CPU in `seq 1 $NCPU`
            do
                 /sbin/start-stop-daemon --start -b -m             \
                   --pidfile $PIDPATH/ipengine$CPU.pid             \
                   --exec /opt/pipeline/dependencies/bin/ipengine -- \
                   --furl-file=$CONTROLPATH/engine.furl            \
                   --logfile=$PIDPATH/log
            done
         ;;
  stop)
         for PIDFILE in $PIDPATH/ipengine*.pid
             do
                 /sbin/start-stop-daemon --stop --pidfile $PIDFILE
             done
         ;;
  *)
         echo "Usage: $0 <controlpath> {start|stop}" >&2
         exit 1
         ;;
esac

exit 0
