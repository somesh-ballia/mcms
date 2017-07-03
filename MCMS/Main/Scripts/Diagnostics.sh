#!/bin/sh
echo "Diagnostics Mode"

rm -f $DIAGNOSTICS

killall syslogd

umount $MCU_HOME_DIR/output
umount $MCU_HOME_DIR/config

e2fsck /dev/hdb -y
e2fsck /dev/hda1 -y
e2fsck /dev/hda2 -y

# Supports new sda CPU
e2fsck /dev/sdb -y
e2fsck /dev/sda1 -y
e2fsck /dev/sda2 -y

mkfifo $MCU_HOME_DIR/tmp/in
mkfifo $MCU_HOME_DIR/tmp/out

nc 169.254.128.16 3333 < $MCU_HOME_DIR/tmp/out > $MCU_HOME_DIR/tmp/in &

$MCU_HOME_DIR/mcms/Bin/Diagnostics < $MCU_HOME_DIR/tmp/in > $MCU_HOME_DIR/tmp/out

reboot



