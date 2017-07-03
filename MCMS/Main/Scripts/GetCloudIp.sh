#!/bin/bash

http_code=$(curl --connect-timeout 3 -sL -w "%{http_code}" -o $MCU_HOME_DIR/tmp/$$ "http://169.254.169.254/openstack")
if [ $? -eq 0 ] && [ $http_code -eq 200 ]; then
	echo "Getting cloud IP"
        http_code=$(curl --connect-timeout 2 -sL -w "%{http_code}" -o $MCU_HOME_DIR/tmp/$$ "http://169.254.169.254/latest/meta-data/public-ipv4")
        if [ $? -eq 0 ] && [ $http_code -eq 200 ]; then
        	echo "$MCU_HOME_DIR/tmp/cloudIp file is ready"
                mv -f $MCU_HOME_DIR/tmp/$$ $MCU_HOME_DIR/tmp/cloudIp
		exit 0
        else
        	echo "Nothing to create"
                rm -rf $MCU_HOME_DIR/tmp/$$
		exit 1
        fi
fi



#First test if running under VPC - then no need to create cloudIp file
http_code=$(curl --connect-timeout 3  -sL -w "%{http_code}" -o $MCU_HOME_DIR/tmp/$$ "http://169.254.169.254/latest/meta-data/network/interfaces/macs/")
if [ $? -eq 0 ] && [ $http_code -eq 200 ]; then
        MAC=`cat $MCU_HOME_DIR/tmp/$$`

        http_code=$(curl --connect-timeout 2 -sL -w "%{http_code}" -o $MCU_HOME_DIR/tmp/$$ "http://169.254.169.254/latest/meta-data/network/interfaces/macs/$MAC/vpc-id")
        if [ $? -eq 0 ] && [ $http_code -eq 200 ]; then
                echo "Instance running under VPC"
                echo "Nothing to create"
                rm -rf $MCU_HOME_DIR/tmp/$$
                rm -rf $MCU_HOME_DIR/tmp/cloudIp
		exit 1
        else
                echo "Getting cloud IP"
                http_code=$(curl --connect-timeout 2 -sL -w "%{http_code}" -o $MCU_HOME_DIR/tmp/$$ "http://169.254.169.254/latest/meta-data/public-ipv4")
                if [ $? -eq 0 ] && [ $http_code -eq 200 ]; then
                        echo "$MCU_HOME_DIR/tmp/cloudIp file is ready"
                        mv -f $MCU_HOME_DIR/tmp/$$ $MCU_HOME_DIR/tmp/cloudIp
			exit 0
                else
                        echo "Nothing to create"
                        rm -rf $MCU_HOME_DIR/tmp/$$
			exit 1
                fi
        fi
else
        echo "No, mac address, Nothing to create"
	exit 1
fi
