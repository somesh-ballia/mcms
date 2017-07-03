#!/bin/sh

# Definitions
#--------------------------------------------------
LOOP_DEV=/dev/loop10
LOOP_DIR=$MCU_HOME_DIR/tmp/loop10
SAFE_UPGRADE_SCRIPT=$MCU_HOME_DIR/mcms/Scripts/SoftSafeUpgrade.sh
SAFE_UPGRADE_LOG=$MCU_HOME_DIR/tmp/startup_logs/safeUpgrade.log
NEW_VERSION_TXT=$MCU_HOME_DIR/mcms/Scripts/SoftSafeUpgradeSrcVersions.txt
IGNORE_SAFE_UPGRADE=$MCU_HOME_DIR/tmp/ignore_safe_upgrade
CFG_KEY=`cat $MCU_HOME_DIR/mcms/Cfg/SystemCfgUser.xml|grep -A 1 ENFORCE_SAFE_UPGRADE|grep DATA`

if [ "$1" != "" ]
  then
  SAFE_UPGRADE_LOG=$1
fi

PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`
CARDS_TYPE=`cat $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt`

# Definitions
#--------------------------------------------------
	
echo "Start safe upgrade flow in SoftFirmwareCheck.sh" >> $SAFE_UPGRADE_LOG

 

 NEW_VERSION=$MCU_HOME_DIR/data/new_version/new_version.bin
 NEW_VERSION_NUMBER=`tail -n 99 $NEW_VERSION | grep FirmwareVersion | awk -F ' ' '{ print $2 }' | awk -F '_' '{ print $2 }'`



 CURRENT_BIN=$(readlink $MCU_HOME_DIR/data/current)
 CURRENT_VERSION=$MCU_HOME_DIR/data/$CURRENT_BIN
 CURRENT_VERSION_NUMBER=`tail -n 99 $CURRENT_VERSION | grep FirmwareVersion | awk -F ' ' '{ print $2 }' | awk -F '_' '{ print $2 }'`



 echo "NEW_VERSION_NUMBER " $NEW_VERSION_NUMBER >> $SAFE_UPGRADE_LOG
 echo "CURRENT_VERSION_NUMBER " $CURRENT_VERSION_NUMBER >> $SAFE_UPGRADE_LOG


#*************************************************************************************************************
# now we run the script on the new version side.
# we want to check if current version can upgrade to new version .
# by passing over SafeUpgradeSrcVersions.txt
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
 if [ -f $LOOP_DIR$SAFE_UPGRADE_SCRIPT ];
	then

   # Check if txt file exists
   if [ -f $LOOP_DIR$NEW_VERSION_TXT ];
	then
     $LOOP_DIR$SAFE_UPGRADE_SCRIPT $LOOP_DIR$NEW_VERSION_TXT $CURRENT_VERSION_NUMBER $CURRENT_VERSION_NUMBER $NEW_VERSION_NUMBER $PRODUCT_TYPE $CARDS_TYPE $1
     SAFE_UPGRADE_SCRIPT_RESULT=$?
     echo "SoftFirmwareCheck.sh:SafeUpgrade script return value " $SAFE_UPGRADE_SCRIPT_RESULT >> $SAFE_UPGRADE_LOG
     if [ "$SAFE_UPGRADE_SCRIPT_RESULT" == "1" ] 
      then
        MESSAGE_STRING="Upgrade rejected. Upgrading from "$CURRENT_VERSION_NUMBER" to "$NEW_VERSION_NUMBER" is not supported, For a list of valid upgrades and downgrades, refer to RMX documentation"
        $MCU_HOME_DIR/mcms/Bin/McuCmd add_asserts Installer 1 "$MESSAGE_STRING"
        echo "current script found match in Black List OR version is not in white list check SoftSafeUpgradeDstVersions.txt" >> $SAFE_UPGRADE_LOG
        echo $MESSAGE_STRING >> $SAFE_UPGRADE_LOG
        echo "-n NO"
        exit 0
        elif [ "$SAFE_UPGRADE_SCRIPT_RESULT" == "3" ]
        then
               MESSAGE_STRING="Upgrade rejected. Upgrading from "$CURRENT_VERSION_NUMBER" to "$NEW_VERSION_NUMBER" is not supported, For a list of valid upgrades and downgrades, refer to RMX documentation"
              $MCU_HOME_DIR/mcms/Bin/McuCmd add_asserts Installer 1 "$MESSAGE_STRING"
              echo "version is not in White list nor Black List in SafeUpgradeSrcVersions.txt " >> $SAFE_UPGRADE_LOG
        	  echo $MESSAGE_STRING >> $SAFE_UPGRADE_LOG
        	  echo "-n NO"
              exit 0
      elif [ "$SAFE_UPGRADE_SCRIPT_RESULT" == "2" ]
        then 
          echo "FirmwareCheck script : found match in white list or it is a case of downgrade" >> $SAFE_UPGRADE_LOG
         # do not exit cause we need to check the old firmware check 
      else 
        echo "FirmwareCheck script : wrong exit number" >> $SAFE_UPGRADE_LOG        
        echo "-n NO" 
        exit 0
      fi
    else
      echo "FirmwareCheck script : txt file not exist",$NEW_VERSION_TXT >> $SAFE_UPGRADE_LOG
  fi
  else 
    echo "FirmwareCheck script : safe upgrade script not exist" >> $SAFE_UPGRADE_LOG
fi
fi

echo -n YES




