#!/bin/sh
# Start.sh

export MCU_HOME_DIR=

STARTUP_LOG=$MCU_HOME_DIR/tmp/startup_logs/start_sh.log
echo uptime | tee $STARTUP_LOG
uptime	| tee -a $STARTUP_LOG
echo "TICKS: " | tee -a $STARTUP_LOG
cat /proc/uptime | tee -a $STARTUP_LOG

export PRODUCT=$(cat $MCU_HOME_DIR/mcms/ProductType)
export SASL_PATH=$MCU_HOME_DIR/usr/lib

# Defines what is partition 3 of CF
export PART3=""
ls /dev/hda3 && export PART3=/dev/hda3 
ls /dev/sdb3 && export PART3=/dev/sdb3
ls /dev/sda3 && export PART3=/dev/sda3

if [ "$PART3" == "" ]
then
  echo "Critical error: there is no partition 3 on CF"
  exit 1
fi

echo "Partition three is $PART3"

if [ "$PRODUCT" == 'CALL_GENERATOR' ]
then
  mount $MCU_HOME_DIR/output
fi

export LICENSE=$(cat $MCU_HOME_DIR/mcms/JITC_MODE.txt)

rm $MCU_HOME_DIR/output/apache/apache.log
mkfifo $MCU_HOME_DIR/tmp/queue/LoggerPipe
chmod a+w $MCU_HOME_DIR/tmp $MCU_HOME_DIR/tmp/queue/LoggerPipe

mkfifo $MCU_HOME_DIR/tmp/queue/SystemMonitoringPipe
chmod a+w $MCU_HOME_DIR/tmp $MCU_HOME_DIR/tmp/queue/SystemMonitoringPipe


mkdir -p $MCU_HOME_DIR/config/states
chmod a+w $MCU_HOME_DIR/config/states
mkdir -p $MCU_HOME_DIR/config/ocs
chmod a+w $MCU_HOME_DIR/config/ocs
mkdir -p $MCU_HOME_DIR/config/ocs/cs1/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs2/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs3/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs4/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs5/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs6/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs7/keys
mkdir -p $MCU_HOME_DIR/config/ocs/cs8/keys

# create $MCU_HOME_DIR/data/backup and $MCU_HOME_DIR/data/restore folders if needed
#if [ -e $MCU_HOME_DIR/data/backup  ]
#then
#  echo "backup folder exists"
#else
#  mount -o remount,rw,async,noexec $PART3 $MCU_HOME_DIR/data
#  mkdir -p $MCU_HOME_DIR/data/backup
#  chmod 0777 $MCU_HOME_DIR/data/backup
#  mkdir -p $MCU_HOME_DIR/data/restore
#  chmod 0777 $MCU_HOME_DIR/data/restore
#  mount -o remount,ro,async,noexec $PART3 $MCU_HOME_DIR/data
#fi

if [ -e $MCU_HOME_DIR/data/restore  ]
then
  echo "restore folder exists"
else
  mount -o remount,rw,async,noexec $PART3 $MCU_HOME_DIR/data
  mkdir -p $MCU_HOME_DIR/data/restore
  chmod 0776 $MCU_HOME_DIR/data/restore
  mount -o remount,ro,async,noexec $PART3 $MCU_HOME_DIR/data
fi

DATA_SIZE=`df $MCU_HOME_DIR/data | grep $MCU_HOME_DIR/data | awk '{print $2}'`

if [ $DATA_SIZE -lt 1000000 ]
then
  FACTORY=`readlink $MCU_HOME_DIR/data/factory | grep factory`
  if [ "$FACTORY" ]
  then
    echo "removing factory version to HD"| tee -a $STARTUP_LOG
    mount -o remount,rw,async,noexec $PART3 $MCU_HOME_DIR/data
    mv -f $MCU_HOME_DIR/data/$FACTORY $MCU_HOME_DIR/output/tmp
    rm -f $MCU_HOME_DIR/data/factory
    ln -sf $MCU_HOME_DIR/data/removed $MCU_HOME_DIR/data/factory
    sync
    mount -o remount,ro,async,noexec $PART3 $MCU_HOME_DIR/data
  fi
fi

#fix for systems that was downgraded from 5.0 before VNGR-12242
if [ -e $MCU_HOME_DIR/data/factory ]
then
  echo ""
else
  mount -o remount,rw,async,noexec $PART3 $MCU_HOME_DIR/data
  ln -sf $MCU_HOME_DIR/data/removed $MCU_HOME_DIR/data/factory
  sync
  mount -o remount,ro,async,noexec $PART3 $MCU_HOME_DIR/data
fi

# removing old folder, if exists
rm -Rf $MCU_HOME_DIR/config/system_cards_mode/*
rmdir $MCU_HOME_DIR/config/system_cards_mode
rm -Rf $MCU_HOME_DIR/config/mcms/SystemCardsMode/*
rmdir $MCU_HOME_DIR/config/mcms/SystemCardsMode

mkdir -p $MCU_HOME_DIR/config/static_states
chmod a+w $MCU_HOME_DIR/config/static_states

mkdir -p $MCU_HOME_DIR/config/keys
chown mcms:mcms $MCU_HOME_DIR/config/keys
mkdir -p $MCU_HOME_DIR/config/keys/cs
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs1
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs1
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs2
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs2
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs3
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs3
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs4
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs4
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs5
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs5
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs6
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs6
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs7
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs7
mkdir -p $MCU_HOME_DIR/config/keys/cs/cs8
chown mcms:mcms $MCU_HOME_DIR/config/keys/cs/cs8

#incase of upgrade change file to new name
for i in 1 2 3 4 5 6 7 8

do

  filename=$MCU_HOME_DIR/cs/ocs/cs$i/keys/certPassword.txt
  if [ -f $filename ]
  then  
    newFileName=$MCU_HOME_DIR/cs/ocs/cs$i/keys/certPassword.txt.orig	
    mv $filename $newFileName
  fi
  
  cs_destination=$MCU_HOME_DIR/cs/ocs/cs$i/keys
  cs_source=$MCU_HOME_DIR/config/keys/cs/cs$i/*
  
  cp -Rf $cs_source $cs_destination
done

#cp -Rf $MCU_HOME_DIR/config/keys/cs/* $MCU_HOME_DIR/cs/ocs/

mkdir -p $MCU_HOME_DIR/config/keys/ca_cert
chown mcms:mcms $MCU_HOME_DIR/config/keys/ca_cert
mkdir -p $MCU_HOME_DIR/config/keys/ca_cert/crl
chown mcms:mcms $MCU_HOME_DIR/config/keys/ca_cert/crl
chown -R mcms:mcms $MCU_HOME_DIR/config/states
chown -R mcms:mcms $MCU_HOME_DIR/config/static_states


cp -f $MCU_HOME_DIR/mcms/Versions.xml $MCU_HOME_DIR/tmp



OUTPUT_FOLDERS='apache core firmware cdr rec cslogs cslogs/cs1 cslogs/cs2
                cslogs/cs3 cslogs/cs4 cslogs/cs5 cslogs/cs6 cslogs/cs7 cslogs/cs8 faults tmp IVR mylogs media_rec audit local_tracer 
                media nids links tcp_dump tcp_dump/mcms tcp_dump/emb log backup external_ivr oprofile
				802_1x 802_1x/emb 802_1x/emb/media1 802_1x/emb/media2 802_1x/emb/media3 802_1x/emb/media4 802_1x/emb/switch'
cd $MCU_HOME_DIR/mcms

ulimit -c unlimited
ulimit -s 1024
ulimit -n 16384

echo -n $PRODUCT > $MCU_HOME_DIR/tmp/EMAProductType.txt
echo -n $LICENSE > $MCU_HOME_DIR/tmp/JITC_MODE.txt
if [ "$PRODUCT" != 'CALL_GENERATOR' ]
then
  echo "Reading time from switch using NTP_Bypass_Client" | tee -a $STARTUP_LOG 
  COUNT=3
  while [ $COUNT -gt 0 ]; do
    Bin/NTP_Bypass_Client
    CLIENT_RES=$?
    echo "NTP_Bypass_Client result=" $CLIENT_RES | tee -a $STARTUP_LOG
    if [ $CLIENT_RES -gt 0 ] 
	  then
	    echo "Failed accessing NTP bypass server." | tee -a $STARTUP_LOG
	    if [ $COUNT -gt 1 ]
	    then
	      sleep 5
	    fi
	    let COUNT=COUNT-1
    else
	    let COUNT=0
    fi
  done
fi

rm -Rf $MCU_HOME_DIR/output/802_1x/*

if test -e $MCU_HOME_DIR/mcms/States/restore_factory_defaults.flg; then
	echo "Restore factory defaults" | tee -a $STARTUP_LOG;

	echo "NO" > $MCU_HOME_DIR/mcms/JITC_MODE.txt
	echo "NO" > $MCU_HOME_DIR/mcms/JITC_MODE_FIRST_RUN.txt
	
  # Removes some of Cfg files
	mv $MCU_HOME_DIR/config/mcms/NetworkCfg_Management.xml $MCU_HOME_DIR/config/lost+found

	rm -Rf $MCU_HOME_DIR/config/mcms/*
	
	mv $MCU_HOME_DIR/config/lost+found/NetworkCfg_Management.xml $MCU_HOME_DIR/config/mcms/

	rm -Rf $MCU_HOME_DIR/config/states/*
	/bin/touch $MCU_HOME_DIR/config/states/McmsRestoreFactoryFileInd.flg

	rm -Rf $MCU_HOME_DIR/mcms/Links/*
	rm -Rf $MCU_HOME_DIR/config/ema/*
	rm -Rf $MCU_HOME_DIR/config/lost+found/*

	rm -Rf cs
    
	rm -Rf $MCU_HOME_DIR/mcms/StaticStates/MepMode.txt
    
	# removing all folders from HD
	cd $MCU_HOME_DIR/output
	rm -Rf $OUTPUT_FOLDERS
fi

cd $MCU_HOME_DIR/mcms
if test -e $MCU_HOME_DIR/config/states/restore_config.flg; then
  Scripts/RestoreConfig.sh $MCU_HOME_DIR/config/states 2>&1 1 >/dev/null;
fi

echo "Remove Diagnostics Indication file States/EnterDiagnosticsind"
rm -f $MCU_HOME_DIR/mcms/States/EnterDiagnosticsind

echo "Start copy IVR default files" | tee -a $STARTUP_LOG
if test -e $MCU_HOME_DIR/mcms/Cfg/IVR; then 
  Scripts/CopyDefaultIVR_no_overwrite.sh 2> /dev/null;
else
  echo "CopyDefaultIVR.sh" | tee -a $STARTUP_LOG
  Scripts/CopyDefaultIVR.sh 2> /dev/null;
fi

if test -e $MCU_HOME_DIR/mcms/Cfg/SystemCfgUserTmp.xml; then
	export SYSCFG_FILE="$MCU_HOME_DIR/mcms/Cfg/SystemCfgUserTmp.xml"
else
	export SYSCFG_FILE="$MCU_HOME_DIR/mcms/Cfg/SystemCfgUser.xml"
fi

# remove the old External IVR's base folder
if [[ -d Cfg/IVR/External ]]; then
	rm -Rf Cfg/IVR/External
fi

# remove the patch slides for AT&T
if [[ -d Cfg/IVR/Slides/ATT_Waiting_Room_Slide ]]; then
	rm -Rf Cfg/IVR/Slides/ATT_Waiting_Room_Slide
fi

Scripts/SetupCustomSlides.sh

echo "End copy IVR default files" | tee -a $STARTUP_LOG
chmod -R a+w $MCU_HOME_DIR/mcms/Cfg/IVR
chown -R mcms:mcms $MCU_HOME_DIR/mcms/Cfg/IVR
chown -R mcms:mcms $MCU_HOME_DIR/output/IVRTEMP

Scripts/CopyAudibleAlarms.sh	

mount -o remount,rw,async,noexec $PART3 $MCU_HOME_DIR/data
#rm -Rf $MCU_HOME_DIR/data/backup/*  
rm -Rf $MCU_HOME_DIR/data/restore/*
rm -Rf $MCU_HOME_DIR/data/new_version/*
chmod -R o-w $MCU_HOME_DIR/data/
chmod -R o-w $MCU_HOME_DIR/mcms/IVRX/
chown mcms:mcms $MCU_HOME_DIR/data/new_version
chown mcms:mcms $MCU_HOME_DIR/data/restore
chown mcms:mcms $MCU_HOME_DIR/data/backup
mount -o remount,ro,async,noexec $PART3 $MCU_HOME_DIR/data

cp $MCU_HOME_DIR/mcms/Scripts/TLS/passphrase.sh     $MCU_HOME_DIR/tmp
cp $MCU_HOME_DIR/mcms/Scripts/TLS/passphraseplcm.sh $MCU_HOME_DIR/tmp
chown mcms:mcms $MCU_HOME_DIR/tmp/passphrase.sh
chown mcms:mcms $MCU_HOME_DIR/tmp/passphraseplcm.sh

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

if [ "$PRODUCT" = 'CALL_GENERATOR' ]
then
  mkdir -p media/AUDIO
  chown mcms:mcms media/AUDIO
  chmod a+w media/AUDIO
  mkdir -p media/AUDIO/A001
  chown mcms:mcms media/AUDIO/A001
  chmod a+w media/AUDIO/A001
  mkdir -p media/VIDEO
  chown mcms:mcms media/VIDEO
  chmod a+w media/VIDEO
  mkdir -p media/VIDEO/V001
  chown mcms:mcms media/VIDEO/V001
  chmod a+w media/VIDEO/V001
  mkdir -p media/CONTENT
  chown mcms:mcms media/CONTENT
  chmod a+w media/VIDEO/V001    
  mkdir -p media/RECORDED_FILES
  chown mcms:mcms media/RECORDED_FILES
  chmod a+w media/RECORDED_FILES
fi

cd $MCU_HOME_DIR/mcms

# test if the mounting to the HD is OK, if it failed we will replace the HD with file system on ram (mounted to $MCU_HOME_DIR/output)
mount | grep output || ( mount -t ramfs hd_mem_sub $MCU_HOME_DIR/output -o maxsize=2000000;
  mkdir -p $MCU_HOME_DIR/output/core;
  mkdir -p $MCU_HOME_DIR/output/apache;
  mkdir -p $MCU_HOME_DIR/output/backup;
  ulimit -c 0) 

chown -R mcms:mcms $MCU_HOME_DIR/config/ema $MCU_HOME_DIR/config/mcms
touch $MCU_HOME_DIR/config/ema/InternalConfigSet_Customized.xml
chown mcms:mcms $MCU_HOME_DIR/config/ema/InternalConfigSet_Customized.xml

touch $MCU_HOME_DIR/config/ema/EMA.DataObjects.OfflineTemplates.AddressbookContent_.xml
chown mcms:mcms $MCU_HOME_DIR/config/ema/EMA.DataObjects.OfflineTemplates.AddressbookContent_.xml

touch $MCU_HOME_DIR/config/ema/EMA.DataObjects.OfflineTemplates.NoteListContent_.xml
chown mcms:mcms $MCU_HOME_DIR/config/ema/EMA.DataObjects.OfflineTemplates.NoteListContent_.xml


if test -e $MCU_HOME_DIR/mcms/JITC_MODE.txt; then
   MCMS_JITC_FILE="$MCU_HOME_DIR/mcms/JITC_MODE.txt"
fi

SNMP_FIPS_MODE="NO"

SNMP_FIPS_MODE_CFG=$(cat $SYSCFG_FILE | grep -A 1 SNMP_FIPS_MODE | grep DATA)

if [ "$SNMP_FIPS_MODE_CFG" ] && [$SNMP_FIPS_MODE_CFG == "<DATA>YES</DATA>" ]; then
	SNMP_FIPS_MODE="YES"
fi
if [ $SNMP_FIPS_MODE == "YES" ]; then
	echo -n "Running snmpdj" | tee -a $STARTUP_LOG
	Bin/snmpdj &
else
	echo -n "Running snmpd" | tee -a $STARTUP_LOG
	Bin/snmpd &
fi

IPMC=Bin/IPMCInterface
IPMC_LINK=$MCU_HOME_DIR/mcms/Links/IPMCInterface
MCMSD=Bin/McmsDaemon
DIAGNOSTICS=Scripts/DiagnosticsDaemon.sh
MCMSD_LINK=$MCU_HOME_DIR/mcms/Links/McmsDaemon

if [ -e $MCMSD_LINK ] 
then
  echo "Using patched McmsDaemon" | tee -a $STARTUP_LOG
  MCMSD=$MCMSD_LINK
fi

if [ -e $IPMC_LINK ] 
then
  echo "Using patched IPMCInterface" | tee -a $STARTUP_LOG
  IPMC=$IPMC_LINK
fi

echo "Removing Recording files" | tee -a $STARTUP_LOG
rm -Rf $MCU_HOME_DIR/output/media_rec/*
mkdir -p $MCU_HOME_DIR/output/media_rec/share

echo "Set file permissions..." | tee -a $STARTUP_LOG
# VNGR-17424 stig: The sticky bit is not set on public directories
chmod +t $MCU_HOME_DIR/tmp

# VNGR-17283 stig: NFS exported system files and system directories are not owned by root
chown root:root $MCU_HOME_DIR/mcms/Cfg/IVR $MCU_HOME_DIR/output/media_rec $MCU_HOME_DIR/output/core $MCU_HOME_DIR/mcms/TcpDumpEmb mcms/802_1xEmb 

chmod -R o-w $MCU_HOME_DIR/config
chmod -R o-w $MCU_HOME_DIR/config/mcms/
chmod -R o-w $MCU_HOME_DIR/output
chmod a+w $MCU_HOME_DIR/output/media_rec/share
chmod a+w $MCU_HOME_DIR/output/tcp_dump/emb
chmod a+w $MCU_HOME_DIR/output/core

echo "TICKS: " | tee -a $STARTUP_LOG
cat /proc/uptime | tee -a $STARTUP_LOG

SELF_SIGNED_LOG_DIR=$MCU_HOME_DIR/tmp/startup_logs/self_signed_log.log
echo -n "Running Self-signed certificate"
Scripts/Self_Signed_Cert.sh boot > $SELF_SIGNED_LOG_DIR 2>&1
Scripts/Check_Bonding_Enabled.sh $LICENSE 2>&1 $STARTUP_LOG
echo Starting McmsDaemon... | tee -a $STARTUP_LOG
if [ "$PRODUCT" != 'CALL_GENERATOR' ]
then
  ($IPMC || reboot;(sleep 20 ; $DIAGNOSTICS )& $MCMSD ;echo McmsDaemon finished;./Scripts/Reset.sh;if test -e $MCU_HOME_DIR/mcms/States/EnterDiagnosticsind; then echo "System is in Diagnostics skip reboot";else reboot; fi;)&
else
  ($MCMSD > /dev/null ;echo McmsDaemon finished;./Scripts/Reset.sh;reboot) &
fi

#added by huiyu
Scripts/Cntl_Alignment.sh

# we refresh /etc/exports list VNGR-22799 
exportfs -a

exit 0
