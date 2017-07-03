#!/bin/sh

# Definitions
#--------------------------------------------------
SAFE_UPGRADE_LOG=$MCU_HOME_DIR/tmp/startup_logs/NinjaFPGACompare.log

FPGA_IMAGE_PATH=$1
FPGA_READBACK_HEAD_PATH=$MCU_HOME_DIR/tmp/fpga_read_back_head_file.bin
FPGA_IMAGE_READBACK_HEAD_PATH=$MCU_HOME_DIR/tmp/fpga_image_read_back_head_file.bin
FPGA_READBACK_TAIL_TEMP_PATH=$MCU_HOME_DIR/tmp/fpga_read_back_tail_temp_file.bin
FPGA_READBACK_TAIL_PATH=$MCU_HOME_DIR/tmp/fpga_read_back_tail_file.bin
FPGA_IMAGE_READBACK_TAIL_PATH=$MCU_HOME_DIR/tmp/fpga_image_read_back_tail_file.bin

(
echo "FPGA_IMAGE_PATH "  $1  >> $SAFE_UPGRADE_LOG

#read back fpga head
/bin/dd if=/dev/mtdblock0 of=$FPGA_READBACK_HEAD_PATH bs=65536 count=4

#read back fpga image head
/bin/dd if=$FPGA_IMAGE_PATH of=$FPGA_IMAGE_READBACK_HEAD_PATH bs=65536 count=4

sum=`/usr/bin/md5sum $FPGA_READBACK_HEAD_PATH | awk '{print $1}'`
sumImage=`/usr/bin/md5sum $FPGA_IMAGE_READBACK_HEAD_PATH | awk '{print $1}'`

echo "Current FPGA head:       " $sum  >> $SAFE_UPGRADE_LOG
echo "Current FPGA Image head: " $sumImage  >> $SAFE_UPGRADE_LOG

if [ "$sum" != "$sumImage" ]
then
  echo -n NO
  exit 0
fi

#read back fpga tail
/bin/dd if=/dev/mtdblock0 of=$FPGA_READBACK_TAIL_TEMP_PATH bs=65536 count=1 seek=0 skip=309
/bin/dd if=$FPGA_READBACK_TAIL_TEMP_PATH of=$FPGA_READBACK_TAIL_PATH bs=22812 count=1

#read back fpga image tail
/bin/dd if=$FPGA_IMAGE_PATH of=$FPGA_IMAGE_READBACK_TAIL_PATH bs=65536 count=1 seek=0 skip=309

sumTail=`/usr/bin/md5sum $FPGA_READBACK_TAIL_PATH | awk '{print $1}'`
sumImageTail=`/usr/bin/md5sum $FPGA_IMAGE_READBACK_TAIL_PATH | awk '{print $1}'`

echo "Current FPGA tail:       " $sumTail  >> $SAFE_UPGRADE_LOG
echo "Current FPGA Image tail: " $sumImageTail  >> $SAFE_UPGRADE_LOG

if [ "$sumTail" != "$sumImageTail" ]
then
  echo -n NO
else
  echo -n YES
fi

) | tee -a $SAFE_UPGRADE_LOG

