#!/bin/bash

FOLDERS=( $MCU_HOME_DIR/mcms/Bin $MCU_HOME_DIR/cs/bin /mrmx $MCU_HOME_DIR/usr/rmx1000/bin $MCU_HOME_DIR/usr/share/MFA/mfa_x86_env/bin $MCU_HOME_DIR/usr/share/EngineMRM/Bin ) 
rm -rf $MCU_HOME_DIR/tmp/ldd.log

for DIR in ${FOLDERS[@]} ; do
	echo "=============" ;
	echo $DIR;
	cd $DIR ;
	echo "=============" ;
	ldd * 2> /dev/null  | grep -v ":" | sort -u 
done

