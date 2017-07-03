#!/bin/bash
# LogCollector.sh
# Written by: Shachar Bar 
# <shachar.bar@polycom.co.il>
####################################

MCU_HOME_DIR=
MCU_HOME_DIR=$(grep MCU_HOME_DIR /home/mcms/.bashrc | cut -d'=' -f2 | awk 'NR==1')
export MCU_LIBS=$MCU_HOME_DIR/lib:$MCU_HOME_DIR/lib64:$MCU_HOME_DIR/usr/lib:$MCU_HOME_DIR/usr/lib64

# Variable Definitions:
OUTP="$MCU_HOME_DIR/tmp/result.out"
LogFiles_Dir="$MCU_HOME_DIR/mcms/LogFiles"
CollectInfo_Script="$MCU_HOME_DIR/mcms/Scripts/CollectInfo.sh"
Target_CollectInfo_Script="$MCU_HOME_DIR/tmp/CollectInfo.sh"
Tmp_Results_File="$MCU_HOME_DIR/tmp/result.out"
Mcms_Logs="$MCU_HOME_DIR/mcms/LogsFiles"
Cs_Logs="$MCU_HOME_DIR/cs/logs/cs1"

AUTH="Authorization: Basic U1VQUE9SVDpTVVBQT1JU"
TYPE="Content-Type: application/vnd.plcm.dummy+xml"
PARM="If-None-Match: -1"
SOCKET=`grep '^Listen' $MCU_HOME_DIR/tmp/httpd.listen.conf | head -1 | cut -d' ' -f2`
IP=`echo -n $SOCKET | cut -d':' -f1`
PORT=`echo -n $SOCKET | cut -d':' -f2`
IS_SECURE=`echo -n \`cat $MCU_HOME_DIR/mcms/Cfg/NetworkCfg_Management.xml | grep '<IS_SECURED>true</IS_SECURED>'\``
PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`


if [[ "$IS_SECURE" != ""  &&  "$PORT" == "443" ]]; then
	HTTP_PREF="https://$IP"
elif [ "$IS_SECURE" != "" ]; then	
	HTTP_PREF="https://$IP:$PORT"
else
	HTTP_PREF="http://$IP:$PORT"
fi

####################################
# Wait for log collector to stop 
####################################
StopLogCollector() {
	
	curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -X POST -d "$DATA" ${HTTP_PREF}/plcm/mcu/api/1.0/system/info_collector/pack > "$OUTP"

	if [[ ! `grep "Status OK" "$OUTP"` ]]; then
		echo -n "Waiting for the info collector to be ready "
		
		while [ `curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -X POST -d "$DATA" ${HTTP_PREF}/plcm/mcu/api/1.0/system/info_collector/pack | grep -q "Status OK"` ]; do
			echo -n "."
			sleep 1
		done
		echo ""
	fi
}
####################################

####################################
# Get Log file name
####################################
GetFile() {
	LogFile=`ls -1tr $MCU_HOME_DIR/mcms/LogFiles/*.tgz 2> /dev/null`
	if [[ "X$LogFile" == "X" ]]; then
		echo "Log file wasn't generated"
	else
		echo "Generated file is $LogFile"
	fi
}

####################################
# Validate there is enough space in hardisk to write new logs
####################################
ValidateEnoughSpace(){
	
	LogSize=`du  -m $LogFiles_Dir/ | cut -f1 | tail -1`
	
	if [[ "$PRODUCT_TYPE" == "GESHER" || "$PRODUCT_TYPE" == "NINJA" || ("$PRODUCT_TYPE" == RMX* && (`whoami` == 'root' || `whoami` == 'mcms')) ]]; then
		FreeSpace=`df -m | grep -E ' /output$' | tr -s ' ' | cut -d' ' -f4`
				
	else
		FreeSpace=`df -m $MCU_HOME_DIR | tail -n -1 | tr -s ' ' | cut -d' ' -f4`		
	fi			
	
	if [[ $LogSize -gt $FreeSpace ]]; then
		echo "Log collection failed - Not enough disk space"
		exit 0
	fi	 	 	
}

####################################
# The Clean temporary files function
####################################
Clean() {
	echo "Cleaning temporary files"
	# Stop the Collector Process
	curl -1 -k -H "$PARM" -H "$TYPE" -H "$AUTH" -X DELETE ${HTTP_PREF}/plcm/mcu/api/1.0/system/info_collector/pack > /dev/null 2>&1
	# Get the state of the log collection
	curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -G ${HTTP_PREF}/plcm/mcu/api/1.0/system/info/state > /dev/null 2>&1
	# Quit by timeout
	curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -G ${HTTP_PREF}/plcm/mcu/api/1.0/system/info/state > /dev/null 2>&1
	# Gets the log filename
	curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -G ${HTTP_PREF}/plcm/mcu/api/1.0/system/info_collector/pack > /dev/null 2>&1
	# The next one shows the created log file
	#GetFile
	# Cleaning temporary files
	rm -f $Target_CollectInfo_Script
	rm -f $Tmp_Results_File
	#rm -f $Mcms_Logs/Log_*
	#rm -rf $Cs_Logs/{Sun_*,Mon_,Tue_,Wed_,Thu_,Fri_,Sat_}*
	# The following removed the final created log file
	rm -f $Mcms_Logs/CollecInfo*.tgz
}
####################################

####################################
# The TERM signal handler
####################################
Terminate() {
	echo "Terminating on request"
	Clean
	exit 1
}
####################################

####################################
# Set the TERM signal handler
####################################
trap 'Terminate' TERM INT
####################################

####################################
# Help function
####################################
Help() {
	echo ""
	echo "Welcome to the Log Collector utility"
	echo "------------------------------------"
	if [[ "$PRODUCT_TYPE" == "SOFT_MCU_MFW" ]] ; then
		echo "Usage: $0 -u <user Name> -p <User Password> [-#m | -#h | -#d | -h]"
	else
		echo "Usage: $0  [-#m | -#h | -#d | -h -u <user Name> -p <User Password>]"
	fi
	echo "Parameter explanations:"
	echo "#  - numeric value"
	echo "-u - user name"
	echo "-p - user password"	
	echo "m  - the preceding value specifies the last number of minutes"
	echo "h  - the preceding value specifies the last number of hours"
	echo "d  - the preceding value specifies the last number of days"
	echo "-h - to show this output"
	echo ""
}
####################################

####################################
# Error handeling function
####################################
Error() {
	if [[ "$1" == "1" ]]; then
		echo "Couldn't copy $CollectInfo_Script $Target_CollectInfo_Script"
		exit 1
	fi
}
####################################

####################################
# Add_Last_Logs
####################################
Add_Last_Logs() {
	if [[ "X$1" == "X" ]]; then
		LogFile=`ls -1tr $MCU_HOME_DIR/mcms/LogFiles/*.tgz | tail -1 2> /dev/null`
	else
		LogFile=$1
	fi
        if [[ "X$LogFile" == "X" ]]; then
		return
	fi
	if [[ ! `tar ztvf $LogFile | grep 'Log_SN.*\.log'` && `ls -l $MCU_HOME_DIR/mcms/LogFiles | grep 'Log_SN.*\.log'` ]]; then
		mkdir -p $MCU_HOME_DIR/mcms/LogFiles/tmp_log_dir 2> /dev/null
		cd $MCU_HOME_DIR/mcms/LogFiles/tmp_log_dir
		mkdir -p output/log 2> /dev/null
		tar zxf $LogFile
		Last_MCMS_Log_file="$MCU_HOME_DIR/mcms/LogFiles/`ls -tr $MCU_HOME_DIR/mcms/LogFiles | grep 'Log_SN.*\.log' | tail -1`"
		cp -f $Last_MCMS_Log_file output/log
		tar zcf $LogFile .
		cd - > /dev/null
		rm -rf $MCU_HOME_DIR/mcms/LogFiles/tmp_log_dir 2> /dev/null
	fi
}
####################################

####################################
# Collect_Log function
####################################
Collect_Log() {
	Time="$1"
	Type="$2"
	ValidateEnoughSpace
	# Wait for previous log collector run to stop
	StopLogCollector
        # time format is: YYYY-MM-DDTHH:MM:SS
	#Now=`date --rfc-3339=seconds | cut -c1-19 | tr ' ' 'T'`
	Now=`date -u --rfc-3339=seconds`
	Now_Epoch=`date --date="${Now}" +"%s"`
	
	Now_localTime=`date --rfc-3339=seconds`
	Now_Epoch_localTime=`date --date="${Now}" +"%s"`
	
	if [[ "$Type" == "h" ]]; then
	#	Back=`echo "$Now_Epoch-($Time*3600)"|bc`
		Back=`expr $Now_Epoch - \( $Time \* 3600 \)`
		Back_localTime=`expr $Now_Epoch_localTime - \( $Time \* 3600 \)`	
	elif [[ "$Type" == "d" ]]; then
	#	Back=`echo "$Now_Epoch-($Time*3600*24)"|bc`
		Back=`expr $Now_Epoch - \( $Time \* 3600 \* 24 \)`
		Back_localTime=`expr $Now_Epoch_localTime - \( $Time \* 3600 \* 24 \)`
	elif [[ "$Type" == "m" ]]; then
	#	Back=`echo "$Now_Epoch-($Time*60)"|bc`
		Back=`expr $Now_Epoch - \( $Time \* 60 \)`
		Back_localTime=`expr $Now_Epoch_localTime - \( $Time \* 60 \)`
	else
	#	Back=`echo "$Now_Epoch-(1*3600*24*365)"|bc`
		Back=`expr $Now_Epoch - \( 3600 \* 24 \* 365 \)`
		Back_localTime=`expr $Now_Epoch_localTime - \( 3600 \* 24 \* 365 \)`
	fi
	From=`date -u -d @${Back} --rfc-3339=seconds`
	From=`echo $From | cut -c1-19 | tr ' ' 'T'`
	Now=`echo $Now | cut -c1-19 | tr ' ' 'T'`
	
	From_localTime=`date -d @${Back_localTime} --rfc-3339=seconds`
	From_localTime=`echo $From_localTime | cut -c1-19 | tr ' ' 'T'`
	Now_localTime=`echo $Now_localTime | cut -c1-19 | tr ' ' 'T'`
	
	
	#echo "Collecting logs since `echo -n $From | tr 'T' ','` till `echo -n $Now | tr 'T' ','`"
	rm -f $Target_CollectInfo_Script 2> /dev/null
	cp -f $CollectInfo_Script $Target_CollectInfo_Script || Error 1
	# Update the start and end time stamps
	sed -i -e "s/^START_TIME=.*$/START_TIME=\"$From\"/" $Target_CollectInfo_Script
	sed -i -e "s/^END_TIME=.*$/END_TIME=\"$Now\"/" $Target_CollectInfo_Script
	# Set all captured data to true
	sed -i -e "s/>false</>true</g" $Target_CollectInfo_Script
	sed -i -e "s/___AUTH_TO_REPLACE___/$AUTH/g" $Target_CollectInfo_Script 
	# Flushing Logger
	( export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin ; $MCU_HOME_DIR/mcms/Bin/McuCmd flush Logger ) &> /dev/null
	sleep 2
	
	# Checking REST API
	TYPE="Content-Type: application/vnd.plcm.dummy+xml"
	PARM="If-None-Match: -1"
	SOCKET=`grep '^Listen' $MCU_HOME_DIR/tmp/httpd.listen.conf | head -1 | cut -d' ' -f2`
	RestAPI=`curl -1 -s -k -H "$PARM" -H "$TYPE" -H "$AUTH" -G ${HTTP_PREF}/plcm/mcu/api/1.0/health/status`
	if [[ `echo -n $RestAPI | grep '^<MCU_STATE>.*success.*</MCU_STATE>'` ]]; then
		echo "Collecting logs :"
        echo "since `echo -n $From | tr 'T' ','` (UTC)        till `echo -n $Now | tr 'T' ','` (UTC)"
        echo "since `echo -n $From_localTime | tr 'T' ','` (Local Time) till `echo -n $Now_localTime | tr 'T' ','` (Local Time)"
		. $Target_CollectInfo_Script
	else
		echo "Collecting logs in offline mode"
		From=`echo -n $From | tr -d 'T-' | cut -d':' -f1,2 | tr -d ':'`
		Now=`echo -n $Now | tr -d 'T-' | cut -d':' -f1,2 | tr -d ':'`
		Backup_FileName="$MCU_HOME_DIR/mcms/LogFiles/CollectInfo_${From}-${Now}.tgz"
		Last_10_mcms_logs=`ls -tr $MCU_HOME_DIR/mcms/LogFiles/*.log 2> /dev/null | tail`
 		tar cz -f ${Backup_FileName} ${Last_10_mcms_logs} $MCU_HOME_DIR/tmp/startup_logs/*.log 2> /dev/null
		sleep 3
		sync; sync; sync
		Add_Last_Logs ${Backup_FileName}
		echo "Generated file is $Backup_FileName"
		exit 0
	fi

	Add_Last_Logs
	exit 0
}
####################################

####################################
# Main function
####################################

user=""
password=""

Param=""

while [ "$1" != "" ]; do
  if [ "$1" == "-u" ] ; then
        shift;
        user="$1"
        shift;
  elif [ "$1" == "-p" ] ; then
        shift;
        password="$1"
        shift;
  else
		Param="$1"
        shift;
  fi
done;

if [[ "X$user" == "X" || "X$password" == "X" ]]; then
	if [[ "$PRODUCT_TYPE" == "SOFT_MCU_MFW" ]] ; then
		Help  
		exit 1		
	else
		user="SUPPORT"
		password="SUPPORT"
	fi
fi

UserPassword="$user:$password"

AUTH=`echo -n $UserPassword | openssl base64`

AUTH="Authorization: Basic $AUTH"

#echo "user: $user password $password AUTH $AUTH"


####################################
# Checking parameters
####################################
All="N"

if [ "X$Param" == "X" ]; then
	#Help
        #exit 0
	All="Y"
fi

if [[ "${Param:0:1}" == '-' ]]; then
	Param="${Param:1}"
else
	if [[ "$All" == "N" ]]; then
		echo "Unknown parameter: ${Param}"
		Help
		exit 1
	fi
fi

if [[ "$Param" == "h" ]]; then
	Help
	exit 0
fi

if [[ "$All" == "N" ]]; then
	Param_Len="${#Param}"
	Param_Period_ind=`expr ${Param_Len} - 1`
	# Getting the period numeric value - #"
	Param_Period_value="${Param:0:${Param_Period_ind}}"
else
	Param_Period_value="A"
fi

# Checking if the parameter period is non-numeric
case ${Param_Period_value} in
'A') echo "Collecting last year's logs"
	;;	
*[!0-9]*) echo "$Param_Period_value is not an acceptible numeric value"
	Help
	exit 2
	;;
*)
	;;
esac

# Getting the Parameter period character: m|h|d"
if [[ "$All" == "N" ]]; then
	Param_Period_type="${Param:${Param_Period_ind}:1}"
else
	Param_Period_type="A"
fi
if [[ "X$Param_Period_type" == "X" || ( "$Param_Period_type" != "d" && "$Param_Period_type" != "h" && "$Param_Period_type" != "m" && "$Param_Period_type" != "A" ) ]]; then
	echo "Unknown period type in ${Param}"
	Help
	exit 1
fi

# Calling the SoftMcuStatus before collecting the logs
./SoftMcuStatus.sh > $MCU_HOME_DIR/tmp/startup_logs/SoftMcuStatus.log 2>&1 

# Executing Insatll_Validator before collecting the logs
./InstallValidator.sh > $MCU_HOME_DIR/tmp/startup_logs/InstallValidator.log 2>&1

# Calling Collect_Log function with the period value and the time frame type character
Collect_Log $Param_Period_value $Param_Period_type
exit 0

# End of Script
