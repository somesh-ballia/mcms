#!/bin/sh

sudo /sbin/ifconfig eth0 up 2>/dev/null
sudo /sbin/ifconfig eth1 up 2>/dev/null
sudo /sbin/ifconfig eth2 up 2>/dev/null
sudo /sbin/ifconfig eth3 up 2>/dev/null
for i in {1..10}
do
	echo "check whether eth0 is up"
	res=`sudo /sbin/ethtool eth0 | grep 'Link detected: yes'`
	if [ "$res" != "" ]
	then
    		echo eth0 is ready
    		break 
     	else
    		echo eth0 not ready $i
    		sleep 1 
     	fi
done
