#!/bin/sh

LOOP_DEV=/dev/loop10
LOOP_DIR=$MCU_HOME_DIR/tmp/loop10
NEW_VERSION=$MCU_HOME_DIR/data/new_version/new_version.bin
#SCRIPT=$MCU_HOME_DIR/mcms/Scripts/FirmwareCheck.sh




if [ ! -f $NEW_VERSION ];
	then
	echo -n STATUS_NOT_FOUND
	exit 0
fi

KSIZE=`tail -n 99 $NEW_VERSION | grep KernelSize | awk -F ' ' '{ print $2 }'`
FSSIZE=`tail -n 99 $NEW_VERSION | grep SquashFsSize | awk -F ' ' '{ print $2 }'`
EMB_SIZE=`tail -n 99 $FIRMWARE | grep EmbSize | awk -F ' ' '{ print $2 }'`

NEWFSSIZE=0
NEWFSSIZE=`tail -n 99 $NEW_VERSION | grep NewSqFsSize | awk -F ' ' '{ print $2 }'`

if [ NEWFSSIZE == 0 ]
 then
    TOOL_CHAIN_FOLDER=/opt/polycom/carmel/tool_chain/v6/
 
    /sbin/losetup -o $KSIZE $LOOP_DEV $NEW_VERSION
    mkdir -p $LOOP_DIR
    /bin/mount -t squashfs $LOOP_DEV $LOOP_DIR -o ro
 else
    /sbin/losetup -o $((KSIZE+FSSIZE+EMB_SIZE)) $LOOP_DEV $NEW_VERSION
    mkdir -p $LOOP_DIR
    /bin/mount -t squashfs $LOOP_DEV $LOOP_DIR -o ro
fi



echo -n STATUS_OK

