#!/bin/sh
mkdir $MCU_HOME_DIR/tmp/USB
mount -t vfat /dev/sda1 $MCU_HOME_DIR/tmp/USB
cp $MCU_HOME_DIR/tmp/USB/extra/License.cfs $MCU_HOME_DIR/mcms/Cfg
umount $MCU_HOME_DIR/tmp/USB
Reset.sh