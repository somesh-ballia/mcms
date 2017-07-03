#!/bin/sh

# Header
# TestName: NinjaDiagStart.sh
# TestPath: /etc/init.d/
# Parameters: NULL
# ScriptOwnerMail: edwin.dong@polycom.com
# ScriptLog: $MCU_HOME_DIR/tmp/startup_logs/ninja_diag_start_sh.log
# ScriptType: CLI
# End Header

BASE_PATH=$MCU_HOME_DIR/tmp/mfa_cm_fs
MNT_PATH=/mnt/mfa_cm_fs/
SOURCE_COREDUMP_PATH=$MCU_HOME_DIR/output/core
DESTINATION_COREDUMP_PATH=$BASE_PATH/coredumps
DBG_FILE=$BASE_PATH/dbgCfg.txt
CLIENTCFG_FILE=$MCU_HOME_DIR/tmp/ClientCfg.xml
SOURCE_EMA_PATH=/EMA
DESTINATION_EMA_PATH=$BASE_PATH/ema
SITE_MODE_FILE=$DESTINATION_EMA_PATH/EMA.UI/JavaScript/EMAHelper.js
SITE_MODE_VALUE="var strSiteModeValue ="
SITE_MODE_NEW_VALUE="var strSiteModeValue = \"ShelfManagement\";"
SOURCE_APACHE_MODULE=$MCU_HOME_DIR/mcms/Bin/mod_embedded.so
DESTINATION_APACHE_MODULE=$DESTINATION_EMA_PATH/mod_embedded.so
EMA_CONFIG_PATH=$BASE_PATH/emaconfig
DIAG_STATUS_PATH=$EMA_CONFIG_PATH/Diag_Status.txt
EMA_SHELF_CONFIG_PATH=$EMA_CONFIG_PATH/EMAShelfconfig

create_dbgCfg()
{
    echo "IGNORE_THIS_FILE=NO" > $DBG_FILE
    echo "RELEASE_DSP=YES" >> $DBG_FILE
    echo "CM_INITIATOR_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "DIAG_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "DISPATCH_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "E2PROM_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "IPMI_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "SHELF_COM_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "LAN_SWITCH_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "MCMS_COM_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "NTP_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "SHARED_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "STARTUP_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "TCP_SERVER_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "USB_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "TIMER_PRINT_LEVEL=NORMAL" >> $DBG_FILE
    echo "OUTPUT_DESTINATION=FILE" >> $DBG_FILE
    echo "LOG_FILE_NAME=DiagLog.txt" >> $DBG_FILE
}

create_ClientCfg()
{
    echo "<CLIENT_CFG>" > $CLIENTCFG_FILE
    echo "        <REQUEST_PEER_CERTIFICATE>false</REQUEST_PEER_CERTIFICATE>" >> $CLIENTCFG_FILE
    echo "        <JITC_MODE>false</JITC_MODE>" >> $CLIENTCFG_FILE
    echo "</CLIENT_CFG>" >> $CLIENTCFG_FILE
}

change_site_mode()
{
    cat $SITE_MODE_FILE |grep -v "$SITE_MODE_VALUE" > $SITE_MODE_FILE.tmp 
    echo $SITE_MODE_NEW_VALUE >> $SITE_MODE_FILE.tmp
    mv $SITE_MODE_FILE $SITE_MODE_FILE.old
    mv $SITE_MODE_FILE.tmp $SITE_MODE_FILE
    chmod 755 $SITE_MODE_FILE
}

create_diag_status()
{
    echo "NO" | head -c 2 > $DIAG_STATUS_PATH
    chmod 777 $DIAG_STATUS_PATH
}

create_ema_shelf_config()
{
    echo "DiagMode=advanced;" | head -c 18 > $EMA_SHELF_CONFIG_PATH
    chmod 777 $EMA_SHELF_CONFIG_PATH
}

prepare_enviroment()
{
    echo "prepare_enviroment..." | tee -a $STARTUP_LOG

    cp -f $MCU_HOME_DIR/mcms/ProductType $MCU_HOME_DIR/tmp/EMAProductType.txt
    export LICENSE=$(cat $MCU_HOME_DIR/mcms/JITC_MODE.txt)
    echo -n $LICENSE > $MCU_HOME_DIR/tmp/JITC_MODE.txt
    create_ClientCfg

    #Prepare Base path
    mkdir $BASE_PATH
    RET=$?
    echo "mkdir $BASE_PATH : $RET" | tee -a $STARTUP_LOG

    ln -sf $SOURCE_COREDUMP_PATH $DESTINATION_COREDUMP_PATH
    RET=$?
    echo "ln -sf $SOURCE_COREDUMP_PATH $DESTINATION_COREDUMP_PATH : $RET" | tee -a $STARTUP_LOG

    create_dbgCfg

    #Prepare EMA path
    cp -rf $SOURCE_EMA_PATH $DESTINATION_EMA_PATH
    RET=$?
    echo "cp -rf $SOURCE_EMA_PATH $DESTINATION_EMA_PATH : $RET" | tee -a $STARTUP_LOG

    change_site_mode

    cp -rf $SOURCE_APACHE_MODULE $DESTINATION_APACHE_MODULE
    RET=$?
    echo "cp -rf $SOURCE_APACHE_MODULE $DESTINATION_APACHE_MODULE : $RET" | tee -a $STARTUP_LOG

    #Prepare EMA Config path
    mkdir $EMA_CONFIG_PATH
    RET=$?
    echo "mkdir $EMA_CONFIG_PATH : $RET" | tee -a $STARTUP_LOG
    chmod 777 $EMA_CONFIG_PATH
    
    create_diag_status
    create_ema_shelf_config

    #Mount
    mount $BASE_PATH $MNT_PATH
    RET=$?
    echo "mount $BASE_PATH $MNT_PATH : $RET" | tee -a $STARTUP_LOG
}

ENTER_TYPE=$1

####################################################################
STARTUP_LOG=$MCU_HOME_DIR/tmp/startup_logs/ninja_diag_start_sh.log
echo uptime | tee $STARTUP_LOG
uptime	| tee -a $STARTUP_LOGS
echo "TICKS: " | tee -a $STARTUP_LOG
cat /proc/uptime | tee -a $STARTUP_LOG
echo "ENTER_TYPE: $ENTER_TYPE" | tee -a $STARTUP_LOG
####################################################################

if [ $(whoami) != "root" ]
then
	echo "user $(whoami) is not root, Exit!" | tee -a $STARTUP_LOG
	exit 0
fi

ulimit -c unlimited
ulimit -s 1024

####################################################################

prepare_enviroment

export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin
export PATH=$PATH:$MCU_HOME_DIR/mcms/Bin:$MCU_HOME_DIR/mcms/Scripts
cd $MCU_HOME_DIR/mcms/Bin
DIAGD=./DiagModule
echo "DiagModule starting ... $(date)" | tee -a $STARTUP_LOG
( 
  $DIAGD 5 $ENTER_TYPE &> /dev/null;
  echo "DiagModule exit. $(date)" | tee -a $STARTUP_LOG;
  if [[ ! -e "$MCU_HOME_DIR/output/DIAGDEBUG" && ! -e "$MCU_HOME_DIR/tmp/DIAGDEBUG" ]] 
  then 
    echo "reboot" | tee -a $STARTUP_LOG;
    reboot
  else
    echo "DIAGDEBUG mode enabled." | tee -a $STARTUP_LOG
  fi
)&

exit 0
