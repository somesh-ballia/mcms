#!/bin/bash
# GesherMcmsStart.sh

####################################################################
SELF_SIGNED_LOG_DIR=$MCU_HOME_DIR/tmp/startup_logs/self_signed_log.log
STARTUP_LOG=$MCU_HOME_DIR/tmp/startup_logs/start_sh.log
echo uptime | tee $STARTUP_LOG
uptime	| tee -a $STARTUP_LOG
echo "TICKS: " | tee -a $STARTUP_LOG
cat /proc/uptime | tee -a $STARTUP_LOG

####################################################################
export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin
export SASL_PATH=$MCU_HOME_DIR/mcms/Bin
export LICENSE=$(cat $MCU_HOME_DIR/mcms/JITC_MODE.txt)

# Defines what is partition 3 of CF
export PART3=""
ls /dev/sda3 && export PART3=/dev/sda3
if [ "$PART3" == "" ]
then
  echo "Critical error: there is no partition 3"  | tee -a $STARTUP_LOG
  exit 1
fi
echo "Partition three is $PART3"  | tee -a $STARTUP_LOG

OUTPUT_FOLDERS='apache core firmware cdr rec cslogs cslogs/cs1 cslogs/cs2
                cslogs/cs3 cslogs/cs4 cslogs/cs5 cslogs/cs6 cslogs/cs7 cslogs/cs8 faults tmp IVR mylogs media_rec audit
                media nids links tcp_dump tcp_dump/mcms log backup'

ulimit -c unlimited
ulimit -s 1024

echo -n $LICENSE > $MCU_HOME_DIR/tmp/JITC_MODE.txt

####################################################################
cd $MCU_HOME_DIR/mcms
sudo Scripts/Cleanup.sh
sudo /bin/chown mcms:mcms $MCU_HOME_DIR/tmp/802_1xCtrl
chmod 777 $MCU_HOME_DIR/tmp/802_1xCtrl
#sudo Scripts/BinLinks.sh

sudo rm -fR $MCU_HOME_DIR/tmp/queue
sudo rm -fR $MCU_HOME_DIR/tmp/shared_memory
sudo rm -fR $MCU_HOME_DIR/tmp/semaphore
mkdir $MCU_HOME_DIR/tmp/queue
mkdir $MCU_HOME_DIR/tmp/shared_memory
mkdir $MCU_HOME_DIR/tmp/semaphore
mkfifo $MCU_HOME_DIR/tmp/queue/LoggerPipe
chmod a+w $MCU_HOME_DIR/tmp/queue/LoggerPipe


sudo rm -f $MCU_HOME_DIR/output/apache/apache.log
sudo rm -f $MCU_HOME_DIR/tmp/httpd.conf*
sudo rm -f $MCU_HOME_DIR/tmp/ssl.conf*

mkdir -p $MCU_HOME_DIR/config/states
mkdir -p $MCU_HOME_DIR/config/ocs
mkdir -p $MCU_HOME_DIR/config/ocs/cs1/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs2/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs3/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs4/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs5/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs6/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs7/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs8/keys
mkdir -p $MCU_HOME_DIR/config/static_states
mkdir -p $MCU_HOME_DIR/config/keys
mkdir -p $MCU_HOME_DIR/config/keys/cs
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs1
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs2
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs3
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs4
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs5
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs6
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs7
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs8
mkdir -p $MCU_HOME_DIR/config/keys/ca_cert
mkdir -p $MCU_HOME_DIR/config/keys/ca_cert/crl

chmod a+w $MCU_HOME_DIR/config/states
chmod a+w $MCU_HOME_DIR/config/ocs
chmod a+w $MCU_HOME_DIR/config/static_states
chown mcms:mcms $MCU_HOME_DIR/config/keys
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs1
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs2
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs3
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs4
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs5
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs6
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs7
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs8
chown mcms:mcms $MCU_HOME_DIR/config/keys/ca_cert
chown mcms:mcms $MCU_HOME_DIR/config/keys/ca_cert/crl

sudo chown mcms:mcms $MCU_HOME_DIR/config/JITC_MODE.txt
sudo chown mcms:mcms $MCU_HOME_DIR/config/JITC_MODE_FIRST_RUN.txt
sudo chown mcms:mcms $MCU_HOME_DIR/config/SEPARATE_MANAGMENT_NETWORK.txt
sudo chown mcms:mcms $MCU_HOME_DIR/config/product

#incase of upgrade change file to new name
for i in 1 2 3 4 5 6 7 8

do

  filename=$MCU_HOME_DIR/cs/ocs/cs$i/keys/certPassword.txt
  if [ -f $filename ]
  then  
    newFileName=$MCU_HOME_DIR/cs/ocs/cs$i/keys/certPassword.txt.orig	
    sudo mv $filename $newFileName
  fi
  
  cs_destination=$MCU_HOME_DIR/cs/ocs/cs$i/keys
  cs_source=$MCU_HOME_DIR/config/keys/cs/cs$i/*
  
  cp -Rf $cs_source $cs_destination
   
done

cp -f $MCU_HOME_DIR/mcms/Versions.xml $MCU_HOME_DIR/tmp

cp $MCU_HOME_DIR/mcms/Scripts/TLS/passphrase.sh     $MCU_HOME_DIR/tmp
cp $MCU_HOME_DIR/mcms/Scripts/TLS/passphraseplcm.sh $MCU_HOME_DIR/tmp
chown mcms:mcms $MCU_HOME_DIR/tmp/passphrase.sh
chown mcms:mcms $MCU_HOME_DIR/tmp/passphraseplcm.sh

sudo $MCU_HOME_DIR/mcms/Scripts/GesherLightSrv.sh stop
sudo $MCU_HOME_DIR/mcms/Scripts/GesherLightSrv.sh start

led_singlemode()
{
	echo "Led: Single Mode !!!"
	sudo $MCU_HOME_DIR/mcms/Bin/LightCli HotStandby single_mode &> /dev/null	
	
}

led_singlemode

####################################################################
DATA_SIZE=$(df $MCU_HOME_DIR/data | grep $MCU_HOME_DIR/data | awk '{print $2}')
if [ $DATA_SIZE -lt 1000000 ]
then
  FACTORY=`readlink $MCU_HOME_DIR/data/factory | grep factory`
  if [ "$FACTORY" ]
  then
    echo "removing factory version to HardDisk"| tee -a $STARTUP_LOG
    sudo mount -o remount,rw,async,noexec $PART3 $MCU_HOME_DIR/data
    sudo mv -f $MCU_HOME_DIR/data/$FACTORY $MCU_HOME_DIR/output/tmp
    sudo rm -f $MCU_HOME_DIR/data/factory
    sudo ln -sf $MCU_HOME_DIR/data/removed $MCU_HOME_DIR/data/factory
    sudo sync
    sudo mount -o remount,ro,async,noexec $PART3 $MCU_HOME_DIR/data
  fi
fi

#fix for systems that was downgraded from 5.0 before VNGR-12242
if [ -e $MCU_HOME_DIR/data/factory ]
then
  echo ""
else
  sudo mount -o remount,rw,async,noexec $PART3 $MCU_HOME_DIR/data
  ln -sf $MCU_HOME_DIR/data/removed $MCU_HOME_DIR/data/factory
  sync
  sudo mount -o remount,ro,async,noexec $PART3 $MCU_HOME_DIR/data
fi

########################################################################
if [ "$CLEAN_AUDIT_FILES" != "NO" ]
then
  echo "Removing Audit directory"  | tee -a $STARTUP_LOG
  sudo rm -Rf Audit
fi

if [ "$CLEAN_LOG_FILES" != "NO" ]
then
  echo "Removing LogFiles directory" | tee -a $STARTUP_LOG
  sudo rm -Rf LogFiles
fi

if [ "$CLEAN_FAULTS" != "NO" ]
then
  echo "Cleaning Faults directory" | tee -a $STARTUP_LOG
  sudo rm -Rf Faults/*
fi

if [ "$CLEAN_CDR" != "NO" ]
then
  echo "Cleaning CdrFiles directory" | tee -a $STARTUP_LOG
  sudo rm -Rf CdrFiles
fi

if [ "$CLEAN_STATES" != "NO" ]
then
  echo "Cleaning States directory" | tee -a $STARTUP_LOG
  sudo rm -Rf States/*
fi

if [ "$CLEAN_MEDIA_RECORDING" != "NO" ]
then
  echo "Cleaning MediaRecordings directory" | tee -a $STARTUP_LOG
  sudo rm -Rf MediaRecording/share/*
fi

if [ "$LICENSE_FILE" == "" ]
then
  LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_180_YES_MultipleServices_YES_v100.0.cfs"
fi

if [ "$NINJA" == "YES" ]
then
  SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmRx.txt"
fi

if [ "$CLEAN_CFG" != "NO" ]
then
  echo "CLEAN_CFG" | tee -a $STARTUP_LOG
  echo "Removing Cfg directory" | tee -a $STARTUP_LOG
  sudo rm -Rf Cfg
  mkdir Cfg
  mkdir Cfg/IVR
  mkdir Cfg/AudibleAlarms
  cp -R VersionCfg/AudibleAlarms/* Cfg/AudibleAlarms/
  chmod -R +w Cfg/AudibleAlarms/*

  if [ "$USE_DEFAULT_IVR_SERVICE" != "NO" ]
  then
    echo "Copy Default IVR service" | tee -a $STARTUP_LOG
    cp -R VersionCfg/IVR/* Cfg/IVR/
    chmod -R +w Cfg/IVR/*
  fi

  if [ "$USE_DEFAULT_PSTN_SERVICE" != "NO" ]
  then
    echo "Copy Default PSTN service" | tee -a $STARTUP_LOG
    cp VersionCfg/RtmIsdnServiceList.xml Cfg/
    cp VersionCfg/RtmIsdnSpanMapList.xml Cfg/
    #cp VersionCfg/EthernetSettings.xml Cfg/
    chmod +w Cfg/RtmIsdnServiceList.xml
    chmod +w Cfg/RtmIsdnSpanMapList.xml
    #chmod +w Cfg/EthernetSettings.xml
  fi
    
  if [ "$USE_DEFAULT_IP_SERVICE" != "NO" ]
  then
    if [ "$USE_ALT_IP_SERVICE" != "" ]
    then
      echo "Copy $USE_ALT_IP_SERVICE" | tee -a $STARTUP_LOG
      cp $USE_ALT_IP_SERVICE Cfg/IPServiceList.xml
      chmod +w Cfg/IPServiceList.xml
    else
      if [[ "$SOFT_MCU" == "YES" || "$SOFT_MCU_MFW" == "YES" || "$SOFT_MCU_EDGE" == "YES" ]]
      then
        echo "SOFTMCU - Nothing to do. File creation is in the code. " | tee -a $STARTUP_LOG
      else
        echo "Copy Default IP: VersionCfg/DefaultIPServiceList.xml" | tee -a $STARTUP_LOG
        cp VersionCfg/DefaultIPServiceList.xml Cfg/IPServiceList.xml
        chmod +w Cfg/IPServiceList.xml
      fi
    fi
  fi
	    
  if [ "$USE_DEFAULT_OPERATOR_DB" != "NO" ]
  then
    echo "Copy Default operator DB" | tee -a $STARTUP_LOG
    cp VersionCfg/OperatorDB.xml Cfg/OperatorDB.xml
    chmod +w Cfg/OperatorDB.xml
  fi
  
  if [ "$USE_DEFUALT_NETWORK_MANAGMENT" != "NO" ]
  then
    echo "Copy NetworkCfg_Management.xml" | tee -a $STARTUP_LOG
    cp VersionCfg/NetworkCfg_Management.xml Cfg/NetworkCfg_Management.xml
    chmod +w Cfg/NetworkCfg_Management.xml
  fi
  
  if [ "$SYSTEM_CFG_USER_FILE" != "" ]
  then
    echo "Copy system cfg user file" | tee -a $STARTUP_LOG
    cp $SYSTEM_CFG_USER_FILE Cfg/SystemCfgUser.xml
    chmod +w Cfg/SystemCfgUser.xml
  fi
  
  if [ "$SYSTEM_CFG_DEBUG_FILE" != "" ]
  then
    echo "Copy system cfg debug file" | tee -a $STARTUP_LOG
    cp $SYSTEM_CFG_FILE Cfg/SystemCfgDebug.xml
    chmod +w Cfg/SystemCfgDebug.xml
  fi

  if [ "$USE_DEFAULT_TIME_CONFIGURATION" != "NO" ]
  then
    echo "Copy Default Time configuration" | tee -a $STARTUP_LOG
    cp VersionCfg/SystemTime.xml Cfg/SystemTime.xml
    chmod +w Cfg/SystemTime.xml
  fi
   
  cp VersionCfg/MPL_SIM.XML Cfg/
  cp VersionCfg/EP_SIM.XML Cfg/
  cp VersionCfg/MEDIA_MNGR_CFG.XML Cfg/
  chmod +w Cfg/MPL_SIM.XML
  chmod +w Cfg/MEDIA_MNGR_CFG.XML    

  # System cards' mode
  sudo rm -Rf $MCU_HOME_DIR/mcms/StaticStates
  mkdir -p $MCU_HOME_DIR/mcms/StaticStates
  chmod a+w $MCU_HOME_DIR/mcms/StaticStates

  # Pizza's default: breeze
  cp VersionCfg/SystemCardsMode_breeze.txt $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt
  chmod a+w $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt
  if [ "$SYSTEM_CARDS_MODE_FILE" != "" ]
  then
    echo "Copy $SYSTEM_CARDS_MODE_FILE" | tee -a $STARTUP_LOG
    cp $SYSTEM_CARDS_MODE_FILE $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt
    chmod a+w $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt
  fi

  cp VersionCfg/Versions.xml Cfg/
   
  # Pizza's default: breeze
  cp VersionCfg/Resource_Setting_Default_Breeze.xml Cfg/Resource_Settings.xml
  chmod a+w Cfg/Resource_Settings.xml

  cp VersionCfg/License.cfs Cfg/
  if [ "$LICENSE_FILE" != "" ]
  then
    echo "Copy $LICENSE_FILE" | tee -a $STARTUP_LOG
    chmod +w Cfg/License.cfs
    cp $LICENSE_FILE Cfg/License.cfs
  fi

  cp VersionCfg/License.cfs Simulation/
  chmod +w Simulation/License.cfs
  if [ "$LICENSE_FILE" != "" ]
  then
    echo "Copy $LICENSE_FILE" | tee -a $STARTUP_LOG
    chmod +w Simulation/License.cfs
    cp $LICENSE_FILE Simulation/License.cfs
  fi
    
  if [ "$MPL_SIM_FILE" != "" ]
  then
     echo "Copy $MPL_SIM_FILE" | tee -a $STARTUP_LOG
     cp $MPL_SIM_FILE Cfg/MPL_SIM.XML
  fi
else
  if [[ "$SOFT_MCU" == "YES" || "$SOFT_MCU_MFW" == "YES" || "$GESHER" == "YES" || "$NINJA" == "YES" || "$SOFT_MCU_EDGE" == "YES" ]]
  then
    if [ -e Cfg/IVR ]
    then 
      echo "IVR folder already exist - nothing to do" | tee -a $STARTUP_LOG
    else
      mkdir -p Cfg/IVR
      echo "Copy Default IVR service"  | tee -a $STARTUP_LOG
      cp -R VersionCfg/IVR/* Cfg/IVR/
      chmod -R +w Cfg/IVR/*
    fi
  fi

  if [[ "$NINJA" == "YES" && "$SYSTEM_CARDS_MODE_FILE" != "" ]]
  then
    echo "Copy $SYSTEM_CARDS_MODE_FILE for NINJA" | tee -a $STARTUP_LOG
    cp $SYSTEM_CARDS_MODE_FILE $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt
    chmod a+w $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt
  fi
fi

if [ "RMX_IN_SECURE" != "" ]
then
  echo "Copy $MPL_SIM_FILE" | tee -a $STARTUP_LOG
  cp $MPL_SIM_FILE Cfg/MPL_SIM.XML	
fi

if [ "$RESOURCE_SETTING_FILE" != "" ]
then
  echo "Copy $RESOURCE_SETTING_FILE" | tee -a $STARTUP_LOG
  cp $RESOURCE_SETTING_FILE Cfg/Resource_Settings.xml
fi


sudo rm -Rf IVRX
sudo mkdir IVRX
sudo chown mcms:mcms IVRX
mkdir IVRX/RollCall
cp IVRX_Save/IVRX/RollCall/SimRollCall.aca IVRX/RollCall/SimRollCall.aca


##############################################################
echo "RMX_4000=$RMX_4000"  | tee -a $STARTUP_LOG
echo "RMX_1500=$RMX_1500" | tee -a $STARTUP_LOG
echo "RMX_1500Q=$RMX_1500Q" | tee -a $STARTUP_LOG
echo "SOFT_MCU=$SOFT_MCU" | tee -a $STARTUP_LOG
echo "SOFT_MCU_MFW=$SOFT_MCU_MFW" | tee -a $STARTUP_LOG
echo "GESHER=$GESHER" | tee -a $STARTUP_LOG
echo "NINJA=$NINJA" | tee -a $STARTUP_LOG
echo "SOFT_MCU_EDGE=$SOFT_MCU_EDGE" | tee -a $STARTUP_LOG
echo "CG=$CG" | tee -a $STARTUP_LOG
echo "CLEAN_CFG=$CLEAN_CFG" | tee -a $STARTUP_LOG
echo "USE_DEFAULT_IVR_SERVICE=$USE_DEFAULT_IVR_SERVICE" | tee -a $STARTUP_LOG
echo "USE_DEFAULT_PSTN_SERVICE=$USE_DEFAULT_PSTN_SERVICE" | tee -a $STARTUP_LOG
echo "USE_DEFAULT_IP_SERVICE=$USE_DEFAULT_IP_SERVICE" | tee -a $STARTUP_LOG
echo "USE_ALT_IP_SERVICE=$USE_ALT_IP_SERVICE" | tee -a $STARTUP_LOG
echo "USE_DEFAULT_OPERATOR_DB=$USE_DEFAULT_OPERATOR_DB" | tee -a $STARTUP_LOG
echo "USE_DEFUALT_NETWORK_MANAGMENT=$USE_DEFUALT_NETWORK_MANAGMENT" | tee -a $STARTUP_LOG
echo "SYSTEM_CFG_USER_FILE=$SYSTEM_CFG_USER_FILE" | tee -a $STARTUP_LOG
echo "SYSTEM_CFG_DEBUG_FILE=$SYSTEM_CFG_DEBUG_FILE" | tee -a $STARTUP_LOG
echo "USE_DEFAULT_TIME_CONFIGURATION=$USE_DEFAULT_TIME_CONFIGURATION" | tee -a $STARTUP_LOG
echo "SYSTEM_CARDS_MODE_FILE=$SYSTEM_CARDS_MODE_FILE" | tee -a $STARTUP_LOG
echo "LICENSE_FILE=$LICENSE_FILE" | tee -a $STARTUP_LOG
echo "MPL_SIM_FILE=$MPL_SIM_FILE" | tee -a $STARTUP_LOG
echo "RMX_IN_SECURE=$RMX_IN_SECURE" | tee -a $STARTUP_LOG
echo "RESOURCE_SETTING_FILE=$RESOURCE_SETTING_FILE" | tee -a $STARTUP_LOG
echo "JITC_MODE_CFG=$JITC_MODE_CFG" | tee -a $STARTUP_LOG
echo "JITC_MODE_CUR=$JITC_MODE_CUR" | tee -a $STARTUP_LOG
echo "PCMSIM=$PCMSIM" | tee -a $STARTUP_LOG

##############################################################
if [ "$GESHER" == "YES" ]
then
    echo -n GESHER > $MCU_HOME_DIR/mcms/ProductType
    echo -n GESHER > $MCU_HOME_DIR/tmp/EMAProductType.txt
    if [ "$RMX_IN_SECURE" == "YES" ]
	then
    	MPL_SIM_FILE="VersionCfg/MPL_SIM_SECURED_BREEZE.XML"
    fi
	if [ "$MPL_SIM_FILE" == "" ]
	then
    	    MPL_SIM_FILE="VersionCfg/MPL_SIM.XML"
	fi
    if [ "$LICENSE_FILE" == "" ]
	then
    	LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_180_YES_MultipleServices_YES_v100.0.cfs"
    fi
    if [ "$RESOURCE_SETTING_FILE" == "" ]
	then
    	RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_Default_Breeze.xml"
    fi 
else
  if [ "$NINJA" == "YES" ]
  then
    echo -n NINJA > $MCU_HOME_DIR/mcms/ProductType
    echo -n NINJA > $MCU_HOME_DIR/tmp/EMAProductType.txt
    if [ "$RMX_IN_SECURE" == "YES" ]
	then
    	MPL_SIM_FILE="VersionCfg/MPL_SIM_SECURED_BREEZE.XML"
    fi
	if [ "$MPL_SIM_FILE" == "" ]
	then
    	    MPL_SIM_FILE="VersionCfg/MPL_SIM.XML"
	fi
    if [ "$LICENSE_FILE" == "" ]
	then
    	LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_180_YES_MultipleServices_YES_v100.0.cfs"
    fi
    if [ "$RESOURCE_SETTING_FILE" == "" ]
	then
    	RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_Default_Breeze.xml"
    fi 
else
    echo -n GESHER > $MCU_HOME_DIR/mcms/ProductType
    echo -n GESHER > $MCU_HOME_DIR/tmp/EMAProductType.txt
    if [ "$RMX_IN_SECURE" == "YES" ]
	then
    	MPL_SIM_FILE="VersionCfg/MPL_SIM_SECURED_BREEZE.XML"
    fi
	if [ "$MPL_SIM_FILE" == "" ]
	then
    	MPL_SIM_FILE="VersionCfg/MPL_SIM.XML"
	fi
    if [ "$LICENSE_FILE" == "" ]
	then
    	LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_180_YES_MultipleServices_YES_v100.0.cfs"
    fi
    if [ "$RESOURCE_SETTING_FILE" == "" ]
	then
    	RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_Default_Breeze.xml"
    fi
fi
fi

####################################################################
cd $MCU_HOME_DIR/mcms 

echo "Reading time using NTP_Bypass_SoftMCU_Client" | tee -a $STARTUP_LOG 
COUNT=1
while [ $COUNT -gt 0 ]; do
  Bin/NTP_Bypass_SoftMCU_Client
  CLIENT_RES=$?
  echo "NTP_Bypass_SoftMCU_Client=" $CLIENT_RES | tee -a $STARTUP_LOG
  if [ $CLIENT_RES -gt 0 ] 
  then
    echo "Failed accessing NTP bypass server." | tee -a $STARTUP_LOG
    if [ $COUNT -gt 1 ]
    then
      sleep 2
    fi
    let COUNT=COUNT-1
  else
    let COUNT=0
  fi
done

echo "Test restore factory default flag" | tee -a $STARTUP_LOG
if [ -e $MCU_HOME_DIR/mcms/States/restore_factory_defaults.flg ]
then
  echo "Restore factory defaults" | tee -a $STARTUP_LOG;
  echo "NO" > $MCU_HOME_DIR/mcms/JITC_MODE.txt
  echo "NO" > $MCU_HOME_DIR/mcms/JITC_MODE_FIRST_RUN.txt
  # Removes some of Cfg files
  sudo mv $MCU_HOME_DIR/config/mcms/NetworkCfg_Management.xml $MCU_HOME_DIR/config/lost+found
  sudo rm -Rf $MCU_HOME_DIR/config/mcms/*
  sudo mv $MCU_HOME_DIR/config/lost+found/NetworkCfg_Management.xml $MCU_HOME_DIR/config/mcms/
  sudo rm -Rf $MCU_HOME_DIR/config/states/*
  /bin/touch $MCU_HOME_DIR/config/states/McmsRestoreFactoryFileInd.flg
  sudo rm -Rf $MCU_HOME_DIR/mcms/Links/*
  sudo rm -Rf $MCU_HOME_DIR/config/ema/*
  sudo rm -Rf $MCU_HOME_DIR/config/lost+found/*
  sudo rm -Rf cs
  sudo rm -Rf $MCU_HOME_DIR/mcms/StaticStates/MepMode.txt
  sudo rm -Rf $MCU_HOME_DIR/mcms/Keys/*
  # removing all folders from Hard Disk
  cd $MCU_HOME_DIR/output
  sudo rm -Rf $OUTPUT_FOLDERS
  # create all configuration folders
  if [ -e /etc/rc.d/20ConfigSys ] 
  then
    sudo /etc/rc.d/20ConfigSys
  else
    echo "ERROR: restore factory default in initial phase, no script file : /etc/rc.d/20ConfigSys"
  fi 
fi

cd $MCU_HOME_DIR/mcms

echo "Test restore config flag" | tee -a $STARTUP_LOG
if [ -e States/restore_config.flg ]
then
	echo "found restore flag and begin to restore" | tee -a ${STARTUP_LOG}
  sudo Scripts/RestoreConfig.sh States 2>&1 1>/dev/null;
  Scripts/Self_Signed_Cert.sh Create_managment_certificate 2>&1 | tee -a ${SELF_SIGNED_LOG_DIR}
	echo "finish to restore" | tee -a ${STARTUP_LOG}
fi

# Defines currect state of JITC flag: code copied from
# ./patches/post-compile-rootfs/etc/rc.d/07check_syscfg
if [ -e $MCU_HOME_DIR/mcms/JITC_MODE.txt ]
then
  export JITC_MODE_CUR=$(cat $MCU_HOME_DIR/mcms/JITC_MODE.txt)
else
  export JITC_MODE_CUR="NO"
fi
if [ -e $MCU_HOME_DIR/mcms/Cfg/SystemCfgUserTmp.xml ]
then
  export SYSCFG_FILE="$MCU_HOME_DIR/mcms/Cfg/SystemCfgUserTmp.xml"
else
  export SYSCFG_FILE="$MCU_HOME_DIR/mcms/Cfg/SystemCfgUser.xml"
fi

export JITC_MODE_CFG=$(cat $SYSCFG_FILE | grep -A 1 ULTRA_SECURE_MODE | grep DATA | sed 's/	//g')
if [[ "$JITC_MODE_CFG" && "$JITC_MODE_CFG" == "<DATA>YES</DATA>" ]]
then
  echo "YES" > $MCU_HOME_DIR/mcms/JITC_MODE.txt
  if [ "$JITC_MODE_CUR" == "NO" ]
  then
     echo "YES" > $MCU_HOME_DIR/mcms/JITC_MODE_FIRST_RUN.txt
  fi
else
  echo "NO" > $MCU_HOME_DIR/mcms/JITC_MODE.txt
fi

echo "Start copy IVR default files" | tee -a $STARTUP_LOG
if [ -e $MCU_HOME_DIR/mcms/Cfg/IVR ]
then 
  echo "CopyDefaultIVR_no_overwrite.sh" | tee -a $STARTUP_LOG
  Scripts/CopyDefaultIVR_no_overwrite.sh 2> /dev/null;
else
  echo "CopyDefaultIVR.sh" | tee -a $STARTUP_LOG
  Scripts/CopyDefaultIVR.sh 2> /dev/null;
fi

# remove the patch slides for AT&T
if [[ -d Cfg/IVR/Slides/ATT_Waiting_Room_Slide ]]; then
	rm -Rf Cfg/IVR/Slides/ATT_Waiting_Room_Slide
fi

Scripts/SetupCustomSlides.sh

echo "End copy IVR default files" | tee -a $STARTUP_LOG
chmod -R a+w $MCU_HOME_DIR/mcms/Cfg/IVR

Scripts/CopyAudibleAlarms.sh

mount -o remount,rw,async,noexec $PART3 $MCU_HOME_DIR/data
#rm -Rf $MCU_HOME_DIR/data/backup/*  
rm -Rf $MCU_HOME_DIR/data/restore/*
#rm -Rf $MCU_HOME_DIR/data/new_version/*
mount -o remount,ro,async,noexec $PART3 $MCU_HOME_DIR/data

cd $MCU_HOME_DIR/output
mkdir -p $OUTPUT_FOLDERS
MAKE_DIR_RESULT=$?
echo "mkdir OUTPUT_FOLDERS result " $MAKE_DIR_RESULT | tee -a $STARTUP_LOG
if [ "$MAKE_DIR_RESULT" != "0" ]
  then
   sleep 1
   sync
   mkdir -p $OUTPUT_FOLDERS
   MAKE_DIR2_RESULT=$?
   echo "mkdir OUTPUT_FOLDERS second try result " $MAKE_DIR2_RESULT | tee -a $STARTUP_LOG
fi
chown mcms:mcms $OUTPUT_FOLDERS
chmod a+w $OUTPUT_FOLDERS

cd $MCU_HOME_DIR/mcms

chown -R mcms:mcms $MCU_HOME_DIR/config/ema $MCU_HOME_DIR/config/mcms
touch $MCU_HOME_DIR/config/ema/InternalConfigSet_Customized.xml
chown mcms:mcms $MCU_HOME_DIR/config/ema/InternalConfigSet_Customized.xml

touch $MCU_HOME_DIR/config/ema/EMA.DataObjects.OfflineTemplates.AddressbookContent_.xml
chown mcms:mcms $MCU_HOME_DIR/config/ema/EMA.DataObjects.OfflineTemplates.AddressbookContent_.xml

touch $MCU_HOME_DIR/config/ema/EMA.DataObjects.OfflineTemplates.NoteListContent_.xml
chown mcms:mcms $MCU_HOME_DIR/config/ema/EMA.DataObjects.OfflineTemplates.NoteListContent_.xml

###################################################################
#if [ "$LICENSE" != "YES" ]
#then
#  echo "Running snmpd"  | tee -a $STARTUP_LOG
#  cp StaticCfg/snmpd.conf.sim $MCU_HOME_DIR/tmp/snmpd.conf
#  chmod a+w $MCU_HOME_DIR/tmp/snmpd.conf
#  /sbin/snmpd -c $MCU_HOME_DIR/tmp/snmpd.conf &
#fi

echo "check the snmp mode, fips or normal"|tee -a $STARTUP_LOG
SNMP_FIPS_MODE="NO"
SNMP_FIPS_MODE_CFG=$(cat $SYSCFG_FILE | grep -A 1 SNMP_FIPS_MODE | grep DATA)
if [ "$SNMP_FIPS_MODE_CFG" ];then
	if [ $SNMP_FIPS_MODE_CFG == "<DATA>YES</DATA>" ]; then
		SNMP_FIPS_MODE="YES"
	else
		if [ $SNMP_FIPS_MODE_CFG == "<DATA>NO</DATA>" ]; then
			SNMP_FIPS_MODE="NO"
		fi
	fi
fi
echo "SNMP_FIPS_MODE=$SNMP_FIPS_MODE" | tee -a $STARTUP_LOG
if [ $SNMP_FIPS_MODE == "YES" ]; then
	echo "fips shmp mode, create file $MCU_HOME_DIR/tmp/netsnmpfipsflag" |tee -a $STARTUP_LOG                            
	touch $MCU_HOME_DIR/tmp/netsnmpfipsflag 
fi

cp $MCU_HOME_DIR/mcms/StaticCfg/snmpd.conf.sim $MCU_HOME_DIR/tmp/snmpd.conf                          
chmod a+w $MCU_HOME_DIR/tmp/snmpd.conf
                                                                                     
echo "Removing Recording files" | tee -a $STARTUP_LOG
sudo rm -Rf $MCU_HOME_DIR/output/media_rec/*
mkdir -p $MCU_HOME_DIR/output/media_rec/share
chmod a+w $MCU_HOME_DIR/output/media_rec/share

# VNGR-17283 stig: NFS exported system files and system directories are not owned by root
chown mcms:mcms $MCU_HOME_DIR/mcms/Cfg/IVR $MCU_HOME_DIR/output/media_rec $MCU_HOME_DIR/output/core $MCU_HOME_DIR/mcms/TcpDumpEmb $MCU_HOME_DIR/output/backup

echo "TICKS: " | tee -a $STARTUP_LOG
cat /proc/uptime | tee -a $STARTUP_LOG
echo -n "Running McmsDaemon..."  | tee -a $STARTUP_LOG
MCMSD=Bin/McmsDaemon
MCMSD_LINK=$MCU_HOME_DIR/mcms/Links/McmsDaemon
if [ -e $MCMSD_LINK ]
then
  echo "(Using patched McmsDaemon)" | tee -a $STARTUP_LOG
  MCMSD=$MCMSD_LINK
fi

if [ -e $MCU_HOME_DIR/tmp/stop_monitor ]
then
	echo "stop monitor for debug" | tee -a $STARTUP_LOG
else
	# to make sure processes are running
	Process_Watcher=$(ps -ef | grep SoftMcuWatcher.sh | grep -v grep)
	if [ "" == "${Process_Watcher}" ]
	then
    	$MCU_HOME_DIR/mcms/Scripts/SoftMcuWatcher.sh &
	fi
fi
echo  "Running Self-signed certificate" | tee -a ${STARTUP_LOG}
Scripts/Self_Signed_Cert.sh boot 2>&1 | tee -a ${SELF_SIGNED_LOG_DIR}
sudo Scripts/Check_Bonding_Enabled.sh ${LICENSE} 2>&1 $STARTUP_LOG



echo "McmsDaemon starting ..." | tee -a $STARTUP_LOG
( 
  (sleep 2 ; $DIAGNOSTICS )& $MCMSD > /dev/null;
  echo "McmsDaemon finished" | tee -a $STARTUP_LOG;
  ./Scripts/Reset.sh;
  if [[ ! -e "$MCU_HOME_DIR/output/MCMSDEBUG" && ! -e "$MCU_HOME_DIR/tmp/MCMSDEBUG" ]] 
  then 
    sudo reboot 
  else
    echo "MCMSDEBUG mode enabled." | tee -a $STARTUP_LOG
    if [ -e "/etc/rc.d/99soft_mcu" ]
    then
        echo "stop mcu..." | tee -a $STARTUP_LOG
        sudo /etc/rc.d/99soft_mcu stop
    fi
  fi
)&

exportfs -a

exit 0
