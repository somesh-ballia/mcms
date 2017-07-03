#!/bin/sh

# Usage:
# $MCU_HOME_DIR/mcms/Script/SoftUpgrade {factory|fallback|file} <file> <IsNeedSUMCheck> <IsNeedVersionCheck> <LogFilePath>
# for example:
#  1. $MCU_HOME_DIR/mcms/Script/SoftUpgrade fallback
#  2. $MCU_HOME_DIR/mcms/Script/SoftUpgrade file /mnt/usb/RMX_1.0.0.0.bin YES YES $MCU_HOME_DIR/tmp/startup_logs/softUpgrade.log
#
# Exit code:
#--------------------------------------------------
# 0: Succeed to Install.(return "0 RMX_7.8.0.1")
# 1: Failed.(Other reasons)
# 2: Failed.(No file)
# 3: Failed.(Firmware mismatch)
# 4: Failed.(Already installed)
# 5: Failed.(MD5SUM or SHA1SUM failed)
# 6: Failed.(Check version failed)
# 7: Failed.(Cycle failed)
#--------------------------------------------------

# Definitions
#--------------------------------------------------
SOFT_UPGRADE_LOG=$MCU_HOME_DIR/tmp/startup_logs/softUpgrade.log
NEW_VERSION=$MCU_HOME_DIR/data/new_version/new_version.bin
MOUNT_NEW_VERSION=$MCU_HOME_DIR/mcms/Scripts/SoftMountNewVersion.sh
RUN_FIRMWARE_CHECK=$MCU_HOME_DIR/mcms/Scripts/SoftRunFirmwareCheck.sh
UNMOUNT_NEW_VERSION=$MCU_HOME_DIR/mcms/Scripts/SoftUnmountNewVersion.sh
CYCLE_VERSION=$MCU_HOME_DIR/mcms/Scripts/SoftCycleVersions.sh
FILE_AFTER_INSTALL=$MCU_HOME_DIR/mcms/States/systemIsAfterVersionInstallation.flg
FILE_IVR_IMPORT=$MCU_HOME_DIR/mcms/States/defaultIVRConfigImported.flg
LOOP_DIR=$MCU_HOME_DIR/tmp/loop10
FPGA_IMAGE_PATH=$LOOP_DIR/$MCU_HOME_DIR/usr/rmx1000/bin/fpga_upgrade/ninja_fpga_image.bin
FPGA_IMAGE_TMP_PATH=$MCU_HOME_DIR/tmp/ninja_fpga_image.bin
FPGA_IMAGE_READBACK_PATH=$MCU_HOME_DIR/tmp/fpga_image_read_back_file.bin
CURRENT_FPGA_IMAGE_MD5_PATH=$MCU_HOME_DIR/config/sysinfo/fpga_upgrade_md5
FPGA_FORCE_UPGRADE_FLG=$MCU_HOME_DIR/mcms/States/forceFPGAUpgrade.flg

export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin:$LD_LIBRARY_PATH

function Exit()
{
  echo  "End Time: "`date` >> $SOFT_UPGRADE_LOG
  if [ "$1" == "0" ]
    then
    rm -f $FILE_IVR_IMPORT
    rm -f $FILE_AFTER_INSTALL
    echo  "SoftUpgrade script Exit: "$1 >> $SOFT_UPGRADE_LOG
    echo -n "0 $2"
    exit 0
  else
    mount $MCU_HOME_DIR/data -o remount -o rw -o async -o noexec
    rm -f $MCU_HOME_DIR/data/new_version/*
    rm -f $FILE_AFTER_INSTALL
    mount $MCU_HOME_DIR/data -o remount -o ro -o noexec
    echo  "SoftUpgrade script Exit: "$1 >> $SOFT_UPGRADE_LOG
    echo -n $1
    exit $1
  fi
}

function CheckProductType()
{
  TYPE=$1
  LIST=$2
  echo "check ProductType List[$LIST] Type[$TYPE]" >> $SOFT_UPGRADE_LOG

  index=1
  version=`echo $LIST | awk -F ';' '{print $'$index'}'`
  while [ "$version" != "" ];
      do 
         echo  $version >> $SOFT_UPGRADE_LOG
         
         if [ "$version" == "ALL" ];
          then
           echo "found ALL in ProductType list" >> $SOFT_UPGRADE_LOG
           return 0
          fi
          
         if [ "$version" == "$TYPE" ];
          then
           echo "found version match in ProductType list: " $version >> $SOFT_UPGRADE_LOG
           return 0
          fi
          
          index=`expr $index + 1`
          version=`echo $LIST | awk -F ';' '{print $'$index'}'`
     done
  echo "not found version match in ProductType list" >> $SOFT_UPGRADE_LOG
  return 1
}

# Parameters
#--------------------------------------------------
upgrade_type=$1
new_version_file=$2
need_sum_check=$3
need_version_check=$4
log_file_path=$5
FILE_ORG_NAME="RMX_7.8.0.1.bin"

if [ "$log_file_path" != "" ]
  then
  SOFT_UPGRADE_LOG=$log_file_path
fi

echo  "---------------------------" >> $SOFT_UPGRADE_LOG
echo  "Start Time: "`date` >> $SOFT_UPGRADE_LOG
echo  "I am in soft upgrade script" >> $SOFT_UPGRADE_LOG

echo "upgrade_type "  $1 >> $SOFT_UPGRADE_LOG
echo "new_version_file " $2 >> $SOFT_UPGRADE_LOG
echo "need_sum_check " $3 >> $SOFT_UPGRADE_LOG
echo "need_version_check " $4 >> $SOFT_UPGRADE_LOG
echo "log_file_path " $5 >> $SOFT_UPGRADE_LOG


if [ "$upgrade_type" == "factory" ]
  then
    FILE_ORG_NAME=$(readlink $MCU_HOME_DIR/data/factory)
    new_version_file="$MCU_HOME_DIR/data/"$FILE_ORG_NAME
    need_sum_check="NO"
    need_version_check="NO"
elif [ "$upgrade_type" == "fallback" ]
  then
    FILE_ORG_NAME=$(readlink $MCU_HOME_DIR/data/fallback)
    new_version_file="$MCU_HOME_DIR/data/"$FILE_ORG_NAME
    need_sum_check="YES"
    need_version_check="YES"
else
  if [ "$new_version_file" == "" ]
    then
      echo "SoftUpgrade script : new_version_file is not assigned." >> $SOFT_UPGRADE_LOG
      Exit 2
  fi
  if [ "$need_sum_check" != "NO" ]
    then
      need_sum_check="YES"
  fi
  if [ "$need_version_check" != "NO" ]
    then
      need_version_check="YES"
  fi
  FILE_ORG_NAME=$(basename "$new_version_file")

  if [ -f $new_version_file ]
    then
    #Check ProductType in Trailer
    CURRENT_PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`
    PRODUCT_TYPE_LIST=`tail -n 99 $new_version_file | grep ProductType | awk -F ' ' '{ print $2 }'`
    if [ "$CURRENT_PRODUCT_TYPE" == "" ]
      then
      echo "SoftUpgrade script :  Upgrade rejected. Firmware mismatch: can not find current product type." >> $SOFT_UPGRADE_LOG
      Exit 3
    fi
    CheckProductType $CURRENT_PRODUCT_TYPE $PRODUCT_TYPE_LIST
    if [ "$?" != "0" ]
      then
      echo "SoftUpgrade script : Upgrade rejected. Firmware mismatch: ProductType in Trailer: "$PRODUCT_TYPE >> $SOFT_UPGRADE_LOG
      Exit 3
    fi

    #Check FirmwareVersion in Trailer
    NEW_VERSION_NUMBER=`tail -n 99 $new_version_file | grep FirmwareVersion | awk -F ' ' '{ print $2 }' | awk -F '_' '{ print $2 }'`
    CURRENT_BIN=$(readlink $MCU_HOME_DIR/data/current)
    CURRENT_VERSION=$MCU_HOME_DIR/data/$CURRENT_BIN
    CURRENT_VERSION_NUMBER=`tail -n 99 $CURRENT_VERSION | grep FirmwareVersion | awk -F ' ' '{ print $2 }' | awk -F '_' '{ print $2 }'`
    echo "NEW_VERSION_NUMBER " $NEW_VERSION_NUMBER >> $SOFT_UPGRADE_LOG
    echo "CURRENT_VERSION_NUMBER " $CURRENT_VERSION_NUMBER >> $SOFT_UPGRADE_LOG
    if [ "$NEW_VERSION_NUMBER" == "$CURRENT_VERSION_NUMBER" ]
      then
      echo "SoftUpgrade script : Upgrade rejected. new package has the same version as current. " >> $SOFT_UPGRADE_LOG
      Exit 4
    fi
    else
    echo "SoftUpgrade script : new version file not exist" >> $SOFT_UPGRADE_LOG
    Exit 2
  fi
fi

echo "Get new_version_file " $new_version_file >> $SOFT_UPGRADE_LOG
echo "Get need_sum_check " $need_sum_check >> $SOFT_UPGRADE_LOG
echo "Get need_version_check " $need_version_check >> $SOFT_UPGRADE_LOG
echo "Get FILE_ORG_NAME " $FILE_ORG_NAME >> $SOFT_UPGRADE_LOG

# Check if new version_file exists
if [ -f $new_version_file ];
  then
  if [ "$new_version_file" != "$NEW_VERSION" ]
    then
      mount $MCU_HOME_DIR/data -o remount -o rw -o async -o noexec
      cp -f $new_version_file $NEW_VERSION
      OUT=$?
      mount $MCU_HOME_DIR/data -o remount -o ro -o noexec
      if [ "$OUT" != "0" ]
        then
        echo "SoftUpgrade script : copy new version file failed: $new_version_file -> $NEW_VERSION" >> $SOFT_UPGRADE_LOG
        Exit 1
    fi
  fi
  else 
    echo "SoftUpgrade script : new version file not exist: $new_version_file" >> $SOFT_UPGRADE_LOG
    Exit 2
fi

# Check SUM
if [ "$need_sum_check" == "YES" ];
  then
  NEW_VERSION_SIZE=`tail -n 99 $NEW_VERSION | grep NoTrailerSize | awk -F ' ' '{ print $2 }'`
  NEW_VERSION_MD5SUM=`tail -n 99 $NEW_VERSION | grep NoTrailerMD5SUM | awk -F ' ' '{ print $2 }'`
  NEW_VERSION_SHA1SUM=`tail -n 99 $NEW_VERSION | grep NoTrailerSHA1SUM | awk -F ' ' '{ print $2 }'`

  echo "NEW_VERSION_SIZE " $NEW_VERSION_SIZE >> $SOFT_UPGRADE_LOG
  echo "NEW_VERSION_MD5SUM " $NEW_VERSION_MD5SUM >> $SOFT_UPGRADE_LOG
  echo "NEW_VERSION_SHA1SUM " $NEW_VERSION_SHA1SUM >> $SOFT_UPGRADE_LOG

  CALCULATE_MD5SUM=`head -c $NEW_VERSION_SIZE $NEW_VERSION | $MCU_HOME_DIR/mcms/Bin/openssl dgst -md5 | cut -d ' ' -f 2`
  echo "CALCULATE_MD5SUM " $CALCULATE_MD5SUM >> $SOFT_UPGRADE_LOG

  if [ "$NEW_VERSION_MD5SUM" != "$CALCULATE_MD5SUM" ]
    then
    echo "SoftUpgrade script : MD5SUM check failed" >> $SOFT_UPGRADE_LOG
    Exit 5
  fi

  CALCULATE_SHA1SUM=`head -c $NEW_VERSION_SIZE $NEW_VERSION | $MCU_HOME_DIR/mcms/Bin/openssl dgst -sha1 | cut -d ' ' -f 2`
  echo "CALCULATE_SHA1SUM " $CALCULATE_SHA1SUM >> $SOFT_UPGRADE_LOG

  if [ "$NEW_VERSION_SHA1SUM" != "$CALCULATE_SHA1SUM" ]
    then
    echo "SoftUpgrade script : SHA1SUM check failed" >> $SOFT_UPGRADE_LOG
    Exit 5
  fi

  echo "SoftUpgrade script : MD5SUM and SHA1SUM check succeed" >> $SOFT_UPGRADE_LOG
fi

# Check Version
if [ "$need_version_check" == "YES" ];
  then
  echo "SoftUpgrade script : Check Version" >> $SOFT_UPGRADE_LOG
  OUT=`$MOUNT_NEW_VERSION`
  if [ "$OUT" != "STATUS_OK" ]
    then
    echo "SoftUpgrade script : SoftMountNewVersion failed. return "$OUT >> $SOFT_UPGRADE_LOG
    Exit 6
  fi

  FirmwareCheckResult=`$RUN_FIRMWARE_CHECK $log_file_path`

  #copy FPGA Image to $MCU_HOME_DIR/tmp
  if [ "$CURRENT_PRODUCT_TYPE" == "NINJA" ]
  then
    cp -f $FPGA_IMAGE_PATH $FPGA_IMAGE_TMP_PATH
    OUT=$?
    echo "SoftUpgrade script : cp -f $FPGA_IMAGE_PATH $FPGA_IMAGE_TMP_PATH failed. return "$OUT >> $SOFT_UPGRADE_LOG
  fi

  OUT=`$UNMOUNT_NEW_VERSION`
  if [ "$OUT" != "STATUS_OK" ]
    then
    echo "SoftUpgrade script : SoftUnmountNewVersion failed. return "$OUT >> $SOFT_UPGRADE_LOG
    Exit 6
  fi

  if [ "$FirmwareCheckResult" != "YES" ]
    then
    echo "SoftUpgrade script : SoftRunFirmwareCheck failed. return "$OUT >> $SOFT_UPGRADE_LOG
    Exit 6
  fi
fi

# Cycle Version
NEW_VERSION_NAME=`tail -n 99 $NEW_VERSION | grep FirmwareVersion | awk -F ' ' '{ print $2 }'`
mount $MCU_HOME_DIR/data -o remount -o rw -o async -o noexec
$CYCLE_VERSION $NEW_VERSION_NAME $log_file_path
OUT=$?
mount $MCU_HOME_DIR/data -o remount -o ro -o noexec
if [ "$OUT" != "0" ]
  then
  if [ "$OUT" == "2" ]
    then
    echo "SoftUpgrade script : SoftCycleVersions failed. return "$OUT >> $SOFT_UPGRADE_LOG
    Exit 4
  fi
  echo "SoftUpgrade script : SoftCycleVersions failed. return "$OUT >> $SOFT_UPGRADE_LOG
  Exit 7
fi

#NINJA: FPGA Upgrade
if [ "$CURRENT_PRODUCT_TYPE" == "NINJA" ]
then
  if [ -s $FPGA_IMAGE_TMP_PATH ]
  then
    sum1=`/usr/bin/md5sum $FPGA_IMAGE_TMP_PATH | awk '{print $1}'`
    OUT=$?
    echo "SoftUpgrade script : /usr/bin/md5sum $FPGA_IMAGE_TMP_PATH. return "$OUT >> $SOFT_UPGRADE_LOG

if [ -f $FPGA_FORCE_UPGRADE_FLG ];
  then
    IsSameAsCurrentFPGA="NO"
  else
    IsSameAsCurrentFPGA=`$MCU_HOME_DIR/mcms/Scripts/NinjaFPGACompare.sh $FPGA_IMAGE_TMP_PATH`
fi

    if [ "$IsSameAsCurrentFPGA" == "YES" ]
    then
      echo "SoftUpgrade script : FPGA is the same to current, Skip FPGA Upgrade." >> $SOFT_UPGRADE_LOG
    else
      echo "SoftUpgrade script : FPGA is not the same." >> $SOFT_UPGRADE_LOG
      DATE=`date`
      echo "SoftUpgrade script : FPGA Upgrade start $DATE." >> $SOFT_UPGRADE_LOG
      
      #fpga upgrade
      $MCU_HOME_DIR/usr/rmx1000/bin/fpga_upgrade/fpga_upgrade $FPGA_IMAGE_TMP_PATH /dev/mtd0 $FPGA_IMAGE_READBACK_PATH
      OUT=$?
      echo "SoftUpgrade script : $MCU_HOME_DIR/usr/rmx1000/bin/fpga_upgrade/fpga_upgrade $FPGA_IMAGE_TMP_PATH /dev/mtd0 $FPGA_IMAGE_READBACK_PATH. return "$OUT >> $SOFT_UPGRADE_LOG

      #upgrade check
      sum2=`/usr/bin/md5sum $FPGA_IMAGE_READBACK_PATH | awk '{print $1}'`
      OUT=$?
      echo "SoftUpgrade script : /usr/bin/md5sum $FPGA_IMAGE_READBACK_PATH. return "$OUT >> $SOFT_UPGRADE_LOG

      DATE=`date`
      echo "SoftUpgrade script : FPGA Upgrade end $DATE." >> $SOFT_UPGRADE_LOG

      if [ "$sum1" == "$sum2" ]
      then
        echo "SoftUpgrade script : FPGA Upgrade OK." >> $SOFT_UPGRADE_LOG
        echo -n $sum2 > $CURRENT_FPGA_IMAGE_MD5_PATH
        rm -f $FPGA_FORCE_UPGRADE_FLG
      else
        echo "SoftUpgrade script : read back md5 is not the same to image file. FPGA Upgrade FAILED." >> $SOFT_UPGRADE_LOG
        Exit 1
      fi
    fi
  else
    echo "SoftUpgrade script : $FPGA_IMAGE_TMP_PATH not exist. Upgrade failed." >> $SOFT_UPGRADE_LOG
    Exit 1
  fi
fi

# touch $FILE_AFTER_INSTALL
# chown mcms:mcms $FILE_AFTER_INSTALL

Exit 0 $NEW_VERSION_NAME
