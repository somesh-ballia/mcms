#!/bin/sh

#Modified: Ninja VideoRecoverySupport, 20/1/2014

MEMORY_AMOUNT=`cat /proc/meminfo | grep MemTotal | awk '{ print $2 }'`
MFWHIGHMEMORY=16000000

PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`
LOG_NAME="$MCU_HOME_DIR/tmp/startup_logs/WatcherLogs/mp_deamon.log"
[ -e $MCU_HOME_DIR/tmp/startup_logs/WatcherLogs ] || mkdir -p $MCU_HOME_DIR/tmp/startup_logs/WatcherLogs
LOG="tee -a "${LOG_NAME}
LOG_SIZE=512000    #500K
LOG_COUNT=5

Total_fail=5
Fail_count=0
FailTimeArray=
MagicNum="new1A2B3C4D5E2"

# these 2 param are max startup time and current startup time. If Current > StartupTime, then restart SoftMCU
StartupTime=60
CurrentTime=0
IntervalTime=5

MonitorProcesses=("mfa" "mpproxy" "audio_soft" "video" "MRM-MrmMain" "MRMP-MrmpMain" "MRMP-RtpRx0" "MRMP-RtpRx1" "MRMP-RtpRx2" "MRMP-RtpRx3" "MRMP-RtpTx0" "MRMP-RtpTx1" "MRMP-RtpTx2" "MRMP-RtpTx3") 
#define a struct to combine process and pid as Process:Pid  mpproxy:1343
ProcessStructList=()
LaunchOnlyProcesses=()

retrieveProcessList=("MRM-MrmMain" "MRMP-MrmpMain" "MRMP-RtpRx0" "MRMP-RtpRx1" "MRMP-RtpRx2" "MRMP-RtpRx3" "MRMP-RtpTx0" "MRMP-RtpTx1" "MRMP-RtpTx2" "MRMP-RtpTx3" "mfa" "mp_launcher" "ice_manager_1" "ice_manager_2" "ice_manager_3" "ice_manager_4" "ice_manager_5" "ice_manager_6" "ice_manager_7" "ice_manager_8")
retrieveStructList=()
ConfFile="$MCU_HOME_DIR/mcms/Cfg/MpWatcher.conf"

#hotbackup
HOTBACKUP_ENABLE=NO
HotBackupConfig=$MCU_HOME_DIR/mcms/Cfg/Failover.xml

#NINJA: check CPLD firmware version, whether support reset.
CPLD_SUPPORT_RESET=NO

#New recovery: Video process recovery
VideoProcessRecovery=NO

Echo()
{
	echo $(date) $@ 2>&1 | ${LOG}
}

ReadConfFile()
{
	Echo "in ReadConfFile"
	unset MonitorProcesses
	unset LaunchOnlyProcesses
	local no_monitor=true
	local no_launch_only=true
	EnableItem=`cat ${ConfFile} | grep -v "^#" | grep -v "launch_only" | grep -i "enable"`  ### remove launch_only process from monitor processes ###
	LaunchOnlyItem=`cat ${ConfFile} | grep -v "^#" | grep -i "launch_only"`
	AllHandleProcesses=("$EnableItem" "$LaunchOnlyItem")
	if [ "X" == "X${AllHandleProcesses[0]}" ]
	then
		Echo "There is no process to be handled with"
        	exit 1
	fi
	for ii in ${AllHandleProcesses[@]}
	do
		Echo "$ii" | grep "enable" | grep -v "launch_only"
        	if [ "$?" == "0" ]
        	then
        		no_monitor=false
            		item=`echo "${ii}" | awk -F"=" '{print $1}'`
			MonitorProcesses=("${MonitorProcesses[@]}" "${item}")
        	else
            		no_launch_only=false
            		item=`echo "${ii}" | awk -F"=" '{print $1}'`
            		LaunchOnlyProcesses=("${LaunchOnlyProcesses[@]}" "${item}")
        	fi
    	done
    	if [ $no_monitor == "true" ]
    	then
    		Echo  "There is no Monitor Processes."
    	else
        	Echo "The new Monitor Processes would be ${MonitorProcesses[@]}"
    	fi

    	if [ $no_launch_only == "true" ]
    	then
        	Echo "There is no Launch Only Processes." 
    	else
        	Echo "The new Launch only Processes would be ${LaunchOnlyProcesses[@]}"
    	fi
	Echo "out ReadConfFile"
}

ConfFileStyle()
{
	echo "${MagicNum}=Magic #Do NOT modify this sentence unless you know what you are doing." | tee ${ConfFile} 2>&1 | ${LOG}
	echo "mfa=enable" | tee -a ${ConfFile} 2>&1 | ${LOG}
	echo "mpproxy=enable" | tee -a ${ConfFile} 2>&1 | ${LOG}
	echo "audio_soft=enable" | tee -a ${ConfFile} 2>&1 | ${LOG}
	echo "video=enable" | tee -a ${ConfFile} 2>&1 | ${LOG}
	echo "MRM-MrmMain=enable" | tee -a ${ConfFile} 2>&1 | ${LOG}
	echo "MRMP-MrmpMain=enable" | tee -a ${ConfFile} 2>&1 | ${LOG}
	#monitor all the MRMP's child processes
	echo "MRMP-RtpRx0=enable" | tee -a ${ConfFile} 2>&1 | ${LOG} 
	echo "MRMP-RtpRx1=enable" | tee -a ${ConfFile} 2>&1 | ${LOG} 
	echo "MRMP-RtpRx2=enable" | tee -a ${ConfFile} 2>&1 | ${LOG} 
	echo "MRMP-RtpRx3=enable" | tee -a ${ConfFile} 2>&1 | ${LOG} 
	echo "MRMP-RtpTx0=enable" | tee -a ${ConfFile} 2>&1 | ${LOG} 
	echo "MRMP-RtpTx1=enable" | tee -a ${ConfFile} 2>&1 | ${LOG} 
	echo "MRMP-RtpTx2=enable" | tee -a ${ConfFile} 2>&1 | ${LOG} 
	echo "MRMP-RtpTx3=enable" | tee -a ${ConfFile} 2>&1 | ${LOG} 
}


CreateConfFile()
{
	#local ConfFileFlag="new1A2B3C4D5E1"         ### Modify it from "1A2B3C4D5E" to "new1A2B3C4D5E" is in order to be compatible to old version(From v8.2.0.73) ###
	local ConfFileFlag="${MagicNum}"
	if [ -e ${ConfFile} ]
	then
		Echo "The configuration file ${ConfFile} is existing." 
		if [ "X`head -n 1 ${ConfFile} | grep "Magic" | awk -F"=" '{print $1}'`" == "X${ConfFileFlag}" ]
		then
			Echo "The style of ${ConfFile} is OK" 
		else
			Echo "${ConfFile} is modified under unknow circumstance." 
			/bin/rm -f ${ConfFile} 2>/dev/null
			ConfFileStyle 
		fi
	else
		Echo "The configuration file ${ConfFile} is disappear, configuring now..."
		ConfFileStyle
	fi
}

############## log check #########################
function CheckLog
{
	[ -e ${LOG_NAME} ] || return

	#get log size
	size=`ls -al ${LOG_NAME} | awk -F ' ' '{ print $5 }'`
	Echo "The size of ${LOG_NAME} is ${size}"
	if [ ${size} -gt ${LOG_SIZE} ]
	then
		mv -f ${LOG_NAME} ${LOG_NAME}_$(date +%Y-%m-%d_%H-%M)
	fi  
	log_count=`ls ${LOG_NAME}* | wc -l`
	if [ ${log_count} -gt ${LOG_COUNT} ]
	then
		cd $MCU_HOME_DIR/tmp/startup_logs/WatcherLogs
		log_base=$(basename ${LOG_NAME})*
		ls -lrt ${log_base} | awk -F ' ' '{ print $9}' | head $(( $LOG_COUNT - $(ls -l ${log_base} | wc -l) )) 2>/dev/null | xargs rm  
		cd -
	fi  
}
##################################################

dumpTimeArray()
{
	local index=0
	local FailTime
	Echo "Dump Fail Time array:"
	for FailTime in ${FailTimeArray[@]}
	do
		((index++))
		Echo "${index} at ${FailTime}"
	done
}

insertTimeArray()
{
	local FailTime=$(date +%s)
	FailTimeArray=("${FailTimeArray[@]}" ${FailTime})   #push new element to array
	if [ ${Total_fail} -lt ${#FailTimeArray[@]} ]
	then
		FailTimeArray=(${FailTimeArray[@]:1})           #pop first element from array  ---- shift
	fi
	dumpTimeArray
}

calculateFailTimes()
{
	((Fail_count++))
	Echo " fail ${Fail_count} times"

	if [ ${Fail_count} -gt ${Total_fail} ]      #  fail times > defined times, compare time
	then
		local FailTime=$(date +%s)
		local interval=$(( ${FailTime} - ${FailTimeArray[0]} ))
		if [ ${interval} -gt 3600 ]   ### > 1 hour
		then
			insertTimeArray
		else
			Disconnection_Start
			sleep 2
			Echo "Media Recovery more than 5 times in 1 hour, restart System"
			uptime | ${LOG}
			ls -l $MCU_HOME_DIR/mcms/Cores/ | ${LOG}
			ps -e | ${LOG}
			#sudo /bin/sync
			sync
			if [[ "GESHER" == "${PRODUCT_TYPE}" || "NINJA" == "${PRODUCT_TYPE}" ]]
			then
				sudo reboot
			else
				touch $MCU_HOME_DIR/tmp/stop_watcher_arg
				sudo /sbin/service soft_mcu restart
			fi
			exit
		fi
	else    #fail times <= defined times, record time
		insertTimeArray
	fi
}

setSystemEnv()
{
	Echo "in setSystemEnv"
	Echo "Checking environment ......"
	local Setting=$(ulimit -c)
	if [ "unlimited" != "${Setting}" ]
	then
		Echo "ulimit -c returns ${Setting}, and set it to unlimited" 
		ulimit -c unlimited
	fi
	Setting=$(ulimit -s)
	if [ "10240" != "${Setting}" ]
	then
		Echo "ulimit -s returns ${Setting}, and set it to 10240" 
		ulimit -s 10240
	fi
#	Setting=$(ulimit -n)
#	if [ "4096" != ${Setting} ]
#	then
#		Echo "ulimit -n returns ${Setting}, and set it to 4096"
#		ulimit -n 4096
#	fi

}

getPidFromStruct()
{
	echo "$1" | awk -F ':' '{ print $2 }'
}

getProcessNameFromStruct()
{
	echo "$1" | awk -F ':' '{ print $1 }'
}

launch_mfa()
{
	Echo "in launch_mfa"
	PROCESS=$(ps -e | grep -v grep | grep "\<mfa")
	if [ "" == "${PROCESS}" ]
	then
		Echo "MPMX_DIR=$MPMX_DIR"
		cd $MPMX_DIR
		retrieveMfaExternalIPC
		Echo "STARTING MPMX" 
		(
			 echo "# Generated by GesherStart.sh"
			 echo export SIMULATION=YES     
			 echo export RUN_MCMS=NO        
			 echo export GDB=NO    
			 echo export VALGRIND="NO"
			 echo export DMALLOC_SIM=NO     
			 echo export EFENCE_SIM=NO      
			 echo export TRACE_IPMC_PROTOCOL=NO
			 echo export PLATFORM=RMX2000   
		) > ./mfa_x86_env/cfg/runmfa.export
		(   
			 export LD_LIBRARY_PATH=$MPMX_DIR/mfa_x86_env/bin:$MCU_LIBS
			 ./scripts_x86/runmfa.sh 2>&1 1>/dev/null &
		)&
	else
		Echo "[ERROR] mfa has already launched"
	fi
}

launch_mpproxy()
{
	Echo "in lanuch_mpproxy"
	local ProcessName=$1
	PROCESS=$(ps -e | grep -v grep | grep "\<$ProcessName")
	if [ "" == "${PROCESS}" ]
	then
		$MCU_HOME_DIR/usr/rmx1000/bin/mpproxy &
		PID=$!
		ProcessStructList=("${ProcessStructList[@]}" "${ProcessName}:${PID}")
		Echo "${ProcessName}:${PID}" 
	else
		Echo "[ERROR] ${ProcessName} has already launched"

	fi
}

launch_audio_soft()
{
	Echo "in launch_audio_soft"
	export LD_LIBRARY_PATH=$MCU_HOME_DIR/usr/rmx1000/bin:$MCU_LIBS
	local ProcessName=$1
	PROCESS=$(ps -e | grep -v grep | grep "\<audio_soft")

	if [ "" == "${PROCESS}" ]
	then
		if [[ "$PRODUCT_TYPE" == "SOFT_MCU_MFW" && "$MEMORY_AMOUNT" -gt "$MFWHIGHMEMORY" ]] || [[ "$MEMORY_MODE" == "High" && "$VM" == "YES" ]]; then
			test ! -f $MCU_HOME_DIR/usr/rmx1000/bin/audio_soft.orig && cp $MCU_HOME_DIR/usr/rmx1000/bin/audio_soft $MCU_HOME_DIR/usr/rmx1000/bin/audio_soft.orig
			rm -f $MCU_HOME_DIR/usr/rmx1000/bin/audio_soft
			ln -fs $MCU_HOME_DIR/usr/rmx1000/bin/audio_soft.mfw $MCU_HOME_DIR/usr/rmx1000/bin/audio_soft
		else
			if [ -f $MCU_HOME_DIR/usr/rmx1000/bin/audio_soft.orig ]; then
				rm -f $MCU_HOME_DIR/usr/rmx1000/bin/audio_soft 
				cp -f MCU_HOME_DIR/usr/rmx1000/bin/audio_soft.orig $MCU_HOME_DIR/usr/rmx1000/bin/audio_soft
			fi
		fi
		$MCU_HOME_DIR/usr/rmx1000/bin/audio_soft &
		PID=$!
		ProcessStructList=("${ProcessStructList[@]}" "${ProcessName}:${PID}")
		Echo "${ProcessName}:${PID}"
	else
		Echo "[ERROR] ${ProcessName} has already launched"
	fi
}

launch_video()
{
	Echo "in launch_video"
	export LD_LIBRARY_PATH=$MCU_HOME_DIR/usr/rmx1000/bin:$MCU_LIBS
	local ProcessName=$1
#	PROCESS=$(ps -e | grep -v grep | grep "\<video")
	PROCESS=$(ps -e | grep -v grep | grep -v videomon | grep "\<video")
	if [ "" == "${PROCESS}" ]
	then
		if [ "NINJA" == "${PRODUCT_TYPE}" ]
		then
			sudo $MCU_HOME_DIR/usr/rmx1000/bin/video &>/dev/null &
		else
			$MCU_HOME_DIR/usr/rmx1000/bin/video &
		fi
		#PID=$!  ##this is the sudo pid in NINJA
		#ProcessStructList=("${ProcessStructList[@]}" "${ProcessName}:${PID}")
		#Echo "${ProcessName}:${PID}" 
	else
		Echo "[ERROR] ${ProcessName} has already launched" 
	fi
}

launch_MRM_MrmMain()
{
	PROCESS=$(ps -e | grep -v grep | grep "\<MRM-MrmMain")
	if [ "" == "${PROCESS}" ]
	then
		cd ${ENGINE_DIR}
		Echo "STARTING ENGINEMRM ENGINE_DIR=${ENGINE_DIR}"
		./mrm.sh all &
		cd -
	else
		Echo "[ERROR] EngineMRM has already launched" 
	fi
}

launch_MRMP_MrmpMain()
{
        PROCESS=$(ps -e | grep -v grep | grep "\<MRMP-MrmpMain")                                                                                                       
        if [ "" == "${PROCESS}" ]                                                                                                                                    
        then
        	#ENGINE_DIR="$MCU_HOME_DIR/usr/share/EngineMRM/"                                                                                                       
		cd ${ENGINE_DIR}                                                                                                                             
	        Echo "STARTING ENGINEMRM ENGINE_DIR=${ENGINE_DIR}"                                                                                                                            
		./mrm.sh all &                                                                                                                                       
		 cd -
        else                                                                                                                            
 		Echo "[ERROR] EngineMRM has already launched"                                                                                                        
 	fi
 } 

startAllMps()
{
	ps -e|grep MRM|${LOG}
	Echo "in startAllMps"
	unset ProcessStructList
	unset retrieveStructList
	local ProcessName
	for ProcessName in ${MonitorProcesses[@]}; do
		case "${ProcessName}" in
		MRM-MrmMain)
			launch_MRM_MrmMain ${ProcessName}
			;;
		MRMP-MrmpMain)
			continue
			;;
		MRMP-RtpRx[0-3])
			Echo "startAllMps: ${ProcessName}"
			continue
			;;
		MRMP-RtpTx[0-3])
			continue
			;;
		*)
			launch_${ProcessName} ${ProcessName}
			;;
		esac
	done
	Echo "out startAllMps"
}

startLaunchOnlyps()
{
	Echo "in startLanchOnlys"
	local ProcessName
	for ProcessName in ${LaunchOnlyProcesses[@]}
	do
		case "$ProcessName" in
		MRM-MrmMain)
                        launch_MRM_MrmMain ${ProcessName}
                        ;;
		*)
			launch_${ProcessName} ${ProcessName}
			;;
		esac
	done
}

getRestPids()
{
	if [[ ${#MonitorProcesses[@]} == ${#ProcessStructList[@]} ]]
	then 
		return
	fi
	local ProcessName
	for ProcessName in ${MonitorProcesses[@]};
	do
		local ProcessStruct
		local NeedGetPid=true
		for ProcessStruct in ${ProcessStructList[@]};
		do
			local Pid=${ProcessStruct##${ProcessName}:}
			if [[ "$ProcessStruct" != ${Pid} ]]
			then
				NeedGetPid=false
				break
			fi
		done
		if [[ "true" == "${NeedGetPid}" ]]
		then	
			#Echo "getRestPids: Geting ${ProcessName}'s pid" 
			PID=$(ps -e | grep -v defunct | grep -v grep | grep "\<${ProcessName}" | awk -F ' ' '{print $1}')
			if [ "" != "${PID}" ]
			then
				Echo "getRestPids: ${ProcessName}:${PID}"
				ProcessStructList=("${ProcessStructList[@]}" "${ProcessName}:${PID}")
				#Echo "getRestPids: ProcessStructList=${ProcessStructList[@]}"
			fi

		fi
	done
	Echo "getRestPids: ProcessStructList=${ProcessStructList[@]}" 
	
	if [[ ${#MonitorProcesses[@]} != ${#ProcessStructList[@]} ]]
	then
		CurrentTime=$(( ${CurrentTime} + ${IntervalTime} ))
		Echo "[Warning] Some processes didn't still launch  in ${CurrentTime} sec" 
		if [ ${CurrentTime} -gt ${StartupTime} ]
		then
			Echo "[ERROR] fail to launch and restart service"
			if [[ "$PRODUCT_TYPE" == "GESHER" || "${PRODUCT_TYPE}" == "NINJA" ]]
			then
				sudo /etc/rc.d/99soft_mcu restart
			else
				touch $MCU_HOME_DIR/tmp/stop_watcher_arg
				sudo /sbin/service soft_mcu restart
			fi
		fi
	fi
	
	#get all processes pid, which are needed to retrieve share-memory
	for ProcessName in ${retrieveProcessList[@]};
	do
		local ProcessStruct                                                                                      
		local NeedGetPid=true                                                                                    
		for ProcessStruct in ${retrieveStructList[@]};                                                            
		do
			local Pid=${ProcessStruct##${ProcessName}:}
			if [[ "$ProcessStruct" != ${Pid} ]]                                                              
			then                                                                                    
				NeedGetPid=false                                                                         
				break                                                                                    
			fi                                                                                             
		done 	
	
	
		if [ "true" == "$NeedGetPid" ]
		then	
			local PID
			PID=$(ps -e | grep -v defunct | grep -v grep | grep "\<${ProcessName}" | awk -F ' ' '{print $1}')
			if [ "" != "${PID}" ]
			then
				retrieveStructList=("${retrieveStructList[@]}" "${ProcessName}:${PID}")
			fi
		fi
	done

	for ProcessName in ${MonitorProcesses[@]};
	do
		ps -e |grep -w ${ProcessName} | ${LOG}
	done
	Echo "getRestPids End: retrieveStructList=${retrieveStructList[@]}"
}

waitall() 
{
	Echo "------------- in waitall --------------"
	## Wait for children to exit and indicate whether all exited with 0 status.
	local mp_crash=false
	local Ret
	while :; do
		getRestPids

		local ProcessStruct
		for ProcessStruct in ${ProcessStructList[@]}; do
			local processid=$(getPidFromStruct ${ProcessStruct})
			local name=$(getProcessNameFromStruct ${ProcessStruct})
			#Ret=$( ps -A | grep -v grep | grep ${name} | grep -w ${processid} | grep -v -e '^[Zz]' )
			Ret=$( ps -A | grep -v defunct | grep ${name} | grep -w ${processid})
			if [ "" == "${Ret}" ]
			then	
				ps -ef | grep $name | ${LOG}
				pstree | ${LOG}
				Echo "processid=$processid not exist."	
				wait $processid
				Echo "${name} $processid exited with $? exit status."
				mp_crash=true
				break
			fi
		done
		if [ true == $mp_crash ]
		then 
			break
		fi
		# TODO: how to interrupt this sleep when a child terminates?
		sleep ${IntervalTime}
	done
	Echo "---------------- out waitall -----------------"
}

retrieveMessageQueue()
{
	local pid=$1
	local MessageQueues=$(ipcs -q -p | grep ${pid} | awk -F ' ' '{ print $1 }')
	for id in ${MessageQueues}
	do
		Echo "retrieving message queue id=${id} for pid=${pid}"
		ipcrm -q ${id} 2>&1 | ${LOG}
	done
}

retrieveSharedMemories()
{
	local pid=$1
	local SharedMemories=$(ipcs -m -p | grep ${pid} | awk -F ' ' '{ print $1 }')
	for id in ${SharedMemories}
	do
		Echo "retrieving shared memory id=${id} for pid=${pid}"
		ipcrm -m ${id} 2>&1 | ${LOG}
	done
}

retrieveMfaExternalIPC()
{
	local PIDS=`ipcs -p -m |grep '^[0-9]'| awk '{print $3}'`
	for p in $PIDS ;
	do
		EXIST=`ps -ef | grep $p | sed '/grep/d'` ;
		if [ -z "$EXIST" ]
		then
			IDS=`ipcs  -m  | grep $p | awk '{print $1}'`
			for i in $IDS ;
			do 
				Echo "ipcrm -m $i"
				ipcrm -m $i 
			done
			IDS=`ipcs  -q  | grep $p | awk '{print $1}'`
			for i in $IDS ; 
			do 
				Echo "ipcrm -q $i"
				ipcrm -q $i 
			done
		fi
	done
	USER=`whoami`
	MFA_DIR=$MCU_HOME_DIR/tmp/$USER/mfa

	#INODE=`ls -di $MCU_HOME_DIR/usr/share/MFA/MPMX/mfa_x86_env/ | awk '{print $1}'`
	INODE=`ls -di $MCU_HOME_DIR/usr/share/MFA/mfa_x86_env/ | awk '{print $1}'`
	MFA_Q_DIR=$MFA_DIR/$INODE/ls_msg_q
	MQIDS=`ls $MFA_Q_DIR`
	Echo "removing mfa queues(${MQIDS})" 
	for p in $MQIDS ;
	do
		echo ipcrm -q $p 2>&1 | ${LOG}
		ipcrm -q $p
		echo rm $MFA_Q_DIR/$p 2>&1 | ${LOG}
		rm $MFA_Q_DIR/$p

	done

	MQIDS=`ls $MCU_HOME_DIR/tmp/$USER/ipmc_sim/$INODE/ls_msg_q`
	Echo "removing ipmc_sim queues(${MQIDS})" 
	for p in $MQIDS ;
	do
		echo ipcrm -q $p 2>&1 | ${LOG}
		ipcrm -q $p
		echo rm $MCU_HOME_DIR/tmp/$USER/ipmc_sim/$INODE/ls_msg_q/$p 2>&1 | ${LOG}
		rm $MCU_HOME_DIR/tmp/$USER/ipmc_sim/$INODE/ls_msg_q/$p

	done
}
killChildProcesses()
{	
	Echo "killChildProcesses: kill all the mfa and audio's child processes by anme."
	#sudo /usr/bin/killall
	killall -9 mp_launcher ice_manager_1 ice_manager_2 ice_manager_3 ice_manager_4 ice_manager_5 ice_manager_6 ice_manager_7 ice_manager_8
	killall -9 ASS-AH AMP-AmpAHAc AMP-AmpAHDec0 AMP-AmpAHDec1 AMP-AmpAHDec2 AMP-AmpAHDec3 AMP-AmpAHDec4 AMP-AmpAHDec5 AMP-AmpAHDec6 AMP-AmpAHDec7 AMP-AmpAHDec8 AMP-AmpAHDec9
	killall -9 AMP-AmpAHEnc0 AMP-AmpAHEnc1 AMP-AmpAHEnc2 AMP-AmpAHEnc3 AMP-AmpAHEnc4 AMP-AmpAHEnc5 AMP-AmpAHEnc6 AMP-AmpAHEnc7 AMP-AmpAHEnc8 AMP-AmpAHEnc9 AMP-AmpAHFp
	killall -9 AMP-AmpAHIvr0 AMP-AmpAHIvr1 AMP-AmpAHIvr2 AMP-AmpAHIvr3 AMP-AmpAHIvr4 AMP-AmpAHIvr5 AMP-AmpAHIvr6 AMP-AmpAHIvr7 AMP-AmpAHIvr8 AMP-AmpAHIvr9 AMP-AmpAHMgr
	killall -9 ASS-AMPLog ASS-AMPMgr ASS-AMPUdpRx0 ASS-AMPUdpRx1 ASS-AMPUdpRx2 ASS-AMPUdpRx3 ASS-AMPUdpRx4
	killall -9 ASS-AMPUdpTx0 ASS-AMPUdpTx1 ASS-AMPUdpTx2 ASS-AMPUdpTx3 ASS-AMPUdpTx4 ASS-AMPUdpTx5 ASS-AMPUdpTx6 ASS-AMPUdpTx7 ASS-AMPUdpTx8 ASS-AMPUdpTx9
	killall -9 AMP-AmpAHSac0 AMP-AmpAHSac1 AMP-AmpAHSac2 AMP-AmpAHSac3 AMP-AmpAHSac4 AMP-AmpAHSac5 AMP-AmpAHSac6 AMP-AmpAHSac7 AMP-AmpAHSac8 AMP-AmpAHSac9
}

stopAllMps()
{
	Echo "in stop Mp related processes" 
	Echo $(ps -ef)
	Echo "Check core files before killing process : "
	Echo $(ls -l ${MCU_HOME_DIR}/mcms/Cores/)
	#sudo /usr/bin/killall -2 IpmcSim.x86 LinuxSysCallPro 2>/dev/null
	#sudo /usr/bin/killall -2 MRM-MrmMain 2>/dev/null
	killall -2 IpmcSim.x86 LinuxSysCallPro 2>/dev/null
	killall -2 MRM-MrmMain 2>/dev/null
	kill -9 `pgrep MRM` `pgrep mrm` 2>/dev/null
	sleep 1
	
	local ProcessStruct
	for ProcessStruct in ${ProcessStructList[@]}
	do
		local processname=$(getProcessNameFromStruct ${ProcessStruct})
	
		Echo "It will killall -2 ${processname}"
		#sudo /usr/bin/killall -2 ${processname} 2>/dev/null
		
		if [ "video" == "$processname" -a "${PRODUCT_TYPE}" == "NINJA" ]
		then
			sudo /usr/bin/killall -2 ${processname} 2>/dev/null
		else
			killall -2 ${processname} 2>/dev/null
		fi

	done
	
	sleep 5
	killChildProcesses
	#killall -9 mp_launcher ice_manager_1 ice_manager_2 ice_manager_3 ice_manager_4 ice_manager_5 ice_manager_6 ice_manager_7 ice_manager_8

	#monitor how long the process take to exit
	local Time2Exit=0
	local Process2KillList=(${ProcessStructList[@]})
	local index=0
	while [ 0 -lt ${#Process2KillList[@]} ]; 
	do
		index=0
		local isProcessExit=false
		for ProcessStruct in ${Process2KillList[@]}
		do
			local Process=$(getProcessNameFromStruct ${ProcessStruct})

			if [ "$Process" == "video" ]
			then
				local pid=$(ps -ef | grep -v grep | grep video$ | awk -F ' ' '{ print $2}')
			else
				local pid=$(ps -ef | grep -v grep | grep ${Process} | awk -F ' ' '{ print $2}')
			fi

			if [[ "" == "${pid}" ]]
			then
				Echo "${ProcessStruct} have exited at ${Time2Exit}s" 
				Process2KillList=(${Process2KillList[@]:0:${index}} ${Process2KillList[@]:$((${index}+1))})
				isProcessExit=true
				break
			elif [ 20 -lt ${Time2Exit} ]
			then
				Echo "There are still ${Process2KillList[@]} which not exit in ${Time2Exit}s"
				#sudo /usr/bin/killall -9 ${Process}
				if [ "$Process" == "video" -a "${PRODUCT_TYPE}" == "NINJA" ]
				then
					sudo /usr/bin/killall -9 ${Process}
				else
					killall -9 ${Process}
				fi
			fi
			((index++))
		done
		if [ "true" == "${isProcessExit}" ]
		then
			continue
		fi
		sleep 1
		((Time2Exit++))
	done

	#sudo /bin/kill -9 `pgrep LinuxSysCallPro`
	#sudo /bin/kill -9 `pgrep IpmcSim.x86`
	kill -9 `pgrep LinuxSysCallPro`
	kill -9 `pgrep IpmcSim.x86`
	
	#for debug info
    for ProcessName in ${MonitorProcesses[@]};                                                                                       
	do                                                                                                                               
		 ps -e |grep -w ${ProcessName} | ${LOG}                                                                                   
	done
	Echo "After all media related process exited."
	Echo $(ps -ef)
	Echo "Check core files after killing process : "
	Echo $(ls -l ${MCU_HOME_DIR}/mcms/Cores/)
	sleep 2	
	Echo "Finally, processes ${#retrieveStructList[@]} will be retrieved"
	for ProcessStruct in ${retrieveStructList[@]}
	do		
		local pid=$(getPidFromStruct ${ProcessStruct})
		retrieveMessageQueue $pid
		retrieveSharedMemories $pid
		wait "$pid" 2>/dev/null
		Echo "retrieve ${ProcessStruct}"
	done
	
	CurrentTime=0
	Echo "out stopAllMps"
}

errorTrap()
{
	Echo "[FUCTION:$1(LINE:$2)] Error: Command or function exited with status $?" 
}

#HotBackup support check
Check_HotBackup()
{
	Echo "in Check_HotBackup"
	if [ "NINJA" == "$PRODUCT_TYPE" ]
	then
		if [ ! -e ${HotBackupConfig} ]
		then
			HOTBACKUP_ENABLE=NO
		else
			HOTBACKUP_STATUS=`cat ${HotBackupConfig} |grep "HOTBACKUP_ENABLED" | awk -F'<|>' '{if(NF>3) {print $3} }'`
			Echo "Check_HotBackup: HOTBACKUP_STATUS=$HOTBACKUP_STATUS"
			if [ "true" == "$HOTBACKUP_STATUS" ]
			then
				HOTBACKUP_ENABLE=YES
			else
				HOTBACKUP_ENABLE=NO
			fi
		fi
	else
		Echo "PRODUCT_TYPE=${PRODUCT_TYPE}, no need Check HotBackup."
		return
	fi
	Echo "out Check_HotBackup, HOTBACKUP_ENABLE=${HOTBACKUP_ENABLE}"
}

#CPLD_Version support reset check
Check_CPLD_ResetSupport()
{
	Echo "in Check_CPLD_Version"
	if [ "NINJA" == "$PRODUCT_TYPE" ]
	then
		#fpga_version=$(echo `sudo /usr/rmx/bin/fpga_test 0x400 | awk -F '0x' '{print $3}'`)
		fpga_version=$(echo `sudo /usr/rmx/bin/cpld_version`)
		Echo "Check_CPLD_ResetSupport: fpga_version=${fpga_version}"
		if [[ "$fpga_version" > "0000000b" ]]
		then
			CPLD_SUPPORT_RESET=YES
		else
			CPLD_SUPPORT_RESET=NO
		fi
	else
		Echo "Check_CPLD_Version, PRODUCT_TYPE=${PRODUCT_TYPE}, no need check."
		return
	fi
	Echo "out Check_CPLD_Version, CPLD_SUPPORT_RESET=${CPLD_SUPPORT_RESET}"
}

#PowerOff all DSP, Ninja
DSP_PowerOff()
{
	Echo "in DSP_PowerOff"
	if [ "YES" == "${VideoProcessRecovery}" ]
	then
		for i in `seq 0 17`                                  
 		do
 			veiX=$(echo `ifconfig | grep vei$i`)
 			if [ "" != "$veiX" ]
 			then
 				#telnet dsp and poweroff                
 				Echo "DSP_PowerOff: i=$i --------------------->poweroff"
 				(sleep 0.1;echo root;echo;echo "uptime;poweroff";sleep 0.1; echo exit) | telnet 169.254.$i.10
 			else
 				Echo "dsp $i not exist veiX=$veiX"
 			fi
		done 
	else
		Echo "DSP_PowerOff: no need PowerOff DSP."
	fi
}

#Reload FPGA, Ninja
FPGA_Reload()
{	
	Echo "in FPGA_Reload"
	if [ "YES" == "${VideoProcessRecovery}" ]
	then
		PROCESS=$(ps -e | grep -v grep | grep -v videomon | grep "\<video")
		Echo "FPGA_Reload: PROCESS=${PROCESS}"	

		sudo /usr/rmx/bin/fpga_recov
		reload_status=$?
		Echo "FPGA_Reload: sudo /usr/rmx/bin/fpga_recov --> reload_status=${reload_status}"
		if [ ${reload_status} -ne 0 ]
		then
			Echo "FPGA_Reload: Failed, reboot."
			sudo reboot
		fi
	else
		Echo "FPGA_Reload: no need Reload FPGA."
	fi
}

#disconnection all participants Start
Disconnection_Start()
{
	Echo "Disconnection_Start: disconnection all participants, Display Alarm on EMA."
	export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin:$MCU_LIBS
	$MCU_HOME_DIR/mcms/Bin/McuCmd video_recovery ConfParty start
	Echo "Disconnection_Start: return $? status"
}

#disconnection all participants Over
Disconnection_Over()
{
	Echo "Disconnection_Over: disconnection over, remove Alarm from EMA."
	export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin:$MCU_LIBS
	$MCU_HOME_DIR/mcms/Bin/McuCmd video_recovery ConfParty over
	Echo "Disconnection_Over: return $? status"
}

#VideoRecovery
VideoRecovery()
{
	Echo "VideoRecovery in, disconnect all participants, PRODUCT_TYPE=${PRODUCT_TYPE}"
	Disconnection_Start
	
	if [ "NINJA" == "${PRODUCT_TYPE}" ]
	then
		Check_HotBackup
		Echo "PRODUCT_TYPE=${PRODUCT_TYPE}, HOTBACKUP=${HOTBACKUP_ENABLE}, CPLD_RESET=${CPLD_SUPPORT_RESET}"
		if [ "YES" == "$HOTBACKUP_ENABLE" ]
		then
			#hotbackup enabled, all scenarios: reboot.
			sleep 2
			Echo "VideoRecovery: hotbackup enabled, reboot"
			sudo reboot
		else
			if [ "YES" == "$CPLD_SUPPORT_RESET" ]
			then    
				sudo /usr/bin/killall -2 video	
				while :; do
					videoPs=$(ps -e | grep -w video)
					Echo "VideoRecovery: wait video=${videoPs} exit."
					if [ "" == "${videoPs}" ]
					then
						break
					fi
					sleep 1
				done
				
				videoProcess=$(ps -e|grep -w video)
				Echo "VideoRecovery: videoProcess=${videoPid}" 
				
				Echo "VideoRecovery: CPLD support reset, video recovery process."
				VideoProcessRecovery=YES
				DSP_PowerOff
			else
			    	sleep 2
				Echo "VideoRecovery: old equipment, reboot."
				sudo reboot
			fi
		fi
	fi
	Echo "out VideoRecovery"
}

StartAllMps_Over()
{
	Echo "StartAllMps_Over in, wait MFA startup end"
	VideoProcessRecovery=NO                                       

	if [ "NINJA" == "$PRODUCT_TYPE" -o "GESHER" == "$PRODUCT_TYPE" ]
	then	
		#wait mfa startup over
		while :; do
			TCPrcvThread=$(pstree | grep TCPrcvThread)
			if [ "" != "${TCPrcvThread}" ] 
			then
				Echo "StartAllMps_Over: MFA startup success. ${TCPrcvThread}"
				break
			fi
			sleep 1
		done
	else
		sleep 20
	fi
	                                                                                                                         
	Disconnection_Over
}


Echo ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> MpWathcer.sh"
trap 'ERRTRAP $FUCNAME $LINENO' ERR
Check_CPLD_ResetSupport
CreateConfFile
ReadConfFile
startLaunchOnlyps
while :; do
	setSystemEnv
	CheckLog
	FPGA_Reload
	startAllMps
	StartAllMps_Over
	if [ -e $MCU_HOME_DIR/tmp/stop_monitor ]
	then
		Echo "$MCU_HOME_DIR/tmp/stop_monitor: MpWatcher.sh exit, no monitor."
		exit 0
	fi
	
	if [[ -e "$MCU_HOME_DIR/output/MCMSDEBUG" || -e "$MCU_HOME_DIR/tmp/MCMSDEBUG" ]] 
	then	
		Echo "MCMSDEBUG: MpWatcher.sh exit, no monitor."
		exit 0
	fi
	
	sleep ${IntervalTime}
	waitall 
	calculateFailTimes
	VideoRecovery
	stopAllMps
done



