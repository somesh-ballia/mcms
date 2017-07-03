#!/bin/sh

# GetUdpPorts.sh
# Written by: Shachar Bar <shachar.bar@polycom.com>
#

if [ "$#" != 1 ]; then
	echo "Usage: $0 Interface_Name"
	exit 1
fi
/sbin/ifconfig $1 &> /dev/null
if [ "$?" != '0' ]; then
	echo "Interface $1 wasn't found"
	exit 2
fi
TMP_FILE="/tmp/`basename $0`.$$.out"
OUT_FILE="/tmp/`basename $0 | cut -d'.' -f1`.$1.out"

MyIP=`/sbin/ifconfig $1 | grep 'inet addr' | tr -s ' ' | cut -d':' -f2 | cut -d' ' -f1`

rm -f $TMP_FILE $OUT_FILE &> /dev/null

if [ $USER != 'root' ]; then
	sudo netstat -nuap | awk '{print $4,$6}' | grep -E "^$MyIP|^0.0.0.0|^:::" > $TMP_FILE 2> /dev/null
else	
	netstat -nuap | awk '{print $4,$6}' | grep -E "^$MyIP|^0.0.0.0|^:::" > $TMP_FILE 2> /dev/null
fi
while read Line; do
	if [ "X$Line" == "X" ]; then
		continue
	fi
	F1=`echo "$Line" | cut -d' ' -f1`
	F2=`echo "$Line" | cut -d' ' -f2`
	if [ `echo $F1 | grep ':.*:'` ]; then
		PORT=`echo $F1 | awk -F: '{ print $NF }'` 
	else
		PORT=`echo $F1 | cut -d':' -f2`
	fi
	if [ -f $OUT_FILE ]; then
		if [ ! `grep "^$PORT," $OUT_FILE` ]; then
			PROC=`echo $F2 | cut -d'/' -f2`
			echo "$PORT,$PROC" >> $OUT_FILE
		fi
	else
		PROC=`echo $F2 | cut -d'/' -f2`
		echo "$PORT,$PROC" >> $OUT_FILE
	fi
done < $TMP_FILE
chmod a+r $OUT_FILE
rm -f $TMP_FILE &> /dev/null
exit 0

