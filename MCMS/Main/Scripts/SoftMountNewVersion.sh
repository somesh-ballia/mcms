#!/bin/sh

LOOP_DEV=/dev/loop10
LOOP_DIR=$MCU_HOME_DIR/tmp/loop10
NEW_VERSION=$MCU_HOME_DIR/data/new_version/new_version.bin


if [ ! -f $NEW_VERSION ];
	then
	echo -n STATUS_NOT_FOUND
	exit 0
fi

KSIZE=`tail -n 99 $NEW_VERSION | grep KernelSize | awk -F ' ' '{ print $2 }'`

NEWFSSIZE=`tail -n 99 $NEW_VERSION | grep NewSqFsSize | awk -F ' ' '{ print $2 }'`

if [ "$NEWFSSIZE" == "0" ] || [ "$NEWFSSIZE" == "" ];
 then
    echo -n STATUS_NO_NEWFSSIZE
    exit 0
 else
    /sbin/losetup -o $KSIZE $LOOP_DEV $NEW_VERSION
    OUT=$?
    if [ "$OUT" != "0" ]
      then
      echo -n STATUS_LOSETUP_FAIL_$OUT
      exit 0
    fi
    mkdir -p $LOOP_DIR
    OUT=$?
    if [ "$OUT" != "0" ]
      then
      echo -n STATUS_MKDIR_FAIL_$OUT
      exit 0
    fi
    /bin/mount -t squashfs $LOOP_DEV $LOOP_DIR -o ro
    OUT=$?
    if [ "$OUT" != "0" ]
      then
      echo -n STATUS_mount_FAIL_$OUT
      exit 0
    fi
fi

echo -n STATUS_OK
