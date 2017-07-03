#!/bin/sh
ulimit -v unlimited

FIRMWARE=$1
# Loop device for mounting the version file
LOOP_DEV=/dev/loop0
# File system type of images 
FSTYPE=squashfs
MNT=$MCU_HOME_DIR/tmp/new_version_test

if [ "`whoami`" != "root" ]; then
    echo error: Must be root
    exit 1;
fi

if test -e $FIRMWARE; then 
    echo $FIRMWARE exists  > /dev/null;
else
    echo $FIRMWARE does not exist;
    exit 2;
fi

# Get NoTrailerSize
NOTRAILERSIZE=`tail -n 99 $FIRMWARE | grep NoTrailerSize | awk -F ' ' '{ print $2 }'`

# Calculate NoTrailerMD5SUM
CALNTMD5SUM=`dd if=$FIRMWARE bs=$NOTRAILERSIZE count=1 | md5sum | awk -F ' ' '{ print $1 }'` 2>/dev/null

dd if=$FIRMWARE bs=$NOTRAILERSIZE count=1 > $MCU_HOME_DIR/output/notrailer

# Calculate NoTrailerSHA1SUM
CALNTSHA1SUM=`$MCU_HOME_DIR/mcms/openssl sha1 $MCU_HOME_DIR/output/notrailer | awk -F ' ' '{ print $2 }'` 2>/dev/null

rm $MCU_HOME_DIR/output/notrailer

# Get NoTrailerMD5SUM from Trailer
NTMD5SUM=`tail -n 99 $FIRMWARE | grep NoTrailerMD5SUM | awk -F ' ' '{ print $2 }'`

# Get NoTrailerSHA1SUM from Trailer
NTMD5SUM=`tail -n 99 $FIRMWARE | grep NoTrailerSHA1SUM | awk -F ' ' '{ print $2 }'`

JITC=$(cat $MCU_HOME_DIR/mcms/JITC_MODE.txt)
if [ "$JITC" != "YES" ]
then
# Verify md5sum identity
    if [ "$NTMD5SUM" = "$CALNTMD5SUM" ]; then
	echo md5sum of no trailer part OK.  > /dev/null
    else
	echo error: Bad md5sum of no trailer part.
	exit 3;
    fi
fi

if [ "$NTSHA1SUM" = "$CALNTSHA1SUM" ]; then
echo sha1 of no trailer part OK.  > /dev/null
else
echo error: Bad openssl sha1 of no trailer part.
exit 4;
fi

# Get kernel size
KSIZE=`tail -n 99 $FIRMWARE | grep KernelSize | awk -F ' ' '{ print $2 }'`
#FSSIZE=`tail -n 99 $FIRMWARE | grep SquashFsSize | awk -F ' ' '{ print $2 }'`


# put firmware on loop device for mounting

if /sbin/losetup $LOOP_DEV $FIRMWARE -o $KSIZE; then
    echo create loop device - OK > /dev/null;
else
    echo error: Failed creating loop device;
    exit 4;
fi
	
# mount firmware
mkdir -p $MNT

if mount -t $FSTYPE $LOOP_DEV $MNT -o ro; then
    echo mount OK > /dev/null;
else
    echo error: Fail mounting $1;
    # rollback
    rmdir $MNT;
    losetup -d $LOOP_DEV;
    exit 5;
fi

if test -e $MNT/version.txt; then 
    cat $MNT/version.txt
else
    echo error: version.txt does not exist;
    umount $MNT
    rmdir $MNT
    exit 6;
fi

umount $MNT
rmdir $MNT
exit 0
