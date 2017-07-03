#!/bin/sh

export LD_LIBRARY_PATH=`echo $PWD`/Bin/

Scripts/Header.sh 'Vasily Debug VOIP Party'
#Scripts/Cleanup.sh

usleep 200000
Bin/Faults &
usleep 200000
Scripts/LogLog.sh


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
Bin/CSApi &
usleep 200000
Bin/CSMngr &
usleep 200000
Bin/GideonSim &
usleep 200000
#Bin/EndpointsSim &
#usleep 200000
#valgrind --tool=memcheck --suppressions=Scripts/ValgrindSup.txt Bin/ConfParty &
Bin/ConfParty &
#gdb -command run Bin/ConfParty &
usleep 500000
/opt/polycom/apache/bin/apachectl start


sleep 12

Scripts/TestLog.sh 'Card Startup' CM_MEDIA_IP_CONFIG_END_REQ || exit 2000

Bin/ConfParty Scripts/AddVoipConf.xml
sleep 2
Scripts/TestLog.sh 'Add Voip Conf' 'CConf::BridgeConnectionCompleted' || exit 2001

Bin/ConfParty Scripts/AddVoipParty.xml
sleep 10
Scripts/TestLog.sh 'Add Voip Party' 'CH323ChangeModeCntl::OnAudConnectPartyIdleOrAnycase' || exit 2002


exit $exiterror


