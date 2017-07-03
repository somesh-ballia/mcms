#!/bin/bash
# GesherLightSrv.sh

################################ HELP ##################################
if [ "$1" == "" ]
then
	echo "usage: GesherLightSrv.sh COMMAND"
	echo "commands:"
	echo "start - start Light Server processes"
	echo "stop - start Light Server processes"
	exit 
fi

################################## START ##################################
start ()
{
	echo $LD_LIBRARY_PATH
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MCU_HOME_DIR/mcms/Bin 
	export LD_LIBRARY_PATH
	rm -f $MCU_HOME_DIR/tmp/light1.log
	mv $MCU_HOME_DIR/tmp/light.log $MCU_HOME_DIR/tmp/light1.log
	#nohup $MCU_HOME_DIR/mcms/Bin/LightSrv 2>&1 1>$MCU_HOME_DIR/tmp/light.log & 
	$MCU_HOME_DIR/mcms/Bin/LightSrv 2>&1 1>$MCU_HOME_DIR/tmp/light.log &
}

################################## STOP ##################################
stop () 
{
    #kill LightSrv 
    killall -HUP LightSrv 2>/dev/null
}

echo -e "\nRunning GesherLightSrv $1 :\n=================================="
        
case "$1" in
start)
	start
	;;
stop)
	stop	
	;;
*)
	echo "$1: Action not supported."
	;;
esac
	
