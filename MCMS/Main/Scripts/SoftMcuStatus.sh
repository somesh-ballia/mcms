#!/bin/bash
#
# Process List
# ------------
# Check the Soft MCU run status
# Written by: Shachar Bar/Ori Pugatzky
#

#if [[ `whoami` == 'root' ]]; then
#	echo "You can't run $0 as root"
#	exit 1
#fi

export MCU_HOME_DIR=
export MCU_HOME_DIR=$(grep MCU_HOME_DIR /home/mcms/.bashrc | cut -d'=' -f2 | awk 'NR==1')
export MCU_LIBS=$MCU_HOME_DIR/lib:$MCU_HOME_DIR/lib64:$MCU_HOME_DIR/usr/lib:$MCU_HOME_DIR/usr/lib64

RETURN_STATUS=0


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

COLORIZE UNDERLINE
echo "Checking Soft MCU environment"
#echo "-----------------------------"
UNCOLORIZE
COLORIZE YELLOW
COLORIZE UNDERLINE
echo -n -e "OS health status for\t\t"
UNCOLORIZE
COLORIZE L_BLUE
echo -e "`hostname`"
UNCOLORIZE
echo -n -e "Uptime:\t\t\t\t"
COLORIZE BLUE
echo -e "`uptime | tr -s ' ' | cut -d' ' -f2- | cut -d',' -f1`"
UNCOLORIZE
RAM_Size=`free | head -2 | tail -1 | tr -s ' ' | cut -d' ' -f2`
RAM_Available=`free | head -2 | tail -1 | tr -s ' ' | cut -d' ' -f4`
echo -n -e "RAM free/available in KB:\t"
COLORIZE L_BLUE
echo -n -e "$RAM_Available"
UNCOLORIZE
echo -n -e " / "
COLORIZE BLUE
echo -n -e "$RAM_Size\n"
UNCOLORIZE
if (( $RAM_Available < 524288 )); then
	if ((  $RAM_Available < 102400 )); then
		COLORIZE RED
		echo "You are running low on RAM (less than 100 MB) !!!"
		UNCOLORIZE
		RETURN_STATUS=1
	else
		COLORIZE RED
		echo "You are running low on RAM (less than 512 MB) !!!"
		UNCOLORIZE
	fi
fi


#CPU_Load=`uptime | tr -s ' ' | cut -d' ' -f11-`
#CPU_Load=`uptime | tr -s ' ' | cut -d':' -f4 | cut -c2-`
CPU_Load=`uptime | tr -s ' ' | cut -d'e' -f4 | cut -c3-`
echo -n -e "CPU load:\t\t\t"
COLORIZE L_BLUE
echo -n -e "$CPU_Load\n"
UNCOLORIZE
#CPU_Load=`uptime | tr -s ' ' | cut -d' ' -f11 | tr -d ','`
#CPU_Load=`uptime | tr -s ' ' | cut -d':' -f4`
CPU_Load=`uptime | tr -s ' ' | cut -d'e' -f4 | cut -c3-`
if [[ `echo -n $CPU_Load | cut -d'.' -f1` > 0 ]]; then
	COLORIZE RED
	echo "Your VM is overloaded !!!"
	UNCOLORIZE
fi

Disk_Size=`df -h / | tail -1 | tr -s ' ' | cut -d ' ' -f2`
Disk_Free=`df -h / | tail -1 | tr -s ' ' | cut -d ' ' -f4`
Disk_Util=`df -h / | tail -1 | tr -s ' ' | cut -d ' ' -f5 | tr -d '%'`
echo -n -e "VM Disk space:\t\t\t"
COLORIZE L_BLUE
echo -n -e "$Disk_Free"
UNCOLORIZE
echo -n -e " / "
COLORIZE BLUE
echo -e "$Disk_Size"
UNCOLORIZE
if (( $Disk_Util > 98 )); then
	COLORIZE RED
	echo "Your VM is running out of disk space (less than 2% free) !!!"
	UNCOLORIZE
fi

#DISK_usage=`df -h -l / | grep "/$" | cut -d'G' -f4 | cut -d'%' -f1 | tr -d ' '`
DISK_usage=`df -h -l | grep '.*/$' | sed -e 's/^.* \([0-9]\+\)%.*/\1/'`
if [[ "X$DISK_usage" == "X" ]]; then
         DISK_usage=`df -h -l / | grep "/$" | cut -d'%' -f1 | tr -s ' ' | cut -d' ' -f5`
fi
echo -n -e "Disk usage%:\t\t\t"
COLORIZE L_BLUE
echo -n -e "$DISK_usage%\n"
UNCOLORIZE
if (( $DISK_usage > 98 )); then
	COLORIZE RED
        echo "Available Disk space is running low (less than 2%) !!!"
        UNCOLORIZE
        RETURN_STATUS=1
fi

echo -n -e "Network interfaces:\t\t"
Ifaces=`ifconfig -a | grep 'HWaddr' | tr -s ' ' | cut -d' ' -f1`
COLORIZE L_BLUE
echo -e "$Ifaces"
UNCOLORIZE

Zombies_Number=`ps -ef | grep '<defunct>' | grep -v grep | wc -l`
Zombies_Names=`ps -ef | grep '<defunct>' | tr -s ' ' | grep -v grep | tr -s ' ' | cut -d' ' -f8 | uniq | xargs -i echo -n {}`
if [[ $Zombies_Number > 0 ]]; then
	echo -n -e "Zombie processes:\t\t"
	COLORIZE RED
	echo -n -e "$Zombies_Number\t"
	COLORIZE WHILE
	echo -n -e "$Zombies_Names\n"
	UNCOLORIZE
else
	echo -n -e "Zombie processes:\t\t"
	COLORIZE GREEN
	echo -n -e "0\n"
	UNCOLORIZE
fi

COLORIZE YELLOW
COLORIZE UNDERLINE
echo "Soft MCU status"
UNCOLORIZE
echo -n -e "Soft MCU build:\t\t\t"
UNCOLORIZE
COLORIZE L_BLUE
grep BASELINE $MCU_HOME_DIR/mcms/Versions.xml | head -1 | cut -d'>' -f2 | cut -d'<' -f1
UNCOLORIZE

echo -n -e "Open SIP Port:\t\t\t"
if [[ `netstat -ntl | grep ':5060[^0-9]'` ]]; then
	COLORIZE GREEN
	echo -n -e "5060\n"
	UNCOLORIZE
elif [[ `netstat -ntl | grep ':5061[^0-9]'` ]]; then
	COLORIZE GREEN
	echo -n -e "5061\n"
	UNCOLORIZE
else
	COLORIZE RED
	echo -n -e "Fail\n"
	UNCOLORIZE
	RETURN_STATUS=1
fi

# Check user environment on simulation only
if [[ -d /opt/polycom/sim_pack ]]; then
	echo -n -e "User status is:\t\t\t"
	MCMS_USER=`stat -c "%U" $MCU_HOME_DIR/mcms/ 2> /dev/null`
	QUEUE_USER=`stat -c "%U" $MCU_HOME_DIR/tmp/queue 2> /dev/null`

	if [[ "$MCMS_USER" == "$QUEUE_USER" ]]; then 
		COLORIZE GREEN
		echo -n -e "OK\n"
		UNCOLORIZE
	else
		COLORIZE RED
		echo -n -e "Fail\n"
		UNCOLORIZE
	fi
fi

if [[ -f $MCU_HOME_DIR/mcms/ProductType ]]; then
	Product=`cat $MCU_HOME_DIR/mcms/ProductType`
	echo -n -e "Product Type:\t\t\t"
	COLORIZE PURPLE
	echo -n -e "$Product\n"
	UNCOLORIZE
else
	COLORIZE RED
	echo -e "Couldn't determine product type"
	UNCOLORIZE
	#exit 1
fi

SOFT_MAIN_RPM=`rpm -qa | grep SoftMcu`
if [ `echo $SOFT_MAIN_RPM | grep "Mfw"` ] ; then
	if [[ $Product != "SOFT_MCU_MFW" ]]; then
		COLORIZE RED
	        echo -e "Product type missmatches installation."
        	UNCOLORIZE
		RETURN_STATUS=1
	fi
elif [ `echo $SOFT_MAIN_RPM | grep "Edge"` ] ; then
        if [[ $Product != "SOFT_MCU_EDGE" ]]; then
                COLORIZE RED
                echo -e "Product type missmatches installation."
                UNCOLORIZE
                RETURN_STATUS=1
        fi
fi


#ps -ef | grep mcms | grep -v sleep | grep -v grep | tr -s ' '  | cut -d' ' -f 8- | sort | uniq 
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
#Bin/Ice 0;\
#Bin/LdapModule 0;\
#$MCU_HOME_DIR/mcms/Bin/httpd;\
#/bin/sh Bin/ApacheModule 0;\
#snmpd;\
#./bin/IpmcSim.x86;\
#============= End of Process List ============================================
tput sc
echo -n -e "Checking processes:\t\t"
COLORIZE YELLOW
echo -n -e "Please wait..."
UNCOLORIZE
if [[ "$Product" == "SOFT_MCU_MFW" ]]; then
	Process_List="/bin/sh $MCU_HOME_DIR/mcms/Scripts/SoftMcuWatcher.sh;$Process_List"
else
	Process_List="$Process_List;Bin/Failover 0;"
fi

if [[ "$Product" == "SOFT_MCU_EDGE" ]]; then
	Process_List="$Process_List;Bin/LicenseServer 0;"
fi


Count=0
Fail=0
Line=''

Process_Array=$(echo $Process_List)
IFS=";"
for Process in $Process_Array; do
        (( Count++ ))
        if [[ ! `ps -ef | grep -v '<defunct>' | grep "$Process" | grep -v grep` ]]; then

        	Failures[$Fail]="$Process"
                (( Fail++ ))
        fi
done

#tput el1
tput rc
echo -n -e "Checking processes:\t\t"
COLORIZE GREEN 
echo -e "OK            "
UNCOLORIZE
# Checking Apache
tput sc
echo -n -e "Checking Apache:\t\t"
COLORIZE YELLOW
echo -n -e "Please wait..."
UNCOLORIZE
success=0
First=`pidof httpd`
if [[ "X`echo $First | cut -d' ' -f2`" != "X" ]]; then
	success=1
fi
sleep 1
Second=`pidof httpd`
if [[ "X`echo $Second | cut -d' ' -f2`" != "X" ]]; then
	(( success ++ ))
fi
sleep 1
Third=`pidof httpd`
if [[ "X`echo $Third | cut -d' ' -f2`" != "X" ]]; then
	(( success ++ ))
fi

#echo "$First, $Second, $Third, $success"
if [[ "$First" == "$Second" && "$Second" == "$Third" && $success > 1 ]]; then
	tput el1
	tput rc
	echo -e -n "Checking Apache:\t\t"
	COLORIZE GREEN
	echo -e "OK"
	UNCOLORIZE
	if [[ ! `ps -ef | grep 'in/httpd' | grep -v grep` ]]; then
		if [[ ! -e $MCU_HOME_DIR/mcms/Bin/libProcessBase.so ]]; then
			Missing="$Missing libProcessBase.so"
		fi
		if [[ ! -e $MCU_HOME_DIR/mcms/Bin/libXmlPars.so ]]; then
			Missing="$Missing libXmlPars.so"
		fi
		if [[ ! -e $MCU_HOME_DIR/mcms/Bin/libCommon.so ]]; then
			Missing="$Missing libCommon.so"
		fi
		if [[ ! -e $MCU_HOME_DIR/mcms/Bin/libSimLinux.so ]]; then
			Missing="$Missing libSimLinux.so"
		fi
		if [[ ! -e $MCU_HOME_DIR/mcms/Bin/libMcmsEncryption.so ]]; then
			Missing="$Missing libMcmsEncryption.so"
		fi
		tput el1
		tput rc
	fi
else
	tput el1
        tput rc
        echo -e -n "Checking Apache:\t\t"
        COLORIZE RED
        echo -e "Fail"
        UNCOLORIZE
	RETURN_STATUS=1
fi

# Testing REST API
export REST=0
tput sc
echo -n -e "REST API:\t\t\t"
COLORIZE YELLOW
echo -n -e "Please wait..."
UNCOLORIZE
CURL_FLAG=""
if [[ -f $MCU_HOME_DIR/tmp/httpd.listen.conf ]]; then
	SOCKET=`grep '^Listen' $MCU_HOME_DIR/tmp/httpd.listen.conf | head -1 | cut -d' ' -f2`
	PORT=`echo -n $SOCKET | cut -d':' -f2`
	IP=`echo -n $SOCKET | cut -d':' -f1`
	IS_SECURE=`echo -n \`cat $MCU_HOME_DIR/mcms/Cfg/NetworkCfg_Management.xml | grep '<IS_SECURED>true</IS_SECURED>'\``
	if [[ "X$IS_SECURE" != "X"  &&  "$PORT" == "443" ]]; then
		HTTP_PREF="https://$IP"	
		CURL_FLAG="-1"
	elif [ "X$IS_SECURE" != "X" ]; then	
		HTTP_PREF="https://$IP:$PORT"	
		CURL_FLAG="-1"
	else
		HTTP_PREF="http://$IP:$PORT"
	fi	
	
	CMD=`curl -1 -i -k $CURL_FLAG --connect-timeout 5 --max-time 10 -H "If-None-Match: -1" -H "Authorization: Basic U1VQUE9SVDpTVVBQT1JU" -G ${HTTP_PREF}/plcm/mcu/api/1.0/conference/summary 2> /dev/null`
	if [[ $CMD =~ "200 OK"  || $CMD =~ "401 Unauthorized" ]] ;
	then
		tput el1
		tput rc
		echo -n -e "REST API (Port=$PORT):\t\t"
		COLORIZE GREEN
       		echo "OK"
       		UNCOLORIZE
       		REST=1
	else
		tput el1
		tput rc
		echo -n -e "REST API (Port=$PORT):\t\t"
		COLORIZE RED
       		echo "Fail"
       		UNCOLORIZE
       		RETURN_STATUS=1
	fi
else 
	tput el1
	tput rc
	echo -n -e "REST API (Port=N/A):\t\t"
	COLORIZE RED
       	echo "Fail"
       	UNCOLORIZE
       	RETURN_STATUS=1

fi
# End of Apache Check

export UP=0
if [[ "$Fail" == "0" ]]; then
	echo -n -e "Soft MCU status is:\t\t"
	COLORIZE GREEN
	echo "OK"
	UNCOLORIZE
	UP=1
else
	echo -n -e "Soft MCU status is:\t\t"
	COLORIZE RED
	echo -n -e "Fail "
	UNCOLORIZE
	echo -n -e "( "
	COLORIZE RED
	echo -n -e "$Fail"
	UNCOLORIZE
	echo -n -e " / "
	COLORIZE BLUE
	echo -n -e "$Count"
	UNCOLORIZE
	echo -n -e " )\n"
	COLORIZE YELLOW
	echo "Missing processes"
	UNCOLORIZE
	IFS=";"
	COLORIZE WHITE
	echo -e "${Failures[*]}"
	UNCOLORIZE
	RETURN_STATUS=1
fi

if [[ "X$1" != "X" && "$1" == "LOOP" ]]; then
	if [[ $REST == "0" && $UP == "0" ]]; then
        sleep 10
        clear
        $0 LOOP
    fi
fi
exit $RETURN_STATUS
