#!/bin/sh
#
# FormatOutput.sh

PATA=/dev/hdb
SATA=/dev/sda
SATA2=/dev/sdb
SSD=/dev/sda4

cd $MCU_HOME_DIR/mcms
killall -9 Start.sh
./Scripts/Reset.sh

(mount | grep $MCU_HOME_DIR/output | grep $PATA) && umount $MCU_HOME_DIR/output && echo "umount succeed"
(yes | mke2fs -b 4096 -j -F $PATA) && echo "mke2fs succeed" || echo "mke2fs failed" 
mount $PATA $MCU_HOME_DIR/output -o rw && echo "$PATA is mounted to $MCU_HOME_DIR/output" || echo "mount failed"
(mount | grep $MCU_HOME_DIR/output | grep $PATA) && export PATA_MOUNTED=YES

if [ $PATA_MOUNTED = 'YES' ]
then 
  echo "PATA HD was mounted successfully"
else

  (mount | grep $MCU_HOME_DIR/output | grep $SSD) && (umount $MCU_HOME_DIR/output) ; (yes | mke2fs -b 4096 -j $SSD)
  mount $SSD $MCU_HOME_DIR/output -o rw
  (mount | grep $MCU_HOME_DIR/output | grep $SSD) && echo "SSD was mounted successfully." && export SSD_MOUNTED=YES

  if [ $SSD_MOUNTED = 'YES' ]
  then
	echo "SSD HD was mounted successfully"
  else

	# If diagnostics (in production) is stopped in middle of run, hard disk
	# is left with a partition under /dev/sda1.
	# This in turn is mounted as $MCU_HOME_DIR/tmp/usb_stick1 in the following bootup,
	# and therefore hard disk format fails.
	umount $MCU_HOME_DIR/tmp/usb_stick1

	(mount | grep $MCU_HOME_DIR/output | grep $SATA2) && umount $MCU_HOME_DIR/output
	yes | mke2fs -b 4096 -F -j $SATA2
	mount $SATA2 $MCU_HOME_DIR/output -o rw
	(mount | grep $MCU_HOME_DIR/output | grep $SATA2) && export SATA2_MOUNTED=YES
    
	if [ $SATA2_MOUNTED = 'YES' ]
	then
		echo "SATA HD was mounted successfully"
	else
		(mount | grep $MCU_HOME_DIR/output | grep $SATA) && umount $MCU_HOME_DIR/output
		yes | mke2fs -b 4096 -F -j $SATA
		mount $SATA $MCU_HOME_DIR/output -o rw
		(mount | grep $MCU_HOME_DIR/output | grep $SATA) && echo "SATA HD was mounted successfully"
	fi
  fi
fi
