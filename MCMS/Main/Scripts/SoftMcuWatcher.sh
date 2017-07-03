#!/bin/sh

PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`
LOG_NAME="$MCU_HOME_DIR/tmp/startup_logs/WatcherLogs/mcu_deamon.log"
[ -e $MCU_HOME_DIR/tmp/startup_logs/WatcherLogs ] || mkdir -p $MCU_HOME_DIR/tmp/startup_logs/WatcherLogs

LOG="tee -a "${LOG_NAME}
LOG_SIZE=5120000    # 5M
LOG_COUNT=5
GRACE=300
SLEEP=60
RESTART_TIME=30

first_time=0
fail_count=0

Edge_Apache_flag=0

############## Edge Apache Load ##################
function Edge_Apache_load
{
        PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`
        if [[ $PRODUCT_TYPE == "SOFT_MCU_EDGE" ]];then
                echo "-- Network checking for EDGE --" >> $MCU_HOME_DIR/tmp/startup_logs/soft_mcu_start_up.log
                Wait_Loop=0
                while [[ ! -e $MCU_HOME_DIR/tmp/httpd.listen.conf && $Wait_Loop < 5 ]]; do
                        echo "Waiting for $MCU_HOME_DIR/tmp/httpd.listen.conf to be created" >> $START_UP_LOG
                        (( Wait_Loop ++ ))
                        sleep 5
                done
                IP_ADDR=`ifconfig | grep 'inet addr' | grep -v '127.0.0.1' | head -1 | tr -s ' ' | cut -d ' ' -f3 | cut -d':' -f2 | grep -v '255.255.255.255'`
                Wait_Loop=0
                while [[ "X${IP_ADDR}" == "X" && $Wait_Loop < 5 ]]; do
                        echo "Waiting for IP to be set (current IP is: $IP_ADDR)" >> $START_UP_LOG
                        (( Wait_Loop ++ ))
                        sleep 5
                done
                L_PORT=`grep '^Listen' $MCU_HOME_DIR/tmp/httpd.listen.conf | cut -d':' -f2`
                echo -n "Checking Apache Listening port ($L_PORT): " >> $START_UP_LOG
                if [[ `netstat -ntlp  2> /dev/null | grep '^tcp' | grep $L_PORT | grep '255.255.255.255'` ]]; then
                        echo "`netstat -ntlp  2> /dev/null | grep '^tcp' | grep http | grep '255.255.255.255'` - need to restart" >> $START_UP_LOG
                        sleep 5
                        if [[ `whoami` == "root" ]]; then
                                pkill httpd
                        else
                                sudo pkill httpd
                        fi
                else
                        echo "`netstat -ntlp  2> /dev/null | grep '^tcp' | grep $L_PORT`" >> $START_UP_LOG
                fi
        fi
}
##################################################
############## log check #########################
function CheckLog
{
	[ -e ${LOG_NAME} ] || return

	#get log size
	size=`ls -al ${LOG_NAME} | awk -F ' ' '{ print $5 }'`
	echo "The size of ${LOG_NAME} is ${size}"
	if [ ${size} -gt ${LOG_SIZE} ]
	then
		mv -f ${LOG_NAME} ${LOG_NAME}_$(date +%Y-%m-%d_%H-%M)
	fi
	log_count=`ls ${LOG_NAME}* | wc -l`
	if [ ${log_count} -gt ${LOG_COUNT} ]
	then
		if [ $PRODUCT_TYPE == "SOFT_MCU_MFW" ]
		then
			cd $MCU_HOME_DIR/tmp
		else
			cd $MCU_HOME_DIR/output/tmp
		fi
		log_base=$(basename ${LOG_NAME})*
		ls -lrt ${log_base} | awk -F ' ' '{ print $9}' | head $(( $LOG_COUNT - $(ls -l ${log_base} | wc -l) )) 2>/dev/null | xargs rm 
		cd -
	fi
}
##################################################
############## handle failure process #############
function HandleRecovery
{
	if [[ -e "$MCU_HOME_DIR/output/MCMSDEBUG" || -e "$MCU_HOME_DIR/tmp/MCMSDEBUG" ]]                                       
	then                                                                                         
		echo "`date` MCMSDEBUG: SoftMcuWatcher.sh exit, no monitor." | $LOG                         
		exit 0                                                                               
	fi
	  
	fail_count=$(( $fail_count +1 ))
	if [ "${fail_count}" == "1" ]
	then
		first_time=`date +%s`
	elif [ ${fail_count} -gt 5 ]
	then
		now_time=`date +%s`
		interval=$(( ${now_time} - ${first_time} ))
		if [ ${interval} -gt 3600 ]   ##### > 1 hour
		then
			echo "The interval is more than 1 hour, so recount" | ${LOG}
			fail_count=0
		else
			echo "restart system" | $LOG
			uptime | $LOG
			ls -l $MCU_HOME_DIR/mcms/Cores/ | $LOG
			ps -e | $LOG
			sync

			if [[ $PRODUCT_TYPE == "SOFT_MCU_MFW" || $PRODUCT_TYPE == "SOFT_MCU_EDGE" ]]
                        then
                                echo "Omits reboot for SOFT_MCU_ MFW" | ${LOG}
                        else
				sudo reboot
				exit
			fi
		fi
	fi

	echo "restart service, ${fail_count} times in ${first_time}" | $LOG
	uptime | $LOG

	if [[ $PRODUCT_TYPE == "SOFT_MCU_MFW" || $PRODUCT_TYPE == "SOFT_MCU_EDGE" ]]
	then
		echo "Restart SoftMCU..." | $LOG
		touch $MCU_HOME_DIR/tmp/stop_watcher_arg
		sudo /sbin/service soft_mcu restart
		echo "SoftMCU started." | ${LOG}
		sleep ${RESTART_TIME}
	else
		sudo touch $MCU_HOME_DIR/tmp/MCMSDEBUG
		sudo /etc/rc.d/99soft_mcu stop
		sleep 5
		sudo /etc/rc.d/99soft_mcu start
		sleep ${RESTART_TIME}
		ret_restart=$(ps -ef | grep GesherMcmsStart.sh | grep -v grep)
		if [ "" == "${ret_restart}" ]
		then
			echo "fail to restart, and restart again" | $LOG
			sudo /etc/rc.d/99soft_mcu start
		fi	
		sleep ${RESTART_TIME}
		sudo /bin/rm -f $MCU_HOME_DIR/tmp/MCMSDEBUG
	fi
}
###################################################

CheckLog

date=$(date)
echo $date "SoftMcuWatcher is up" | $LOG

date=$(date)
echo $date "waiting "$GRACE" seconds for startup..." | $LOG
if [[ -e "$MCU_HOME_DIR/output/MCMSDEBUG" || -e "$MCU_HOME_DIR/tmp/MCMSDEBUG" ]]                                       
then                                                                                         
	echo "`date` MCMSDEBUG: SoftMcuWatcher.sh exit, no monitor." | $LOG                  
        exit 0                                                                               
fi
sleep $GRACE

# Do not deliver the change to V100.
#WATCHED_NO_CS="httpd McmsDaemon ConfParty Resource"
WATCHED_NO_CS="McmsDaemon ConfParty Resource"

WATCHED_ALL=${WATCHED_NO_CS}" calltask csman gkiftask h323LoadBalance mcmsif siptask"
if [ $PRODUCT_TYPE == "GESHER" ]
then
	WATCHED=${WATCHED_ALL}
	#test whether first installation
	if [ -e $MCU_HOME_DIR/mcms/Cfg/IPServiceListTmp.xml ]
	then
		IP_SPAN=$(grep "<IP_SPAN>" $MCU_HOME_DIR/mcms/Cfg/IPServiceListTmp.xml | head -1 | sed 's/ //g' | sed 's/\t//g')
		if [ "" == "${IP_SPAN}"  ]
		then
			WATCHED=${WATCHED_NO_CS}
		else
			echo "monitor all components" | $LOG
		fi
	else
		WATCHED=${WATCHED_NO_CS}
	fi
elif [[ $PRODUCT_TYPE == "SOFT_MCU_MFW"  || $PRODUCT_TYPE == "SOFT_MCU_EDGE" ]]
then
	#MRMP process is monitored in MpWatcher.sh
	WATCHED=${WATCHED_ALL}
else
	echo Exits on unsupported product $PRODUCT_TYPE | $LOG
	exit
fi

date=$(date)
echo $date "product type: $PRODUCT_TYPE" | $LOG
echo $date "watched processes: $WATCHED" | $LOG
while true
do
	date=$(date)
	ALL_PROCESSES=$(ps -e | grep -v defunct | awk -F ' ' '{print $4}')
	IsAllProccesesUp=false
	for process in $WATCHED
	do
		RET=$( echo ${ALL_PROCESSES} | grep $process )
		if [ "" != "${RET}" ]
		then
			IsAllProccesesUp=true
			#echo $date $process is up | $LOG
		else
			IsAllProccesesUp=false
			echo $date $process is down | $LOG
			HandleRecovery

			if [[ $PRODUCT_TYPE == "SOFT_MCU_MFW"  || $PRODUCT_TYPE == "SOFT_MCU_EDGE" ]]
	        	then
                		echo "Continues for $PRODUCT_TYPE." | $LOG
	        	else
				break
        		fi
		fi
	done

	if [[ "$Edge_Apache_flag" == "0" ]]
	then
		Edge_Apache_load
		Edge_Apache_flag=1
	fi

	if $IsAllProccesesUp ; then
		echo $date "Procceses: $WATCHED are up" | $LOG
	fi

	sleep $SLEEP

done

