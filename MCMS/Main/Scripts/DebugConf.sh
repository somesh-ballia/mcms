#!/bin/sh

export LD_LIBRARY_PATH=`echo $PWD`/Bin/

Scripts/Cleanup.sh

usleep 200000
Bin/Faults &
usleep 200000
Bin/Logger > $MCU_HOME_DIR/tmp/loglog.txt &
usleep 200000
Bin/Authentication &
usleep 200000
Bin/McuMngr &
usleep 200000
Bin/Cards &
usleep 200000
Bin/Resource &
usleep 200000
Bin/MplApi &
usleep 200000
Bin/GideonSim &
usleep 200000
Bin/EndpointsSim &
usleep 200000
#Bin/ConfParty &
#usleep 200000
/opt/polycom/apache/bin/apachectl start

sleep 1

