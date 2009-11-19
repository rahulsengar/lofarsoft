export PYTHONPATH="/home/swinbank/apps/lib/python2.5/site-packages/:/opt/LofIm/daily/pyrap/lib:/opt/LofIm/daily/lofar/lib/python2.5/site-packages:/opt/pythonlibs/lib/python/site-packages"
export PATH="/home/swinbank/apps/bin:/usr/local/bin:/usr/bin:/bin"
export LD_LIBRARY_PATH="/home/swinbank/apps/lib:/opt/LofIm/daily/pyrap/lib:/opt/LofIm/daily/casacore/lib:/opt/LofIm/daily/lofar/lib:/opt/wcslib/lib/:/opt/hdf5/lib"
CONTROLPATH=$1
PIDPATH=$CONTROLPATH/engines/`hostname`

mkdir -p $PIDPATH

case "$2" in
  start) 
         NCPU=`cat /proc/cpuinfo | grep ^processor | wc -l`
         for CPU in `seq 1 $NCPU`
            do
                 /sbin/start-stop-daemon --start -b -m             \
                   --pidfile $PIDPATH/ipengine$CPU.pid             \
                   --exec /home/swinbank/apps/bin/ipengine -- \
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
