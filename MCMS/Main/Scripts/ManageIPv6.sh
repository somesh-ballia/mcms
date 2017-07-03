#!/bin/bash
# ManageIPv6.sh 
# Written by: Shachar Bar

Check_State () {
	if [[ `grep '^NETWORKING_IPV6==yes' /etc/sysconfig/network` ]]; then
		return 0
	else
		return 1
	fi
}

Change_IPv6 () {
	#if [[ "$1" == "Enable" ]]; then
	#	sed -i 's/^#NETWORKING_IPV6=.*/NETWORKING_IPV6=yes/g' /etc/sysconfig/network
	#else
	#	sed -i 's/^NETWORKING_IPV6=.*/#NETWORKING_IPV6=yes/g' /etc/sysconfig/network
	#fi

 	if [[ -f $MCU_HOME_DIR/mcms/Cfg/NetworkCfg_Management.xml ]]; then
                INTERFACE_LIST=`cat $MCU_HOME_DIR/mcms/Cfg/NetworkCfg_Management.xml | grep NETWORK_INTERFACE | cut -d '>' -f2 | cut -d '<' -f1 | grep -v '^$'`
        else
                INTERFACE_LIST=`tail -n +3 /proc/net/dev | grep -v 'lo:' | cut -d':' -f1`
        fi

	for INTERFACE in $INTERFACE_LIST; do
		if [[ "$1" == "Enable" ]]; then
			#sed -i 's/^#IPV6INIT=.*$/IPV6INIT=yes/g' /etc/sysconfig/network-scripts/ifcfg-$INTERFACE
			sudo sed -i s/^#DHCPV6C=.*$/DHCPV6C=yes/g /etc/sysconfig/network-scripts/ifcfg-$INTERFACE
		else
			#sed -i 's/^IPV6INIT=.*$/#IPV6INIT=yes/g' /etc/sysconfig/network-scripts/ifcfg-$INTERFACE
			sudo sed -i s/^DHCPV6C=.*$/#DHCPV6C=yes/g /etc/sysconfig/network-scripts/ifcfg-$INTERFACE
		fi
	done
	sudo sync;sudo sync;sudo sync
	# Need service network restart + service soft_mcu restart at least !
}

if [[ "X$1" == "X" ]]; then
	echo "You must supply Enable or Disable as parameters"
	exit 1
fi
if [[ "$1" != "Enable" && "$1" != "Disable" ]]; then 
	echo "You must supply Enable or Disable as parameters"
	exit 1
fi

# Handling Enable
if [[ "$1" == "Enable" ]]; then
	if [[ `Check_State` == "0" ]]; then
		echo "Current IPv6 is already enabled"
		exit 1
	else
		ret=`Change_IPv6 Enable`
		exit $ret
	fi
fi

# Handling Disable
if [[ "$1" == "Disable" ]]; then
	if [[ `Check_State` == "1" ]]; then
		echo "Current IPv6 is already disabled"
		exit 1
	else
		ret=`Change_IPv6 Disable`
		exit $ret
	fi
fi

exit 1
