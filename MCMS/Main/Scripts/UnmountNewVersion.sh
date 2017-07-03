#!/bin/sh

# Definitions
#--------------------------------------------------
LOOP_DEV=/dev/loop10
LOOP_DIR=$MCU_HOME_DIR/tmp/loop10

#Unmount new version	
umount $LOOP_DIR
#/sbin/losetup -d $LOOP_DEV
rmdir $LOOP_DIR
echo -n STATUS_OK