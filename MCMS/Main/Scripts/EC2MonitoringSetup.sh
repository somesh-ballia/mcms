#!/bin/bash
#
# Written by Pugatzky Ori
#
# Setup AWS cloud CLI tools
# Edits EC2-InstanceMonitoring.sh parameters
# Adds 'soft_mcu_monitor' as service.
# 
NO_COLOR=$(tput sgr0)
BLACK=$(tput setaf 0)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
LIME_YELLOW=$(tput setaf 190)
YELLOW=$(tput setaf 3)
POWDER_BLUE=$(tput setaf 153)
BLUE=$(tput setaf 4)
MAGENTA=$(tput setaf 5)
CYAN=$(tput setaf 6)
WHITE=$(tput setaf 7)
BRIGHT=$(tput bold)
NORMAL=$(tput sgr0)
BLINK=$(tput blink)
REVERSE=$(tput smso)
UNDERLINE=$(tput smul)

control_c()
# run if user hits control-c - TRAP
{
  echo -e ${NO_COLOR}
  rm -f $PARAMS_FILE &> /dev/null
  exit
}
 
# trap keyboard interrupt (control-c)
trap control_c SIGINT



export PARAMS_FILE="$MCU_HOME_DIR/mcms/Scripts/EC2-CloudParams.sh"

##########################
# 1. VERIFICATIONS
##########################
if [[ $USER != "root" ]];then
	echo -e  ${RED}"To be used by 'root' only."${NO_COLOR}
	exit 1
fi

if [[ `chkconfig --list | grep soft_mcu` == "" ]]; then
	echo -e ${RED}"SoftMcu packages must be installed before configuring cloud monitoring."${NO_COLOR}
	exit 2
fi
if [[ `service soft_mcu status` == "SoftMcu service is down" ]]; then
	echo -e ${RED}"Please start soft_mcu service before configuring Cloud monitoring."${NO_COLOR}
	exit 2
fi


##########################
# 2. COLLECT DATA
##########################

clear
echo -e ${BLUE}"This script sets up Amazon Cloud Watch and Monitoring."
echo "Please enter the following data:"
echo -e "--------------------------------" ${NO_COLOR}

read -p ${GREEN}"Enter EMA admin account name: "${BLACK} ADMIN
read -p ${GREEN}"Enter EMA admin account password: "${BLACK} MCU_PWD

HTTP_HEADER_BASE64=`echo -n "$ADMIN:$MCU_PWD" | base64`

read -p ${GREEN}"Enter AWS Account AWSAccessKeyId: "${BLACK} AWSKEYID
read -p ${GREEN}"Enter AWS Account AWSSecretKey: "${BLACK} AWSECKEYID


CALLGEN="Yes"
read -p ${GREEN}"Would you like to setup Call-Generator tests [Yes] ?"${BLACK} CALLGEN
if [[ $CALLGEN != "Yes" && $CALLGEN != "" ]];then
	#Disable callgen tests
	sed -i "s/CALL_GEN_CALL=[0,1]/CALL_GEN_CALL=0/" $MCU_HOME_DIR/mcms/Scripts/EC2-InstanceMonitoring.sh
else
	#Enable callgen tests
	sed -i "s/CALL_GEN_CALL=[0,1]/CALL_GEN_CALL=1/" $MCU_HOME_DIR/mcms/Scripts/EC2-InstanceMonitoring.sh
	while true; do
		read -p ${GREEN}"Call Generator IP [212.179.41.17]: "${BLACK} CALLGEN_IP
		if [[ $CALLGEN_IP == "" ]]; then
			CALLGEN_IP="212.179.41.17"
		fi
		ping -c 1 $CALLGEN_IP &> /dev/null
		if [[ $? != 0 ]]; then
			echo -e ${RED}"Address is not reachable"${NO_COLOR}
			sleep 2
		else
			break
		fi
	done

	read -p ${GREEN}"Call Generator destination conf id: "${BLACK} MCU_CONF

	read -p ${GREEN}"Cloud machine serial number: "${BLACK} SERIAL
	export DELAY=$(( 2*$SERIAL ))

fi

##########################
# 3. WRITE DATA
##########################

# Update EC2-InstanceMonitoring.sh script params file
echo "export HTTP_HEADER_BASE64=\"$HTTP_HEADER_BASE64\"" > $PARAMS_FILE
echo "export CALLGEN_IP=\"$CALLGEN_IP\"" >> $PARAMS_FILE
echo "export MCU_PWD=\"$MCU_PWD\"" >> $PARAMS_FILE
echo "export MCU_CONF=\"$MCU_CONF\"" >> $PARAMS_FILE
echo "export DELAY=$DELAY" >> $PARAMS_FILE

echo -e "AWSAccessKeyId=$AWSKEYID\nAWSSecretKey=$AWSECKEYID" > /root/CloudWatch-1.0.12.1/credential-file-path.template

##########################
# 4. PREPARE SERVICE
##########################

if [ -e $PARAMS_FILE ]; then
	# Add service
	cp -f $MCU_HOME_DIR/mcms/Scripts/soft_mcu_monitor /etc/init.d/ 
	if [[ $? == 0 ]]; then
		chkconfig --add soft_mcu_monitor
		chkconfig --level 345 soft_mcu_monitor on
	else
		rm -f $PARAMS_FILE &> /dev/null
		exit 3
	fi
	
	echo -e ${BLUE}"-----------------------------------------------"
	echo -e "SoftMcu cloud monitoring installed ${GREEN}successfully"
	echo -e ${BLACK}${BLINK}"Use: 'service soft_mcu_monitor start' to run."${NO_COLOR}
fi


