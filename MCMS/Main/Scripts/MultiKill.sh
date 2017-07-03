#!/bin/sh
export LD_LIBRARY_PATH=`echo $PWD`/Bin/

erorrcode=0
Bin/McuCmd kill McmsDaemon >/dev/null

PROCESSES="ConfParty EndpointsSim GideonSim CSMngr CSApi MplApi Resource Cards McuMngr Authentication Logger Faults Configurator McmsDaemon QAAPI CDR ExchangeModule NotificationMngr"
sleep 8

for process in $PROCESSES
do
  if ps -e | grep $process ; 
  then
      Scripts/ErrorMsg.sh "Failed killing" $process
      erorrcode=100
      echo failed killing $process - $errorcode
  else
      echo prcoess $process killed
  fi
done


exit $errorcode


