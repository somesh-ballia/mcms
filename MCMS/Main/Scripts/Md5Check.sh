#! /bin/sh

$MCU_HOME_DIR/mcms/Scripts/RemountData.sh
RESULT=$?

ALLFIRMWARE=`ls -l $MCU_HOME_DIR/data | grep current | awk -F ' ' '{print $11}'`
#echo $ALLFIRMWARE
FIRMWARE1=`echo $ALLFIRMWARE |awk -F ' ' '{print $1}'`
FIRMWARE1=$MCU_HOME_DIR/data/$FIRMWARE1

NOTRAILERSIZE=`tail -n 99 $FIRMWARE1 | grep NoTrailerSize | awk -F ' ' '{ print $2 }'`
CALNTMD5SUM=`dd if=$FIRMWARE1 bs=$NOTRAILERSIZE count=1 | md5sum | awk -F ' ' '{ print $1 }'`
NTMD5SUM=`tail -n 99 $FIRMWARE1 | grep NoTrailerMD5SUM | awk -F ' ' '{ print $2 }'`
if [ $NTMD5SUM = $CALNTMD5SUM ]; then
     #echo md5sum of no trailer part OK.
      return 0
else
     #echo Bad md5sum of no trailer part.
      return -1;
fi
