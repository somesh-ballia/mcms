#!/bin/bash 

function log 
{
	if [ ! -f "$LOG_FOLDER$LOG_FILE" ]; then
        	mkdir $LOG_FOLDER
	fi
	echo $1
	echo $1 >> $LOG_FOLDER$LOG_FILE
}

function pause()
{
   read -p "$*"
}

function Wait 
{
	timeToWait=$1
	for((i=1; i<=$timeToWait; i++))
	do
		sleep 1
		echo -n "."
	done
	echo ""
}

function EndScript 
{ 
	log "Script_Exit"
}

INI_FILE=StartSoft.ini
LOG_FOLDER=/tmp/Smokelogs/
LOG_FILE=upgradeLog
LOG_STARTUP_FILE=startupLog
LOG_STATUS_FILE=statusLog

#delete log files
rm -rf $LOG_FOLDER

log "Script_Start"

log "arg 1 (OFFICIAL_DIR)= $1"
if [ -z "${7}" ] ; then
	log "arg 2 (load_timeout) = $2"
else
	log "arg 2 (MCMS_DIR) = $2"
	log "arg 3 (CS_DIR) = $3"
	log "arg 4 (ENGINE_DIR) = $4"
	log "arg 5 (MRMX_DIR) = $5"
	log "arg 6 (MPMX_DIR) = $6"
	log "arg 7 (load_timeout) = $7"
fi
log " "

########################################################### Vars setup ###########################################################
export OFFICIAL_DIR="$1"
if [ -z "${7}" ] ; then	
	MCMS_DIR="NULL"
	CS_DIR="NULL"		
	ENGINE_DIR="NULL"
	MRMX_DIR="NULL"
	MPMX_DIR="NULL"
	load_timeout="$2"
else	
	MCMS_DIR="$2"
	CS_DIR="$3"
	ENGINE_DIR="$4"
	MRMX_DIR="$5"
	MPMX_DIR="$6"
	load_timeout="$7"	
fi

scriptPath="Scripts/soft.sh"
StatusScriptPath="Scripts/SoftMcuStatus.sh"

if [ ${MCMS_DIR:="NULL"} == "NULL" ] ; then
	lastChar="${OFFICIAL_DIR:${#OFFICIAL_DIR}-1}"
	if [ "$lastChar" == "/" ]   ; then
		scriptPath="mcms/$scriptPath"
		StatusScriptPath="mcms/$StatusScriptPath"
	else
		scriptPath="/mcms/$scriptPath"
		StatusScriptPath="/mcms/$StatusScriptPath"
	fi

	status_Script_path=$OFFICIAL_DIR$StatusScriptPath
	soft_sh_path=$OFFICIAL_DIR$scriptPath
else
	lastChar="${MCMS_DIR:${#MCMS_DIR}-1}"
	if [ "$lastChar" != "/" ]   ; then
		scriptPath="/$scriptPath"
		StatusScriptPath="/$StatusScriptPath"
	fi

	status_Script_path=$MCMS_DIR$StatusScriptPath
	soft_sh_path=$MCMS_DIR$scriptPath
fi

cd $OFFICIAL_DIR/mcms
make active
cd -

log "############################## Using scripts ##############################"
log "soft.sh path is: "$soft_sh_path
log "will run on official dir: "$OFFICIAL_DIR
log "SoftMcuStatus.sh path is: "$status_Script_path
log "###########################################################################"

if [ -s $soft_sh_path ] ; then
	log "soft.sh Script Exists at path"
else
	log "soft.sh Script doesn't Exists at path"
	EndScript
	exit 1
fi

####################################### Export Components (ini file) #######################################
log " "

if [ -z "${7}" ] ; then	
	log "Look for ini file at path: "$INI_FILE
	if [ -s $INI_FILE ] ; then
		log "ini file Exists at path: "$INI_FILE

		#read ini file vars
		cp $INI_FILE $INI_FILE.tmp
		dos2unix $INI_FILE.tmp
		. $INI_FILE.tmp	
	else
		log "ini file doesn't Exists at path: "$INI_FILE
		EndScript
		exit 1
	fi
fi

log "#################### init vars #####################"
log "Test will run with components: "

a=0

if [[ "$MCMS_DIR" == *"NULL"* ]] ; then
	unset MCMS_DIR
else		
	if [[ $MCMS_DIR == \/home\/* ]] ; then
		log "PCTC user is running S.MCU."
		log "PCTC user MCMS_DIR="$MCMS_DIR
		newPath="/net/172.21.111.63/vol/vol4/"
  		tmpMCMS_DIR=${MCMS_DIR/\/home\//$newPath}
		log "check if directory exist - "$tmpMCMS_DIR
  		if [ ! -d "$tmpMCMS_DIR" ]; then
			log "directory dosent exist - "$tmpMCMS_DIR
			newPath="/net/172.21.111.63/vol/vol2/"
  			tmpMCMS_DIR=${MCMS_DIR/\/home\//$newPath}
			log "check if directory exist - "$tmpMCMS_DIR		
  			if [ ! -d "$tmpMCMS_DIR" ]; then
				log "directory dosent exist - "$tmpMCMS_DIR
			else
				log "directory exist - "$tmpMCMS_DIR
				MCMS_DIR=$tmpMCMS_DIR	
			fi
		else
			log "directory exist - "$tmpMCMS_DIR
			MCMS_DIR=$tmpMCMS_DIR
		fi	
	fi	

	log "MCMS_DIR="$MCMS_DIR
	export MCMS_DIR=$MCMS_DIR
	sleep 1
	(( a=a+1 ))
fi

if [[ "$CS_DIR" == *"NULL"* ]] ; then
	unset CS_DIR
else
	log "CS_DIR="$CS_DIR		
	export CS_DIR=$CS_DIR
	sleep 1
	(( a=a+1 ))
fi

if [[ "$MPMX_DIR" == *"NULL"* ]] ; then
	unset MPMX_DIR
else
	log "MPMX_DIR="$MPMX_DIR	
	export MPMX_DIR=$MPMX_DIR
	sleep 1
	(( a=a+1 ))
fi

if [[ "$MRMX_DIR" == *"NULL"* ]] ; then
	unset MRMX_DIR
else
	log "MRMX_DIR="$MRMX_DIR
	export MRMX_DIR=$MRMX_DIR
	sleep 1
	(( a=a+1 ))
fi

if [[ "$ENGINE_DIR" == *"NULL"* ]] ; then
	unset ENGINE_DIR
else
	log "ENGINE_DIR="$ENGINE_DIR	
	export ENGINE_DIR=$ENGINE_DIR
	sleep 1
	(( a=a+1 ))
fi

if [ $a -eq 0 ] ; then
	log "All components from Official steam: "$OFFICIAL_DIR
fi
log "#####################################################"

####################################### Starting SWMCU #######################################
log " "
log "Starting S.MCU..."

$soft_sh_path "start_vm_edge" &> $LOG_FOLDER$LOG_STARTUP_FILE &

Wait "($load_timeout-2)" &
Wait_ps=$!

timeout $load_timeout $status_Script_path LOOP &> $LOG_FOLDER$LOG_STATUS_FILE
timeoutRES=$?
$status_Script_path  &> $LOG_FOLDER$LOG_STATUS_FILE

log " "
log "Results: $timeoutRES"
log " "

kill $Wait_ps

if [[ "$timeoutRES" == "124" ]] ;then
	log "Soft MCU didn't run"
	$soft_sh_path "stop"
	EndScript
	exit 1
fi

log "Soft MCU is up and running"
EndScript
exit 0
