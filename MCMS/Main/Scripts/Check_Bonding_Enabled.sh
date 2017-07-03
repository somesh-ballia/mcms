#!/bin/sh

STARTUP_LOG=$MCU_HOME_DIR/tmp/startup_logs/start_sh.log
ENABLE_BONDING="NO"
MULTIPLE_SERVICES="NO"
LAN_REDUNDANCY="NO"
GW_EQUALS="NO"
IP_TYPE="ipv4"
MNGMT_GW_IP="0"
IPSERVICE_GW_IP="0"

echo "Check_Bonding_Enabled.sh"
if test -e $MCU_HOME_DIR/mcms/Cfg/SystemCfgUserTmp.xml; then
  export SYSCFG_FILE="$MCU_HOME_DIR/mcms/Cfg/SystemCfgUserTmp.xml"
fi

if test -e $MCU_HOME_DIR/mcms/Cfg/NetworkCfg_Management.xml; then
  export MNGMT_CFG_FILE="$MCU_HOME_DIR/mcms/Cfg/NetworkCfg_Management.xml"
fi

if test -e $MCU_HOME_DIR/mcms/Cfg/IPServiceListTmp.xml; then
  export IPSERVICE_CFG_FILE="$MCU_HOME_DIR/mcms/Cfg/IPServiceListTmp.xml"
fi

#//if JITC_MODE parameters is YES Bonding will be disabled
if [[ $1 == "YES" ]];then
	echo -n 'NO' > $MCU_HOME_DIR/mcms/ENABLE_BONDING.txt | tee -a $STARTUP_LOG
	echo "JITC_MODE=YES -> write 'no to ENABLE_BONDING.txt and exit script" | tee -a $STARTUP_LOG
	exit 0
fi

MULTIPLE_SERVICES=$(cat $SYSCFG_FILE | grep -A 1 MULTIPLE_SERVICES | grep DATA)

if [ "$MULTIPLE_SERVICES" ] && [$MULTIPLE_SERVICES == "<DATA>YES</DATA>" ]; then
	MULTIPLE_SERVICES="YES"
fi

if [ $MULTIPLE_SERVICES == "YES" ]; then

	echo -n 'NO' > $MCU_HOME_DIR/mcms/ENABLE_BONDING.txt | tee -a $STARTUP_LOG
	echo "MULTIPLE_SERVICES=YES -> write 'no to ENABLE_BONDING.txt and exit script" | tee -a $STARTUP_LOG
	exit 0
else
	echo "MULTIPLE_SERVICES=NO, don't write to ENABLE_BONDING.txt and continue script"  | tee -a $STARTUP_LOG
fi

LAN_REDUNDANCY=$(cat $SYSCFG_FILE | grep -A 1 LAN_REDUNDANCY | grep DATA)

if [ "$LAN_REDUNDANCY" ] && [$LAN_REDUNDANCY == "<DATA>YES</DATA>" ]; then
	LAN_REDUNDANCY="YES"
fi

if [ $LAN_REDUNDANCY == "YES" ]; then

	echo "LAN_REDUNDANCY=YES, don't write to ENABLE_BONDING.tx and continue script" | tee -a $STARTUP_LOG
	
else
	echo -n 'NO' > $MCU_HOME_DIR/mcms/ENABLE_BONDING.txt | tee -a $STARTUP_LOG
	echo "LAN_REDUNDANCY=NO -> write 'no to ENABLE_BONDING.txt and exit script" | tee -a $STARTUP_LOG
	exit 0
fi

IP_TYPE=$(cat $MNGMT_CFG_FILE | grep IP_TYPE)

if [ "$IP_TYPE" ] && [$IP_TYPE == "<IP_TYPE>ipv4</IP_TYPE>" ]; then

	echo "IP_TYPE=ipv4, don't write to ENABLE_BONDING.tx and continue script"  | tee -a $STARTUP_LOG
	
else
	echo -n 'NO' > $MCU_HOME_DIR/mcms/ENABLE_BONDING.txt | tee -a $STARTUP_LOG
	echo "IP_TYPE different from ipv4 -> write 'no to ENABLE_BONDING.txt and exit script"  | tee -a $STARTUP_LOG
	exit 0
fi

MNGMT_GW_IP=$(cat $MNGMT_CFG_FILE | grep "<DEFAULT_ROUTER>" )
echo $MNGMT_GW_IP

IPSERVICE_GW_IP=$(cat $IPSERVICE_CFG_FILE | grep "<DEFAULT_ROUTER>" )
echo $IPSERVICE_GW_IP

if [ "$MNGMT_GW_IP" == "$IPSERVICE_GW_IP" ]; then

	echo "GW equals"  | tee -a $STARTUP_LOG
	echo -n 'YES' > $MCU_HOME_DIR/mcms/ENABLE_BONDING.txt  | tee -a $STARTUP_LOG
	
else
	echo "GW not equals"  | tee -a $STARTUP_LOG
	echo -n 'NO' > $MCU_HOME_DIR/mcms/ENABLE_BONDING.txt  | tee -a $STARTUP_LOG
fi


echo "end script"



