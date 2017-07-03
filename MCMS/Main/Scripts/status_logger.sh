#!/bin/bash

if [[ $# -ne 4 ]] ; then
	echo "Usage: $0 <process name to wait on> <path to log file> <seconds to repeat on fail status> <seconds to repeat on OK status>"
	exit 1
fi

WAIT_FOR=$1
LOG_PATH=$2
FAIL_INTERVAL=$3
OK_INTERVAL=$4

sudo rmtmp.sh $LOG_PATH >& /dev/null

echo "Waiting for $WAIT_FOR: `date`" >> $LOG_PATH

while [[ ! `ps -ef | grep $WAIT_FOR | grep -v $0 | grep -v grep` ]]; do
	sleep 1
done

echo "$WAIT_FOR started: `date`" >> $LOG_PATH

while [[ `ps -ef | grep $WAIT_FOR  | grep -v $0 | grep -v grep` ]]; do
	if [[ `$MCU_HOME_DIR/mcms/Scripts/SoftMcuStatus.sh | tee -a  $LOG_PATH | grep -i fail >& /dev/null` ]] ; then
		sleep $OK_INTERVAL
	else
		sleep $FAIL_INTERVAL
	fi	
done

echo "$WAIT_FOR finished: `date`" >> $LOG_PATH
exit 0
