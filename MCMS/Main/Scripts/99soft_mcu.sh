#!/bin/bash
#
# /etc/init.d/soft_mcu
#
# description: This is the management script of the mrm service.
#
#
# chkconfig: 345 99 01
# 



start() {
	echo "Starting SoftMcu"
	if [ "$1" != "" ]; then
	   echo "running $1 under Valgrind"
	fi
	ulimit -c unlimited
	ulimit -s 10240	
	
	cd $MCU_HOME_DIR/mcms
	. $MCU_HOME_DIR/mcms/Scripts/SoftMcuExports.sh
	./Scripts/GesherStart.sh target $1 1>/dev/null 2>/dev/null &
}

debug() {
	echo "Starting SoftMcu"
	if [ "$1" != "" ]; then
	   echo "running $1 under Valgrind"
	fi	
	touch $MCU_HOME_DIR/tmp/stop_monitor
	cd $MCU_HOME_DIR/mcms
	. $MCU_HOME_DIR/mcms/Scripts/SoftMcuExports.sh
	./Scripts/GesherStart.sh target $1 1>$MCU_HOME_DIR/tmp/console.log 2>&1 &
}

stopall() {
	echo "Stopping SoftMcu"
	rm -f $MCU_HOME_DIR/tmp/stop_monitor
	sudo kill -9 `ps -ef | grep -v grep | grep SoftMcuWatcher.sh  | awk -F ' ' '{print $2}'`
	su - mcms -c "cd $MCU_HOME_DIR/mcms ; . $MCU_HOME_DIR/mcms/Scripts/SoftMcuExports.sh ; nohup ./Scripts/GesherStart.sh stop &> /dev/null"
	
	kill -9 `ps -e | grep -v grep | grep ApacheModule | awk -F ' ' '{ print $1 }'`
	cd $MCU_HOME_DIR/usr/rmx1000/bin
	killall -9 launcher
	killall *
	killall httpd
	cd -
	
	while ( netstat -t -l -n -p | grep "0.0.0.0:61000" ) 
    do 
    	sleep 2
        echo "wait for mermaid (port 61000)"; 
        /mrmx/kill-rmx1000.sh &> /dev/null
    done
	retriveResource
}

stop() {
	echo "Stopping SoftMcu"

	su - mcms -c "cd $MCU_HOME_DIR/mcms ; . $MCU_HOME_DIR/mcms/Scripts/SoftMcuExports.sh ; nohup ./Scripts/GesherStart.sh stop &> /dev/null"
	
	kill -9 `ps -e | grep -v grep | grep ApacheModule | awk -F ' ' '{ print $1 }'`
	cd $MCU_HOME_DIR/usr/rmx1000/bin
	killall -9 launcher
	killall *
	killall httpd
	cd -
	
	while ( netstat -t -l -n -p | grep "0.0.0.0:61000" ) 
    do 
    	sleep 2
        echo "wait for mermaid (port 61000)"; 
        /mrmx/kill-rmx1000.sh &> /dev/null
    done
	retriveResource
}

retriveResource() {
	echo "retrieve resource"
	IDS=`ipcs  -m |  awk '{print $2}'`
	for i in $IDS ; do ipcrm -m $i ; done
	IDS=`ipcs  -q |  awk '{print $2}'`
	for i in $IDS ; do ipcrm -q $i ; done
}

restart() {
	echo "Restarting SoftMcu"
	stop
	start
}

status() {
        test=`pgrep GesherMcmsStart.sh`
        if [[ $test != "" ]]; then
		echo "The service is up" 
	else 
		echo "The service is down" 
	fi
}

case $1 in 
start)
	if [[ `status` == "The service is up" ]];then
                echo "MCU is already running"
        else
                start $2
        fi
	;;
stop)
	stop
	;;
restart)
	restart
	;;
status)
	status
	;;
retrieve)
	retriveResource
	;;
stopall)
	stopall
	;;
debug)
	debug
	;;
*)
	echo "$1 is unknown."
esac
