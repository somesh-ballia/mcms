#!/bin/sh

#Mount new version
LOOP_DEV=/dev/loop4
ANTI_VIRUS_MOUNT_POINT=$MCU_HOME_DIR/tmp/anti_virus
VERSION=$MCU_HOME_DIR/data/current
OUTPUT_ANTIVIRUS=$MCU_HOME_DIR/output/anti_virus/

mkdir $ANTI_VIRUS_MOUNT_POINT

tail -n 99 $MCU_HOME_DIR/data/current > $MCU_HOME_DIR/tmp/trailer
KSIZE=`cat $MCU_HOME_DIR/tmp/trailer | grep KernelSize | awk -F ' ' '{ print $2 }'`
FSSIZE=`cat $MCU_HOME_DIR/tmp/trailer | grep SquashFsSize | awk -F ' ' '{ print $2 }'`

EMB_SIZE=`cat $MCU_HOME_DIR/tmp/trailer | grep EmbSize | awk -F ' ' '{ print $2 }'`
rm $MCU_HOME_DIR/tmp/trailer

/sbin/losetup -o $((KSIZE+FSSIZE+EMB_SIZE)) $LOOP_DEV $VERSION
/bin/mount -t ext2 $LOOP_DEV $ANTI_VIRUS_MOUNT_POINT -o ro

#Copy AntiVirus files
mkdir -p $OUTPUT_ANTIVIRUS
cp -rf $ANTI_VIRUS_MOUNT_POINT/* $OUTPUT_ANTIVIRUS
chown mcms:mcms $OUTPUT_ANTIVIRUS
chown mcms:mcms $OUTPUT_ANTIVIRUS/*

#Unmount new version	
umount $ANTI_VIRUS_MOUNT_POINT
#/sbin/losetup -d $LOOP_DEV
rmdir $ANTI_VIRUS_MOUNT_POINT
echo -n STATUS_OK
