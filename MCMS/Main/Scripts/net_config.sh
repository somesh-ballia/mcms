#!/bin/sh
echo "Network configuration script, system will be restarted after configuration."

echo -n "Enter IP address (XXX.YYY.ZZZ.WWW):"
read IP
echo -n "Enter subnet mask (XXX.YYY.ZZZ.WWW):"
read MASK
echo -n "Enter default gateway (XXX.YYY.ZZZ.WWW):"
read DGW
$MCU_HOME_DIR/mcms/Bin/McuCmd update_ip_service_configuration CSMngr $IP $MASK $DGW
$MCU_HOME_DIR/mcms/Bin/McuCmd update_network_configuration McuMngr $IP $MASK $DGW
sync
#sleep 3
#$MCU_HOME_DIR/mcms/Bin/McuCmd kill McmsDaemon
