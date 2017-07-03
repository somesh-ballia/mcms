#!/bin/sh

# Definitions
#--------------------------------------------------
LOOP_DEV=/dev/loop10
LOOP_DIR=$MCU_HOME_DIR/tmp/loop10
VERSION=$MCU_HOME_DIR/data/current
EMB_EXT2_FILE=/opt/emb.ext2

EMB_EXT2_TAR=$MCU_HOME_DIR/tmp/emb.tgz
EMB_EXT2_NEW=$MCU_HOME_DIR/tmp/emb.ext2

UPGRADE_LOOP_DEV=/dev/loop11
UPGRADE_EMB_MOUNT_POINT=/opt/upgrade_emb

#--------------------------------------------------------------
# $MCU_HOME_DIR/data/current is now the new version that should be mounted
# 1) mount current version
#--------------------------------------------------------------

(

if [ ! -f $VERSION ];
	then 
	echo -n STATUS_NOT_FOUND
	exit 0
fi

KSIZE=`tail -n 99 $VERSION | grep KernelSize | awk -F ' ' '{ print $2 }'`

FSSIZE=`tail -n 99 $VERSION | grep SquashFsSize | awk -F ' ' '{ print $2 }'`


/sbin/losetup -o $KSIZE $LOOP_DEV $VERSION
mkdir -p $LOOP_DIR
/bin/mount -t squashfs $LOOP_DEV $LOOP_DIR -o ro

#--------------------------------------------------------------
# 2) mount the new embedded fs 
#--------------------------------------------------------------

#if new style bin file 
if [ $FSSIZE -ne 0 ]
	then
	echo new style
	# mount emb.ext2 from bin
	/sbin/losetup -o $((KSIZE+FSSIZE)) $UPGRADE_LOOP_DEV $VERSION

	/bin/mount -t ext2 $UPGRADE_LOOP_DEV $UPGRADE_EMB_MOUNT_POINT -o ro
	
#else, old style bin file
else
	echo old style
	#mount emb.ext2 from within squash fs
	/sbin/losetup $UPGRADE_LOOP_DEV $LOOP_DIR$EMB_EXT2_FILE 

	/bin/mount -t ext2 $UPGRADE_LOOP_DEV $UPGRADE_EMB_MOUNT_POINT -o ro
	
fi

#--------------------------------------------------------------
# 3) link new CpuIpmc software
#--------------------------------------------------------------

#if new style bin file - ipmc is part of emb.ext2
if [ $FSSIZE -ne 0 ]
	then
	
	CPU_IPMC=`ls $UPGRADE_EMB_MOUNT_POINT/ipmc/`
	echo $CPU_IPMC

	ln -sf $UPGRADE_EMB_MOUNT_POINT/ipmc/$CPU_IPMC $MCU_HOME_DIR/tmp/upgrade_CpuIpmc
	echo `ls -l $MCU_HOME_DIR/tmp/upgrade_CpuIpmc`

#else, old style bin file - ipmc is under fs : ipmc directory
else
	echo old style
	
	CPU_IPMC=`ls $LOOP_DIR/ipmc/`

	ln -sf $LOOP_DIR/ipmc/$CPU_IPMC $MCU_HOME_DIR/tmp/upgrade_CpuIpmc
fi

) | tee $MCU_HOME_DIR/tmp/expose.txt

