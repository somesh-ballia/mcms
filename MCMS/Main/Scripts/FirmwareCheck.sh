#!/bin/sh

# Definitions
#--------------------------------------------------
LOOP_DEV=/dev/loop10
LOOP_DIR=$MCU_HOME_DIR/tmp/loop10
SAFE_UPGRADE_SCRIPT=$MCU_HOME_DIR/mcms/Scripts/SafeUpgrade.sh
SAFE_UPGRADE_LOG=$MCU_HOME_DIR/tmp/startup_logs/safeUpgrade.log
NEW_VERSION_TXT=$MCU_HOME_DIR/mcms/Scripts/SafeUpgradeSrcVersions.txt
IGNORE_SAFE_UPGRADE=$MCU_HOME_DIR/tmp/ignore_safe_upgrade
CFG_KEY=`cat $MCU_HOME_DIR/mcms/Cfg/SystemCfgUser.xml|grep -A 1 ENFORCE_SAFE_UPGRADE|grep DATA`



PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`
CARDS_TYPE=`cat $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt`

#--------------------------------------------------
# RMX1500 do nothing 
#--------------------------------------------------
if [  "$PRODUCT_TYPE" == "RMX1500" ];
	then
	  SPECIAL_PRODUCT_TYPE=`cat $MCU_HOME_DIR/tmp/SpecialProductType`
	  if [  "$SPECIAL_PRODUCT_TYPE" == "RMX1500Q" ];
	    then
	      PRODUCT_TYPE=$SPECIAL_PRODUCT_TYPE
	  else
	     echo -n YES
	     exit 0
	  fi
fi

# Definitions
#--------------------------------------------------
	
echo "Start safe upgrade flow in firmwareCheck.sh" >> $SAFE_UPGRADE_LOG

 

 NEW_VERSION=$MCU_HOME_DIR/data/new_version/new_version.bin
 NEW_VERSION_NUMBER=`tail -n 99 $NEW_VERSION | grep FirmwareVersion | awk -F ' ' '{ print $2 }' | awk -F '_' '{ print $2 }'`



 CURRENT_BIN=$(readlink $MCU_HOME_DIR/data/current)
 CURRENT_VERSION=$MCU_HOME_DIR/data/$CURRENT_BIN
 CURRENT_VERSION_NUMBER=`tail -n 99 $CURRENT_VERSION | grep FirmwareVersion | awk -F ' ' '{ print $2 }' | awk -F '_' '{ print $2 }'`



 echo "NEW_VERSION_NUMBER " $NEW_VERSION_NUMBER >> $SAFE_UPGRADE_LOG
 echo "CURRENT_VERSION_NUMBER " $CURRENT_VERSION_NUMBER >> $SAFE_UPGRADE_LOG

echo "It is a fix for bug VNGFE 4269" >> $SAFE_UPGRADE_LOG

version_to_check="7.1.0" 
cut_version=`echo $CURRENT_VERSION_NUMBER | awk -F '.' '{print $1"."$2"."$3}'`
echo "cut_version" $cut_version >> $SAFE_UPGRADE_LOG
if [ "$cut_version" == "$version_to_check" ];
then
  echo " Current Version is 7.1.0 to fix bug VNGFE 4269 we need to remove the ipserviceList.xml file "  >> $SAFE_UPGRADE_LOG
  echo " the file will be removed only if MS==NO and just one service and file ipMultipleServiceList.xml exist "  >> $SAFE_UPGRADE_LOG
  
  CFG_KEY=`cat $MCU_HOME_DIR/mcms/Cfg/SystemCfgUser.xml|grep -A 1 MULTIPLE_SERVICES|grep DATA`
  
  if [ $CFG_KEY == "<DATA>NO</DATA>" ]
   then
	echo "CFG_KEY_MULTIPLE_SERVICES value is NO" >> $SAFE_UPGRADE_LOG
	NUM_OF_SERVICES=`cat $MCU_HOME_DIR/mcms/Cfg/IPMultipleServicesList.xml|grep -c  "<IP_SERVICE>"`
	if [ $NUM_OF_SERVICES == "1" ]
	 then
	    echo "There is only one service remove files IPServiceList.xml and IPServiceListTmp.xml" >> $SAFE_UPGRADE_LOG
	    rm -f $MCU_HOME_DIR/mcms/Cfg/IPServiceList.xml
	    rm -f $MCU_HOME_DIR/mcms/Cfg/IPServiceListTmp.xml
	fi
	
  fi

fi



#echo "SHA256 MIGRATION when upgrade from 7.5 7.6 7.7 to 7.8 and up user/password file should be removed" >> $SAFE_UPGRADE_LOG

#version_to_check1="7.5" 
#version_to_check2="7.6"
#version_to_check3="7.7"
#cut_version=`echo $CURRENT_VERSION_NUMBER | awk -F '.' '{print $1"."$2}'`
#echo "cut_version" $cut_version >> $SAFE_UPGRADE_LOG
#if [ "$cut_version" == "$version_to_check1" ];
#then
#  echo " Current Version is 7.5  we need to remove the EncOperatorDB.xml file "  >> $SAFE_UPGRADE_LOG
  
#  MESSAGE_STRING="RMX user/password list will be reset."
#  $MCU_HOME_DIR/mcms/Bin/McuCmd add_asserts Installer 1 "$MESSAGE_STRING"
#  
#  rm -f $MCU_HOME_DIR/mcms/Cfg/EncOperatorDB.xml
#  
#elif [ "$cut_version" == "$version_to_check2" ];
#  then
#    echo " Current Version is 7.6  we need to remove the EncOperatorDB.xml file "  >> $SAFE_UPGRADE_LOG  
#    MESSAGE_STRING="RMX user/password list will be reset."
#    $MCU_HOME_DIR/mcms/Bin/McuCmd add_asserts Installer 1 "$MESSAGE_STRING"
#    
#    rm -f $MCU_HOME_DIR/mcms/Cfg/EncOperatorDB.xml
#
#elif [ "$cut_version" == "$version_to_check3" ];
#  then
#    echo " Current Version is 7.7  we need to remove the EncOperatorDB.xml file "  >> $SAFE_UPGRADE_LOG
#  
#    MESSAGE_STRING="RMX user/password list will be reset."
#    $MCU_HOME_DIR/mcms/Bin/McuCmd add_asserts Installer 1 "$MESSAGE_STRING"
#    
#    rm -f $MCU_HOME_DIR/mcms/Cfg/EncOperatorDB.xml
#
#
#
#fi


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
elif [ $CFG_KEY == "<DATA>NO</DATA>" ]
   then
	echo "CFG_KEY_ENFORCE_SAFE_UPGRAD value is NO" >> $SAFE_UPGRADE_LOG
else
	

 # Check if Script exists
 if [ -f $LOOP_DIR$SAFE_UPGRADE_SCRIPT ];
	then

   # Check if txt file exists
   if [ -f $LOOP_DIR$NEW_VERSION_TXT ];
	then
     $LOOP_DIR$SAFE_UPGRADE_SCRIPT $LOOP_DIR$NEW_VERSION_TXT $CURRENT_VERSION_NUMBER $CURRENT_VERSION_NUMBER $NEW_VERSION_NUMBER $PRODUCT_TYPE $CARDS_TYPE
     SAFE_UPGRADE_SCRIPT_RESULT=$?
     echo "firmwareCheck.sh:SafeUpgrade script return value " $SAFE_UPGRADE_SCRIPT_RESULT >> $SAFE_UPGRADE_LOG
     if [ "$SAFE_UPGRADE_SCRIPT_RESULT" == "1" ] 
      then
        MESSAGE_STRING="RMX Software version change from "$CURRENT_VERSION_NUMBER" to "$NEW_VERSION_NUMBER" is not supported, For a list of valid upgrades and downgrades, refer to RMX documentation"
        $MCU_HOME_DIR/mcms/Bin/McuCmd add_asserts Installer 1 "$MESSAGE_STRING"
        echo "current script found match in Black List OR version is not in white list check SafeUpgradeDstVersions.txt" >> $SAFE_UPGRADE_LOG
        echo $MESSAGE_STRING >> $SAFE_UPGRADE_LOG
        echo "-n NO"
        exit 0
        elif [ "$SAFE_UPGRADE_SCRIPT_RESULT" == "3" ]
        then
               MESSAGE_STRING="RMX Software version change from "$CURRENT_VERSION_NUMBER" to "$NEW_VERSION_NUMBER" is not supported, For a list of valid upgrades and downgrades, refer to RMX documentation"
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


#--------------------------------------------------
#Check card type is supported
#--------------------------------------------------
CARDS_TYPE=`cat $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt`
if [ $CARDS_TYPE == "CARDS_MODE_MPM" ]
	then
	$MCU_HOME_DIR/mcms/Bin/McuCmd add_asserts Installer 1 "Upgrade aborted because the installed MPM cards are not supported with the version you are trying to install. The system is fully functional with the current version."
	echo -n NO
	exit 0 
fi

echo "-n YES"




