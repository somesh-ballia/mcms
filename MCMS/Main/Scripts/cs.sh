#!/bin/sh


Bin/Logger > $MCU_HOME_DIR/tmp/loglog.txt &
usleep 200000

Bin/Faults &
usleep 200000

Bin/CSApi &
usleep 200000

Bin/McuMngr &
usleep 200000

#Bin/CSMngr &
#usleep 200000

/opt/polycom/apache/bin/apachectl start

