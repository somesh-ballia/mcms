#!/bin/sh
#echo -n `/sbin/ifconfig eth0|grep HWaddr|awk '{ print $5}'`

if test -e ~mcms/.bashrc; then
	TMP_MCU_HOME_DIR=`grep MCU_HOME_DIR ~mcms/.bashrc | cut -d'=' -f2`
fi
if [[ "$MCU_HOME_DIR" == "" ]]; then
        MCU_HOME_DIR=$TMP_MCU_HOME_DIR
fi

PRODUCT_TYPE=$(cat $MCU_HOME_DIR/mcms/ProductType)

if [[ "$PRODUCT_TYPE" == "SOFT_MCU_MFW" ]]
then
        ethname=`cat $MCU_HOME_DIR/mcms/Cfg/NetworkCfg_Management.xml |grep "NETWORK_INTERFACE" | awk -F'<|>' '{if(NF>3) {print $3} }' | head -1`
        echo -n `/sbin/ifconfig $ethname|grep HWaddr|awk '{ print $5 }'`
else
        echo -n `/sbin/ifconfig eth0|grep HWaddr|awk '{ print $5 }'`
fi
