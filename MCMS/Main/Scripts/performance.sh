#!/bin/bash 

COLOR="On"
if [[ "$1" == "C" ]]; then
        COLOR="Off"
else
        COLOR="On"
fi

COLORIZE () {
        if [[ "X$COLOR" == "X" || "$COLOR" == "Off" ]]; then
                return
        fi
        case $1 in
        NORMAL)
                COL="\033[0m"
                ;;
        UNDERLINE)
                COL="\e[4m"
                ;;
        WHITE)
                COL="\033[1;28m"
                ;;
        RED)
                COL="\033[0;31m"
                ;;
        GREEN)
                COL="\033[0;32m"
                ;;
        YELLOW)
                COL="\033[0;33m"
                ;;
        BLUE)
                COL="\033[0;34m"
                ;;
        PURPLE)
                COL="\033[0;35m"
                ;;
        L_BLUE)
                COL="\033[0;36m"
                ;;
        esac
        echo -n -e "$COL"
}

UNCOLORIZE () {
        if [[ "X$COLOR" == "X" ]]; then
                return
        fi
        echo -n -e "\033[0m"
}
    
Process_List="./bin/acloader -c -C;\
Bin/Auditor 0;\
Bin/Authentication 0;\
Bin/BackupRestore 0;\
./bin/calltask -Tct1;\
./bin/calltask -Tct2;\
Bin/Cards 0;\
Bin/CDR 0;\
Bin/CertMngr 0;\
Bin/Collector 0;\
Bin/Configurator 0;\
Bin/ConfParty 0;\
Bin/CSApi 0;\
./bin/csman -Tcsman -Mcsman;\
Bin/CSMngr 0;\
Bin/DNSAgent 0;\
Bin/EncryptionKeyServer 0;\
Bin/ExchangeModule 0;\
Bin/Failover 0;\
Bin/Faults 0;\
Bin/Gatekeeper 0;\
./bin/gkiftask -Tgkiftask -Mgkiftask;\
./bin/h323LoadBalancer -Tlb323 -Mlb323;\
./bin/mp_launcher 1 -x86;\
./bin/LinuxSysCallProcess;\
Bin/Logger 0;\
Bin/MCCFMngr 0;\
Bin/McmsDaemon;\
./bin/mcmsif -Tmcmsif -Mmcmsif;\
Bin/McuMngr 0;\
./bin/mfa;\
Bin/MplApi 0;\
./Bin/mrmTargetBinFile all;\
Bin/NotificationMngr 0;\
Bin/QAAPI 0;\
Bin/Resource 0;\
Bin/RtmIsdnMngr 0;\
/bin/sh ./mrm.sh all;\
/bin/sh ./scripts_x86/runmfa.sh;\
Bin/SipProxy 0;\
./bin/siptask -Tsiptask -Msiptask;\
Bin/SNMPProcess 0;\
Bin/SystemMonitoring 0;\
Bin/Utility 0;\
$MCU_HOME_DIR/usr/rmx1000/bin/audio_soft;\
$MCU_HOME_DIR/usr/rmx1000/bin/launcher;\
/mpproxy;\
$MCU_HOME_DIR/usr/rmx1000/bin/traced;\
$MCU_HOME_DIR/usr/rmx1000/bin/video"

Process_Array=$(echo $Process_List)

DATA_FILE="$MCU_HOME_DIR/tmp/perf.csv"

#########################
#   RAM			#
#########################
USED_RAM=$(free | grep cache: | awk '{ print $3 }' )
RAM_SIZE=$(free | grep Mem: | awk '{ print $2 }' )
USED_RAM_PERCENT=$(( $USED_RAM*100/$RAM_SIZE ))

#########################
#   Shared Memory       #
#########################
LIST_SHMEM_SEG_SIZES=$(ipcs -m | grep mcms | tr -s ' ' | cut -d ' ' -f5 )
USED_SHMEM_SIZE=0; 
for i in $LIST_SHMEM_SEG_SIZES; do 
	USED_SHMEM_SIZE=$(( $USED_SHMEM_SIZE+$i))
done; 
#USED_SHMEM_SIZE=$(( USED_SHMEM_SIZE/1024 ))
MAX_SHMEM_SIZE=$(cat /proc/sys/kernel/shmmax)
USED_SHMEM_SIZE_PERCENT=$(( $USED_SHMEM_SIZE*100/$MAX_SHMEM_SIZE ))

USED_SHMEM_SEG=$(ipcs -um | grep "segments allocated" | cut -d' ' -f3)
MAX_SHMEM_SEG=$(cat /proc/sys/kernel/shmmni)
USED_SHMEM_SEG_PERCENT=$(( $USED_SHMEM_SEG*100/$MAX_SHMEM_SEG ))

#########################
#   Semaphores          #
#########################
USED_SEMAPHORES=$(ipcs -us | grep "used arrays" | awk '{ print $4 }')
MAX_SEMAPHORES=$(cat /proc/sys/kernel/sem | awk '{ print $1 }')
USED_SEMAPHORES_PERCENT=$(( USED_SEMAPHORES*100/MAX_SEMAPHORES ))

#########################
#   Queues              #
#########################
USED_Q=$(ipcs -uq | grep "allocated queues" | awk '{ print $4 }')
MAX_Q=$(cat /proc/sys/kernel/msgmni)
USED_Q_PERCENT=$(( USED_Q*100/MAX_Q ))


NUM_OF_PROCS=$(ps H -u mcms | wc -l)
$(uptime | cut -d':' -f5)

$(./Scripts/TotalNumParties.py &> /dev/null)
NUM_OF_PARTIES=$?
echo "NUM_OF_PARTIES=${NUM_OF_CONFERENCES}"

#General system info
echo "General system info"
echo "==================="
echo "Used RAM: ${USED_RAM} / ${RAM_SIZE} (${USED_RAM_PERCENT}%)"
echo "Used shm size: ${USED_SHMEM_SIZE} / ${MAX_SHMEM_SIZE} (${USED_SHMEM_SIZE_PERCENT}%)"
echo "Used shm segments: ${USED_SHMEM_SEG} / ${MAX_SHMEM_SEG} (${USED_SHMEM_SEG_PERCENT}%)"
echo "Used Semaphores: ${USED_SEMAPHORES} / ${MAX_SEMAPHORES} (${USED_SEMAPHORES_PERCENT}%)"
echo "Used Queues: ${USED_Q} / ${MAX_Q} (${USED_Q_PERCENT}%)"
echo "Load Avg: $(uptime | cut -d':' -f5)" 
echo

#'mcms' user account related data
echo "'mcms' user account related data:"
echo "==================================="

su - mcms -c "echo \"Number of open file descriptors:\" \$(lsof | wc -l) "
su - mcms -c "echo \"Soft/Hard FD limit:  \$(ulimit -Sn) / \$(ulimit -Hn) \" "

echo "Number of running processes: ${NUM_OF_PROCS} "
su - mcms -c "echo \"Soft/Hard #processes limit:  \$(ulimit -Su) / \$(ulimit -Hu) \" "

echo 


#Enter data to file
if [ -f $DATA_FILE ];then
	echo ${NUM_OF_PARTIES},${USED_RAM},${NUM_OF_PROCS} >> $DATA_FILE
else
	touch $DATA_FILE
	echo "Parties,RAM,Processes" > $DATA_FILE
	echo ${NUM_OF_PARTIES},${USED_RAM},${NUM_OF_PROCS} >> $DATA_FILE
fi
exit 4

#Print processes sorted by %CPU
ps aux | head -1
echo "==== Processes sorted by %CPU ===="
ps aux | grep mcms | sort -k3,3 -r -n | head
echo "==== Processes sorted by %MEM ===="
ps aux | grep mcms | sort -k4,4 -r -n | head
echo "==== Processes sorted by %VSZ ===="
ps aux | grep mcms | sort -k5,5 -r -n | head
echo "==== Processes sorted by %RSS ===="
ps aux | grep mcms | sort -k6,6 -r -n | head





#for Process in $Process_Array; do
#	$(ps -ef | grep Process)
#done

