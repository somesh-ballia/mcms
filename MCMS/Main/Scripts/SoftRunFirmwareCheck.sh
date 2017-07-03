#!/bin/sh

# Definitions
#--------------------------------------------------
LOOP_DEV=/dev/loop10
LOOP_DIR=$MCU_HOME_DIR/tmp/loop10
SCRIPT=$MCU_HOME_DIR/mcms/Scripts/SoftFirmwareCheck.sh
SAFE_UPGRADE_SCRIPT=$MCU_HOME_DIR/mcms/Scripts/SoftSafeUpgrade.sh
NEW_VERSION_TXT=$MCU_HOME_DIR/mcms/Scripts/SoftSafeUpgradeSrcVersions.txt
CURRENT_TXT=$MCU_HOME_DIR/mcms/Scripts/SoftSafeUpgradeDstVersions.txt
SAFE_UPGRADE_LOG=$MCU_HOME_DIR/tmp/startup_logs/safeUpgrade.log

IGNORE_SAFE_UPGRADE=$MCU_HOME_DIR/tmp/ignore_safe_upgrade
CFG_KEY=`cat $MCU_HOME_DIR/mcms/Cfg/SystemCfgUser.xml|grep -A 1 ENFORCE_SAFE_UPGRADE|grep DATA`

if [ "$1" != "" ]
  then
  SAFE_UPGRADE_LOG=$1
fi

 PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`
 CARDS_TYPE=`cat $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt`

 NEW_VERSION=$MCU_HOME_DIR/data/new_version/new_version.bin
 NEW_VERSION_NUMBER=`tail -n 99 $NEW_VERSION | grep FirmwareVersion | awk -F ' ' '{ print $2 }' | awk -F '_' '{ print $2 }'`



 CURRENT_BIN=$(readlink $MCU_HOME_DIR/data/current)
 CURRENT_VERSION=$MCU_HOME_DIR/data/$CURRENT_BIN
 CURRENT_VERSION_NUMBER=`tail -n 99 $CURRENT_VERSION | grep FirmwareVersion | awk -F ' ' '{ print $2 }' | awk -F '_' '{ print $2 }'`


echo "NEW_VERSION_NUMBER " $NEW_VERSION_NUMBER >> $SAFE_UPGRADE_LOG
echo "CURRENT_VERSION_NUMBER " $CURRENT_VERSION_NUMBER >> $SAFE_UPGRADE_LOG

 

#KSIZE=`tail -n 99 $NEW_VERSION | grep KernelSize | awk -F ' ' '{ print $2 }'`
#FSSIZE=`tail -n 99 $NEW_VERSION | grep SquashFsSize | awk -F ' ' '{ print $2 }'`

#/sbin/losetup -o $KSIZE $LOOP_DEV $NEW_VERSION
#mkdir -p $LOOP_DIR
#/bin/mount -t squashfs $LOOP_DEV $LOOP_DIR -o ro

#*************************************************************************************************************
#
# first we want to check if current version can upgrade to new version .
# we check it by passing over SafeUpgradeDstVersions.txt
#
#*************************************************************************************************************

# Check if file $MCU_HOME_DIR/tmp/ignore_safe_upgrade exists
if [ -f $IGNORE_SAFE_UPGRADE ];
	then
	echo "file exist ignore safe upgrade flow" >> $SAFE_UPGRADE_LOG
elif [ "$CFG_KEY" == "<DATA>NO</DATA>" ]
   then
	echo "CFG_KEY_ENFORCE_SAFE_UPGRAD value is NO" >> $SAFE_UPGRADE_LOG
else
	
  # Check if Script exists
  if [ -f $SAFE_UPGRADE_SCRIPT ];
	then
	 if [ -f $CURRENT_TXT ];
	  then
       $SAFE_UPGRADE_SCRIPT $CURRENT_TXT $NEW_VERSION_NUMBER $CURRENT_VERSION_NUMBER $NEW_VERSION_NUMBER $PRODUCT_TYPE $CARDS_TYPE $1
       SAFE_UPGRADE_SCRIPT_RESULT=$?
       if [ "$SAFE_UPGRADE_SCRIPT_RESULT" == "1" ] 
        then
         echo -n NO
         echo "current script found match in Black List" >> $SAFE_UPGRADE_LOG
         MESSAGE_STRING="Upgrade rejected. Upgrading from "$CURRENT_VERSION_NUMBER" to "$NEW_VERSION_NUMBER" is not supported, For a list of valid upgrades and downgrades, refer to RMX documentation"
         $MCU_HOME_DIR/mcms/Bin/McuCmd add_asserts Installer 1 "$MESSAGE_STRING" >> $SAFE_UPGRADE_LOG
         echo $MESSAGE_STRING >> $SAFE_UPGRADE_LOG
         exit 0
       fi 
        if [ "$SAFE_UPGRADE_SCRIPT_RESULT" == "2" ] 
        then
         echo -n YES
         echo "current script found match in White List or it is a downgrade case " >> $SAFE_UPGRADE_LOG
         exit 0
        fi 
        if [ "$SAFE_UPGRADE_SCRIPT_RESULT" == "3" ] 
        then
         echo "current script found NO match in Black List nor white list " >> $SAFE_UPGRADE_LOG
        fi 
     else
          echo "SoftRunFirmwareCheck script : txt file not exist" $CURRENT_TXT >> $SAFE_UPGRADE_LOG
    fi
    else
     echo "SoftRunFirmwareCheck script : safe upgrade script not exist" >> $SAFE_UPGRADE_LOG
fi
fi



  
if [ -f $LOOP_DIR$SCRIPT ];
	then 
     RESULT=`$LOOP_DIR$SCRIPT $1`
     echo -n $RESULT 
  else
     echo -n YES      
fi





	



