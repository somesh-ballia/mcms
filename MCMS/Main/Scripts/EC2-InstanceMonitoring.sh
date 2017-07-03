#!/bin/bash
#
# Written by Pugatzky Ori
#
# Setup AWS cloud CLI tools and run instance monitoring and notification.
# Monitoring - for #conferences , #participants
# If process/service is down - service is reset, and AWS SNS is sent
# If no participants are connected, system is reset once an hour
# 

PARAMS_FILE="$MCU_HOME_DIR/mcms/Scripts/EC2-CloudParams.sh"
. $PARAMS_FILE


cd /root

if [[ ! -d /root/CloudWatch-1.0.12.1 ]];then
	rm -f CloudWatch-2010-08-01.zip
	wget http://ec2-downloads.s3.amazonaws.com/CloudWatch-2010-08-01.zip
	unzip CloudWatch-2010-08-01.zip &> /dev/null
fi

if [[ ! -d /root/SimpleNotificationServiceCli-1.0.3.0 ]];then
	rm -f SimpleNotificationServiceCli-2010-03-31.zip
	wget http://sns-public-resources.s3.amazonaws.com/SimpleNotificationServiceCli-2010-03-31.zip
	unzip SimpleNotificationServiceCli-2010-03-31.zip &> /dev/null
fi

export JAVA_HOME=/usr/java/default
export PATH=$PATH:$JAVA_HOME/bin

export AWS_CLOUDWATCH_HOME=/root/CloudWatch-1.0.12.1/
export AWS_SNS_HOME=/root/SimpleNotificationServiceCli-1.0.3.0
export PATH=$PATH:$AWS_CLOUDWATCH_HOME/bin:$AWS_SNS_HOME/bin

chmod 600 /root/CloudWatch-1.0.12.1/credential-file-path.template
export AWS_CREDENTIAL_FILE=/root/CloudWatch-1.0.12.1/credential-file-path.template



MCUProcesses=( McmsDaemon ApacheModule Auditor  Authentication BackupRestore CDR CSApi CSMngr Cards CertMngr Collector ConfParty Configurator DNSAgent EncryptionKeySe ExchangeModule Failover Faults Gatekeeper LdapModule Logger McuMngr MplApi QAAPI Resource RtmIsdnMngr SNMPProcess SipProxy Ice SystemMonitorin Utility NotificationMngr soft.sh acloader csman calltask gkiftask h323LoadBalance mcmsif siptask launcher ManageMcuMenu Proxy audio_soft ASS-AH AMP-AmpAHDec0 AMP-AmpAHEnc0 AMP-AmpAHIvr0 AMP-AmpAHMgr ASS-AMPMgr ASS-AMPTx ASS-AMPUdpRx mpproxy mp sys sys_status_moni traced video runmfa.sh IpmcSim.x86 mfa )

PARTY_NUM_URL="https://127.0.0.1:443/plcm/mcu/api/1.0/participant/summary"
CONFERENCE_LIST_URL="https://127.0.0.1:443/plcm/mcu/api/1.0/conference/summary"
HTTP_HEADER="Authorization: Basic $HTTP_HEADER_BASE64"

CHECK_PARTY=0
TIMER=0
IP=`ifconfig eth0 | grep "inet addr:" | cut -d':' -f2 | cut -d' ' -f1`
EC2_INSTANCE_ID="`wget -q -O - http://169.254.169.254/latest/meta-data/instance-id`" 
EC2_AVAIL_ZONE="`wget -q -O - http://169.254.169.254/latest/meta-data/placement/availability-zone`"
EC2_REGION="`echo \"$EC2_AVAIL_ZONE\" | sed -e 's:\([0-9][0-9]*\)[a-z]*\$:\\1:'`"
EXT_IP=`wget -q -O - http://169.254.169.254/latest/meta-data/public-ipv4`

LAST_ERR=""
ERR_COUNT=0


# CallGenerator related exports
#export CALLGEN_IP=212.179.41.17
export MCU_IP=`cat $MCU_HOME_DIR/tmp/cloudIp`
#export MCU_CONF=0802
#export MCU_PWD="CloudMe2"
#export DELAY=2

############################################
notify_sns_reset (){
	SUBJECT=`echo "$1" | cut -c1-38`
	sns-publish arn:aws:sns:us-east-1:586973347769:SoftMcu_Reset --subject "AWS:$EXT_IP was reset ($SUBJECT) " --message "SoftMcu ($EC2_INSTANCE_ID) was reset due to $1"
	echo ""
}

############################################
notify_sns (){
	SUBJECT=`echo "$1" | cut -c1-38`
	sns-publish arn:aws:sns:us-east-1:586973347769:SoftMcu_Reset --subject "AWS:$EXT_IP $1 " --message "SoftMcu ($EC2_INSTANCE_ID) - $1"
	echo ""
}

############################################
reset_service (){
	notify_sns_reset "$@"
	check_reboot "$@"

	service soft_mcu restart

	sleep $(( 60 * 3 ))
}


############################################
check_reboot (){
	INPUT="$1"
	if [[ $INPUT != "bridge was empty" ]]; then
		if [[ $INPUT == $LAST_ERR ]]; then
			ERR_COUNT=$(( $ERR_COUNT + 1 ))
			echo "ERR_COUNT=$ERR_COUNT"
			if [[ $ERR_COUNT == 3 ]]; then
				echo "REBOOTING MACHINE - Same error 3 times: $LAST_ERR"
				ERR_COUNT=0
				reboot
			fi
		else
			LAST_ERR=$INPUT
			ERR_COUNT=0
		fi	
	fi
}

############################################
timer_tests(){
	echo `date +%D-%H:%M`
	#Allow check parties once an hour
	if [[ `date +%M` == 55 ]]; then
		CHECK_PARTY=1
	fi

	#Allow run CallGen tests every 10 minutes
	if [[ $(( `date +%-M` % 10 )) == $DELAY ]]; then
		echo "Enable CallGen test"
		CALL_GEN_CALL=1
	fi
}


sleep $(( 60 * 3 ))

######################### MAIN #############################


(while [ TRUE ]; do

	# Get Conference list
	#####################
	CONF_REPLY=`wget -nv -O - --no-check-certificate $CONFERENCE_LIST_URL --header "$HTTP_HEADER" 2> /dev/null`
	if [ $? == 0 ]; then
		#PARSE REPLY
		CONF_NUM=`echo $CONF_REPLY | cut -d">" -f3 | cut -d"<" -f1`
		#Report
		mon-put-data --dimensions "ID=$EC2_INSTANCE_ID,Zone=$EC2_AVAIL_ZONE,Region=$EC2_REGION" -m Conferences -n "MCUReport" -v $CONF_NUM
		echo "Conferences= $CONF_NUM"
	fi


	# Get number of parties
	#####################

	PARTY_REPLY=`wget -nv -O - --no-check-certificate $PARTY_NUM_URL --header "$HTTP_HEADER" 2> /dev/null`
	if [ $? == 0 ]; then
		#PARSE REPLY
		PARTY_NUM=`echo $PARTY_REPLY | cut -d">" -f3 | cut -d"<" -f1`
		#Report
		mon-put-data --dimensions "ID=$EC2_INSTANCE_ID,Zone=$EC2_AVAIL_ZONE,Region=$EC2_REGION"  -m Participants -n "MCUReport" -v $PARTY_NUM
		echo "Paticipants= $PARTY_NUM"
		
		if [ $CHECK_PARTY == 1 ]; then
			#If no parties are connected - reset soft_mcu service
			if [ $PARTY_NUM == 0 ];then
				CHECK_PARTY=0
				reset_service "bridge was empty"
				continue
			fi
		fi

		
		# Dial-in from Cloud CallGenerator to validate connectivity
		if [[ $CALL_GEN_CALL == 1 ]]; then
			# Only If no parties are connected
			if [ $PARTY_NUM == 0 ];then
				cd $MCU_HOME_DIR/mcms
				Scripts/SoftMcuDialFromCG2Cloud.py
				if [[ $? != 0 ]]; then
					reset_service "Failed to connect CallGenerator test call"
				else
					echo "CallGenerator test call - OK"
				fi
				cd -
			fi
			CALL_GEN_CALL=0
		fi
	fi


	# Check service status
	######################
	SERVICE_REPLY=`service soft_mcu status`
	if [[ $SERVICE_REPLY == "The MRM service is up" ]]; then
		SERVICE_STATUS=1
	else
		SERVICE_STATUS=0
		reset_service "service was down"
		continue
	fi
	#mon-put-data -m ServiceStatus -n "MCUReport" -v $SERVICE_STATUS
	echo "ServiceStatus=$SERVICE_STATUS"


	# Check proccesses are up
	#########################
	FAILED_PROCESSES=""
	PS_REPLY=1
	PS_LIST=`ps -A`
	ELEMENTS=${#MCUProcesses[@]}
	for (( i=0; i <$ELEMENTS; i++)); do

	        PROCESS=${MCUProcesses[i]}
		echo $PS_LIST | grep -q $PROCESS 
		if [ $? == 1 ]; then
			FAILED_PROCESSES="$FAILED_PROCESSES $PROCESS"
		fi	
	done
	if [[ $FAILED_PROCESSES != "" ]];then
		echo "$FAILED_PROCESSES"
		reset_service "the following processes were not found: ($FAILED_PROCESSES)"
		continue
	fi
	PS_REPLY=0

	# Check 'mfam process cpu% is lower than 85%
	############################################
        MFA=`pgrep "^mfa$"`
	CPU=`ps -p $MFA -o %cpu | grep -v CPU | cut -d' ' -f2`
	echo "mfa($MFA): $CPU"
	if [[ $CPU > 85 ]];then
		echo "Process 'mfa' takes $CPU"
		if [[ $PARTY_NUM == 0 ]];then
			echo "RESETTING DUE TO MFA"
			reset_service "'mfa' process with high cpu ($CPU)"
			continue
		fi
	fi

	#mon-put-data -m ProcessesStatus -n "MCUReport" -v $PS_REPLY
	echo "ProcessesStatus=$PS_REPLY"
	
	echo "=============================="

	timer_tests
	
	#Wait 1 minute	
	sleep 60

	timer_tests

done) | tee /var/log/EC2-InstanceMonitoring.log


