#!/bin/sh

export LD_LIBRARY_PATH=`echo $PWD`/Bin/

echo  Test Kill $1
Scripts/Cleanup.sh
e=0
usleep 200000
Scripts/LogLog.sh
usleep 500000
Bin/$1 &
sleep 1
Bin/McuCmd kill $1 >/dev/null
sleep 1


Scripts/TestLog.sh 'Killing' 'CProcessBase::TearDown' || e=2008

Scripts/batchkill.sh
killall -9 tail -q && echo 'kill tail'
Scripts/FindException.sh || e=2999
exit $e


