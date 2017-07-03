#!/bin/bash
export LD_LIBRARY_PATH=`echo $PWD`/Bin/
Scripts/Header.sh 'Card Startup'
Scripts/Cleanup.sh

Bin/McmsDaemon > loglog.txt &

sleep 3
echo 'Killing McuMngr...'
killall -9 McuMngr -q
sleep 1
ps | grep McuMngr > /dev/null && (echo 'McuMngr wasnt killed !!!' ; exit 101)
echo 'McuMngr Killed'
sleep 10
ps | grep McuMngr > /dev/null || (echo 'McuMngr wasnt cloned' ; exit 102)
echo 'McuMngr Cloned'
sleep 1
echo 'Killing McmsDaemon'
Bin/McuCmd kill McmsDaemon
sleep 10
ps | grep McmsDaemon > /dev/null && (echo 'McmsDaemon wasnt killed !!!' ; exit 103)
echo 'McmsDaemon Killed'

Scripts/batchkill.sh
killall -9 tail -q
find -name core*

#Scripts/FindException.sh && exit 2999
exit 0


