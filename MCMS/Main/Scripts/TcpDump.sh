#!/bin/sh
if [ "$1" = "" ]
then
    FILE="tcp.dump";
else
    FILE=$1;
fi
echo "Press control-c to end capture"
echo "Use http://MACHINE_IP/LogFiles/"$FILE".gz to get the file"
echo
echo
FILE_FULL=$MCU_HOME_DIR/mcms/LogFiles/$FILE
trap "sleep 1; rm -f $FILE_FULL.gz; gzip $FILE_FULL" 2
tcpdump -s0 -w $FILE_FULL
trap 2
