#!/bin/sh
#RemountData.sh

echo "Remount with option -o noexec"
mount /dev/hda3 $MCU_HOME_DIR/data -o remount -o rw -o async || mount /dev/sda3 $MCU_HOME_DIR/data -o remount -o rw -o async || mount /dev/sdb3 $MCU_HOME_DIR/data -o remount -o rw -o async
