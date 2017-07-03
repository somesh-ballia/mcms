#!/bin/bash
export PATH=$MCU_HOME_DIR/mcms/python/bin:${PATH}
export LD_LIBRARY_PATH=$MCU_HOME_DIR/usr/local/apache2/lib:$MCU_HOME_DIR/mcms/Bin
export SASL_PATH=$MCU_HOME_DIR/mcms/Bin
export ENDPOINTSSIM=YES

echo "RunTestReport: Start: `date` ; `hostname` ; `whoami` ; $@ "  

#echo $TXTBOLD$COLORWHITE$BGCOLORGREEN"RunTest is rinning with params $@"$TXTRESET


ScripTimeRunnings=""
OverAllScriptTimeRun="$(date +%s)"
CurrentScriptTimeRun="$(date +%s)"
ValdgrindProcesses=""
ProfilerProcesses=""
exitValgrind="10"
exiterror=0
scriptName=$1

echo "group name $4"
Scripts/Header.sh  $1



Scripts/Cleanup.sh
killall Timer.sh
killall sleep

#USAGE: $1 = TEST NAME
#       $2 = process under valgrind or ""
#       $3 = TIMER (40)

sleep 2

RenameCore() {
    #scriptName=`echo $1 | awk '{line=$0}; {split(line,names,"/")};END {print names[2]}' | awk '{line=$0}; {split(line,names,".")};END {print names[1]}'`
    scriptName=$(basename $1 .py)
    #echo $scriptName


#    coreFiles=`find -maxdepth 1 -name "*.core.*" | grep -v ".dump" | grep -v ".Te*"`

	coreFiles=`ls Cores/`
 	echo "corefiles: " $coreFiles
    for coreFile in $coreFiles
    do

		#newFileName=$coreFile"_"$scriptName".dump."${HOSTNAME%%.*}
		newFileName=$coreFile"_"$scriptName".dump"
        echo "RunTestReport: RenameCore Cores/$coreFile TestResults/$newFileName"
		mv Cores/$coreFile TestResults/$newFileName
		echo "call nt_results_db.py AddCoreDump  $1 $newFileName $4"
		Scripts/nt_results_db.py AddCoreDump $1 $newFileName $4 
    done
}

RenameValgrindCore() {

    scriptName=$(basename $1 .py)
    process=$2

	valgrind_corefiles=`ls TestResults/$process.core*`
	echo "Getting all core files $valgrind_corefiles"

    for valgrind_corefile in $valgrind_corefiles
      do
        if [[ "$valgrind_corefile" =~ "TestResults/" ]];
        then
                filename=$valgrind_corefile
        else
                filename="TestResults/$valgrind_corefile"
                echo "Adding TestResult/:  $filename"
        fi
        newFileName=$filename"_"$scriptName".valgrinddump"
        echo "RunTestReport: RenameCore $filename $newFileName"
        mv $filename $newFileName
        echo "call nt_results_db.py AddCoreDump  $1 $newFileName $4"
		Scripts/nt_results_db.py AddCoreDump $1 $newFileName $4 
    done
}

CopyLogFiles() {
	echo "RunTestReport: CopyLogFiles"
	Bin/LogUtil LogFiles/Log_*.log
	#rm -f TestResults/$(basename $1 .sh).log
	for log_file_created in `ls LogFiles/Log_*.txt`
	 do
		echo "copy log to TestResults/$(basename $1 .sh).log"
	  	cat $log_file_created >> TestResults/$(basename $1 .sh).log
   done
}

CopyValgrindLogFiles() {
#use LogUtil insted of loglog:
echo "RunTestReport: CopyValgrindLogFiles: $1 $2"

Bin/LogUtil LogFiles/Log_*.log

#rm -f TestResults/$(basename $1 .sh).log
	for log_file_created in `ls LogFiles/Log_*.txt`
	 do
	  echo "RunTestReport: Copy $log_file_created to TestResults/$(basename $1 .sh)_$process.log" 
	  cat $log_file_created >> "TestResults/$(basename $1 .sh)_"$process".log"

	done
	

  valgrind_file_to_copy=$2

  if [ "$valgrind_file_to_copy" != "" ]; then
		valgrind_file_dest_name=$(echo $valgrind_file_to_copy | sed 's/TestResults\//valgrind./g')
		echo "CopyValgrindLogFiles:   mv $valgrind_file_to_copy TestResults/$valgrind_file_dest_name"
		mv $valgrind_file_to_copy TestResults/$valgrind_file_dest_name  
		chmod 777 TestResults/$valgrind_file_dest_name	
  else
	echo "valgrind file: 	$valgrind_file_to_copy not exists"
  fi

}

RemoveCallgrindCores() {
	process=$1
	echo "RunTestReport: RemoveCallgrindCores: process:$process"
	
	echo "rm TestResults/$process.prof"
	rm TestResults/$process.prof
	echo "rm TestResults/$process.prof.core.*"
	rm TestResults/$process.prof.core.*
}

CopyCallgrindFiles() {
	script_name=$1
	file_to_copy=$2
	echo "RunTestReport: CopyCallgrindFiles params: script_name:$script_name file_to_copy:$file_to_copy"
	
	if [ "$file_to_copy" != "" ]; then
		#file_dest_name=$(echo $file_to_copy | sed 's/TestResults\//callgrind./g')

		startup_file=$(echo "$file_to_copy".1)
		if [ -f "$startup_file" ]; then
			file_dest_name=$(echo "$startup_file" | sed -e 's:.callgrind.out.*::g' -e 's:TestResults/:callgrind.:g').startup
			echo "RunTestReport: CopyCallgrindFiles:   mv $startup_file TestResults/$file_dest_name"
			mv "$startup_file" TestResults/$file_dest_name  
			chmod 777 TestResults/$file_dest_name	
		fi
		
		if [ -f "$file_to_copy" ]; then
			file_dest_name=$(echo $file_to_copy | sed -e 's:.callgrind.out.*::g' -e 's:TestResults/:callgrind.:g')
			echo "RunTestReport: CopyCallgrindFiles:   mv $file_to_copy TestResults/$file_dest_name"
			mv $file_to_copy TestResults/$file_dest_name  
			chmod 777 TestResults/$file_dest_name
		fi	
	else
		echo "RunTestReport: CopyCallgrindFiles report file: '$file_to_copy' not exists"
	fi
}

PROCESSES='Authentication McuMngr Cards Resource MplApi CSApi CSMngr GideonSim 
           EndpointsSim ConfParty QAAPI CDR SipProxy DNSAgent Faults Gatekeeper
           Logger Configurator EncryptionKeyServer CertMngr BackupRestore'

PROFILER_PROC='ConfParty'

# try to read which processes need to run in valgrind from the script itself
X=$(grep "#*PROCESSES_FOR_VALGRIND" $1)
if [ "$X" != "" ]
then
    export PROCESSES=`echo ${X:25} | sed -e "s:'::g" -e 's:"::g'`
fi

# try to read which processes should not run in valgrind from the script itself
X=$(grep "#*PROCESSES_NOT_FOR_VALGRIND" $1)
if [ "$X" != "" ]
then
    export PROCESSES_NOT_FOR_VALGRIND=${X:29}
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=all" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then
        echo "All processes will run under valgrind"
      fi  
      PROCESSES='Authentication McuMngr Cards Resource MplApi CSApi CSMngr GideonSim EndpointsSim ConfParty QAAPI CDR SipProxy DNSAgent Faults Gatekeeper Logger Configurator EncryptionKeyServer McmsDaemon Installer IPMCInterface Collector SystemMonitoring Auditor CertMngr'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_1 (Conf/Party) processes will run under valgrind"
      fi  
      PROCESSES='Resource MplApi CSApi ConfParty CDR SipProxy DNSAgent Faults Gatekeeper Logger EncryptionKeyServer Auditor'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_2 (Users /Connections) processes will run under valgrind"
      fi  
      PROCESSES='Authentication McuMngr MplApi GideonSim QAAPI Faults Logger Configurator'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_3" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then
        echo "Profile_3 (IP service) processes will run under valgrind"
      fi  
      PROCESSES='McuMngr Cards Resource MplApi CSApi CSMngr GideonSim EndpointsSim ConfParty SipProxy DNSAgent Faults Gatekeeper Logger'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then 
        echo "Profile_4 (MR/EQ/profiles lists) processes will run under valgrind"
      fi  
      PROCESSES='Resource ConfParty	CDR'
fi      

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_5" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then 
        echo "Profile_5 (cards monitoring) processes will run under valgrind"
      fi  
      PROCESSES='Resource Cards Faults Logger'
fi   

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then
        echo "Profile_6 (Cards startup and maintain) processes will run under valgrind"
      fi  
      PROCESSES='McuMngr Cards Resource MplApi CSApi CSMngr GideonSim EndpointsSim Faults Logger McmsDaemon Installer'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_7" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then
        echo "Profile_7 (CS Cards startup and maintain) processes will run under valgrind"
      fi  
      PROCESSES='Cards Resource CSApi CSMngr EndpointsSim Faults Logger Installer'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_8 (Conf/Party) processes will run under valgrind"
      fi  
      PROCESSES='Resource MplApi CSApi GideonSim EndpointsSim ConfParty CDR Faults Logger EncryptionKeyServer'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_9 (Conf/Party) Application processes will run under valgrind"
      fi  
      PROCESSES='ConfParty'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_10" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_10 (Resource MplApi CSApi GideonSim EndpointsSim CDR Faults Logger EncryptionKeyServer) processes will run under valgrind - to much stress for ConfParty"
      fi  
      PROCESSES='Resource MplApi CSApi GideonSim EndpointsSim CDR Faults Logger EncryptionKeyServer'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_11" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_11(CSMngr) - Authentication McuMngr Cards Resource MplApi CSApi CSMngr GideonSim EndpointsSim ConfParty SipProxy DNSAgent Gatekeeper, will run under valgrind "
      fi  
      PROCESSES='Authentication McuMngr Cards Resource MplApi CSApi CSMngr GideonSim EndpointsSim ConfParty SipProxy DNSAgent Gatekeeper'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_12" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_12() - CDR , will run under valgrind "
      fi  
      PROCESSES='CDR McuMngr'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_13" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_13() - McuMngr , will run under valgrind "
      fi  
      PROCESSES='McuMngr'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_14" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_14() - CDR Faults Logger , will run under valgrind "
      fi  
      PROCESSES='CDR Faults Logger'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_15" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_15() - Configurator Installer , will run under valgrind "
      fi  
      PROCESSES='Configurator Installer'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_16" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_16() - SNMPProcess , will run under valgrind "
      fi  
      PROCESSES='SNMPProcess'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_17" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_17(CSMngr) - McuMngr Cards Resource MplApi CSApi CSMngr GideonSim EndpointsSim ConfParty SipProxy DNSAgent Gatekeeper, will run under valgrind "
      fi  
      PROCESSES='McuMngr Cards Resource MplApi CSApi CSMngr GideonSim ConfParty SipProxy DNSAgent Gatekeeper'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_18" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_18 - Cards Resource GideonSim ConfParty, will run under valgrind "
      fi  
      PROCESSES='Cards Resource GideonSim ConfParty'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_19" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_19 - Auditor, will run under valgrind "
      fi  
      PROCESSES='Auditor'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_20" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_20 - Cards Resource ConfParty, will run under valgrind "
      fi  
      PROCESSES='Cards Resource ConfParty'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_21" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_21 -  ConfParty SipProxy DNSAgent  EncryptionKeyServer  Gatekeeper, will run under valgrind "
      fi  
      PROCESSES='ConfParty SipProxy DNSAgent  EncryptionKeyServer  Gatekeeper'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_22" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_21 - ApacheModule, will run under valgrind "
      fi  
      PROCESSES='ApacheModule'
fi

X=$(grep "#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind" $1)
if [ "$X" != "" ]
then
      if [ "$2" == "valgrind" ]
      then  
        echo "Profile_No_Valgrind (no process will run under valgrind)"
      fi  
      PROCESSES=''
fi

Y=$(grep "#*PROCESSES_FOR_PROFILER" $1)
if [ "$Y" != "" ]
then
	PROFILER_PROC=`echo ${Y:25} | sed -e "s:'::g" -e 's:"::g'`
fi


# kobig : the previous export within inside the tmp.sh did not work at all
######## start : export part of Script sile
IN=`grep "#*export" $1 | sed 's/"//g'| awk '{ print $2 }'`
arr=$(echo $IN | tr " " "\n")

for x in $arr
do
    echo "export $x"
    export "$x"
done
######## end : export part of Script sile

data=$(grep "#-LONG_SCRIPT_TYPE" $1)
if [ "$data" == "" ] #Regular script activate timer
then
    echo "Activate script Timer"
    Scripts/Timer.sh 330 &	# 5.5 minutes timeout for each test
fi

# try to activate prerun utility.
X=$(grep "#*PRERUN_SCRIPTS" $1 | awk -F "=" '{ print $2 }')
if [ "$X" != "" ]
    then
    echo "Run Prerun utility : " $X
    ./Scripts/$X
fi

startupCMD="Scripts/Startup.sh"
cleanupCMD="Scripts/Cleanup.sh"

export MCMS_DIR=`pwd`
	if [[ -z $OFFICIAL_DIR ]]
	then           
           export OFFICIAL_DIR=/Carmel-Versions/SVN/Builds/prod/Main/trunk/last/
    fi
if [ "$SOFT_MCU" == "YES" ]
then	
	startupCMD="./Scripts/soft.sh start_vm&"
	cleanupCMD="./Scripts/soft.sh stop"
fi

if [ "$SOFT_MCU_EDGE" == "YES" ]
then		
	startupCMD="./Scripts/soft.sh start_vm_edge&"
	cleanupCMD="./Scripts/soft.sh stop"
fi

if [ "$SOFT_MCU_MFW" == "YES" ]
then	
	startupCMD="./Scripts/soft.sh start_vm_mfw&"
	cleanupCMD="./Scripts/soft.sh stop"
fi
###### cs simulation var 
CS_PID=0
if [ "$CS_SIMULATION_TEST" == "YES" ];then
	cleanupCMD="./Scripts/cleanup_cs_mcms.sh"
	if [ ! -d "$MCU_HOME_DIR/cs" ]; then
  		echo "cannot run cs tests, CS folder does not exist please make sure to export CS_DIR."
  		exit 1
	fi
	CS_PID=$(ps -ef | grep acloader | grep -v ' grep ' | awk '{print $2}') 
	if [ ! -z "$CS_PID" ]; then
		echo cs loader already running, closing it. pid $CS_PID
		kill $CS_PID
	fi
        eval "$cleanupCMD"
  cd $MCU_HOME_DIR/cs
  ./bin/loader > $MCU_HOME_DIR/cs/logs/output/cs_console.txt&
  cd -
 sleep 2
 CS_PID=$(ps -ef | grep acloader | grep -v ' grep ' | awk '{print $2}') 
	if [ -z "$CS_PID" ]; then
	  echo cs loader not running
	else
	  echo cs loader is running! pid  $CS_PID
	fi
else
  echo "not a cs simulation test"
  
fi
if [ "$2" != "" ] && [ "$2" != "valgrind" ]
then
    echo "RunTestReport: $startupCMD(1) $2_$3"
    if [[ "$SOFT_MCU" == "YES" || "$SOFT_MCU_EDGE" == "YES" || "$SOFT_MCU_MFW" == "YES" ]]; then
      SOFT_VALGRIND_PROC=$2 "$startupCMD"
    else
      eval "$startupCMD" $2 "" $3
    fi
    echo "`ps`"
    echo "`ps | grep valgrind`"
    echo "`ps | grep memcheck`"
    echo "$2.`ps | grep memcheck | awk '{print $1}'`"
    echo "TestResults/$2.`ps | grep memcheck | awk '{print $1}'`"
else
    echo "RunTestReport: $startupCMD(2)"

    if [[ "$SOFT_MCU" == "YES" || "$SOFT_MCU_EDGE" == "YES" || "$SOFT_MCU_MFW" == "YES" ]]; then		
        eval "$startupCMD"
	echo "sleep 3 minutes to wait for soft MCU Startup"
	sleep 3m
    else	
        eval "$startupCMD" "" "" $3
    fi
fi

if $1;
then

    ####valgrind_file="TestResults/$2.pid`ps | grep valgrind | awk '{print $1}'`"
    #valgrind_file="TestResults/$2.`ps | grep memcheck | awk '{print $1}'`"

    echo "RunTestReport: EndTest.sh(1) $1 $2"

    #exiterror=(Scripts/EndTest.sh $1 0 "$2")
    Scripts/EndTest.sh $1 0 "$2" || exiterror=$?

   # echo "RunTestReport: Analyzing valgrind file $valgrind_file"
    #Scripts/AnalizeValgrindFile.sh $valgrind_file $1 $2    
else
    #valgrind_file="TestResults/$process.`ps | grep memcheck | awk '{print $1}'`"
    #echo "RunTestReport: Analyzing valgrind file $valgrind_file"        

    echo "RunTestReport: EndTest.sh(2) $1 $2"
    Scripts/EndTest.sh $1 1 "$2" || exiterror=$?
    if [ $exiterror -eq 0 ] 
    then
	#echo $COLORBROWN"RunTest($1): errNo will be set 101"$TXTRESET
    	exiterror=101
    fi
    
    #echo "RunTestReport: Analyzing valgrind file $valgrind_file"
    #Scripts/AnalizeValgrindFile.sh $valgrind_file $1 $2
fi

if [ "$CS_SIMULATION_TEST" == "YES" ]
then
 echo kill cs loader process  $CS_PID
 kill $CS_PID
 eval "$cleanupCMD"

fi

CurrentScriptTimeRun="$(($(date +%s)-CurrentScriptTimeRun))"
ScripTimeRunnings="NoValgrind - ${CurrentScriptTimeRun} sec"

if [ "$data" == "" ] #Regular script kill timer
then
    echo "Kill script  Timer"
    killall Timer.sh
    killall sleep
fi

echo copy log to the test resutls folder
#was cp -fp $MCU_HOME_DIR/tmp/loglog.txt TestResults/$(basename $1 .sh).log
CopyLogFiles $1
RenameCore $1

if [ "$4" != "" ]
then
	echo "call nt_results_db.py AddTestStatus $1 $exiterror $4"
	Scripts/nt_results_db.py AddTestStatus $1 $exiterror $4
else
	echo "group is empty not a night test run"
fi



echo "With valgrind: $2 exiterror $exiterror"
if [ "$2" == "valgrind" ]  && [ $exiterror == "0" ]
then
    for process in $PROCESSES 
    do
      export isProcessFoundInNonValgrindList=0
      for process_not_for_valgrind in $PROCESSES_NOT_FOR_VALGRIND 
      do
      		if [ "$process_not_for_valgrind" == "$process" ] 
      		then
      			isProcessFoundInNonValgrindList=1
      			break
      		fi
      done
           
      if [ $isProcessFoundInNonValgrindList == 1 ] 
      then
      		continue
      fi

      echo "------------------------------------------------------------------------------"
      echo "testing " $1 " with " $process " under valgrind"
      export PROCESS_UNDER_VALGRIND=$process


      # try to activate prerun utility.
      X=$(grep "#*PRERUN_SCRIPTS" $1 | awk -F "=" '{ print $2 }')
      if [ "$X" != "" ]
	  then
	  echo "Run Prerun utility : " $X
	  ./Scripts/$X
      fi


     # usleep 200000
      sleep 20
      if [ "$data" == "" ]
      then
          	if [ "$PROCESS_UNDER_VALGRIND" == "EncryptionKeyServer" ]
          	then
              	Scripts/Timer.sh 480 & # 8 minutes timeout for Encryption under valgrind test
          	else
      	  		if [ "$PROCESS_UNDER_VALGRIND" == "CDR" ]
      	  		then
        			Scripts/Timer.sh 600 & # 10 minutes timeout for each valgrind test
          		else
        			Scripts/Timer.sh 300 & # 5 minutes timeout for each valgrind test
          		fi
        	fi
      fi  
      #CurrentScriptTimeRun="$(date +%s)"

      echo "RunTestReport: $cleanupCMD"
      eval "$cleanupCMD"
      echo "RunTestReport: $startupCMD $process"
      if [[ "$SOFT_MCU" == "YES" || "$SOFT_MCU_EDGE" == "YES" || "$SOFT_MCU_MFW" == "YES" ]]; then
          SOFT_VALGRIND_PROC=$process $startupCMD
      else
          eval "$startupCMD" $process "" ""
      fi
      echo "---------------"
      echo "RunTestReport: Executing $1 with $process unger valgrind"
      echo "`ps | grep ConfParty`"

      valgrind_file="TestResults/$process.`ps | grep "memcheck" | awk '{print $1}'`"
	#  valgrind_file_to_copy=`ls -altr TestResults/$process* | awk '{print $9}'`
 	  if [ -e $valgrind_file ]; then	
		echo "valgrind_file file $valgrind_file"	
	  else
		valgrind_file="TestResults/$process"
	  fi

      $1

      Scripts/EndTest.sh $1"_With_"$process"_under_valdgrind" $? "$process"
     if [ "$4" != "" ]
	 then
		exiterror=$?
		echo "call nt_results_db.py AddValgrindStatus $1 $exiterror $4 $process"
		Scripts/nt_results_db.py AddValgrindStatus $1 $exiterror $4 $process
	 else
		 echo "group is empty not a night test run"
	 fi
     
    
	 
      #CurrentScriptTimeRun="$(($(date +%s)-CurrentScriptTimeRun))"

      ValdgrindProcesses="${ValdgrindProcesses} ${process}"


      echo "RunTestReport: Analyzing valgrind file $valgrind_file"
     
    result=`Scripts/AnalizeValgrindFile.sh $valgrind_file $1 $2`
     if [ "$4" != "" ]
	 then
		echo "$result vs $exitValgrind" 
		if [ "$result" -gt "$exitValgrind" ]
		then
			exitValgrind=$result
		else
			echo " exitValgrind $exitValgrind  >  result $result " 
		fi		
	 else
		 echo "group is empty not a night test run"
	 fi
      RenameCore $1

      CopyValgrindLogFiles $1 $valgrind_file
	  RenameValgrindCore  $1 $process   

      #killall Timer.sh
      if [ "$data" == "" ] #Regular script kill timer
	  then
    	echo "Kill script  Timer"
    	killall Timer.sh
    	killall sleep
	  fi
    done
    if [ "$4" != "" ]
	 then
		echo "call nt_results_db.py AddValgrindError  $1 $exitValgrind $4"
		Scripts/nt_results_db.py AddValgrindError $1 $exitValgrind $4
	 else
		 echo "group is empty not a night test run"
	 fi
else
	echo "no valgrind required"
fi

# VASILY start
echo "With profiler: $2 exiterror $exiterror"
if [ "$2" == "valgrind" ]  && [ $exiterror == "0" ]
then
	for process in $PROFILER_PROC
	do
		echo "------------------------------------------------------------------------------"
		echo "testing " $1 " with " $process " under profiler"
		export PROCESS_UNDER_PROFILER=$process

		# try to activate prerun utility.
		X=$(grep "#*PRERUN_SCRIPTS" $1 | awk -F "=" '{ print $2 }')
		if [ "$X" != "" ]
		then
			echo "Run Prerun utility : " $X
			./Scripts/$X
		fi

		sleep 20
		if [ "$data" == "" ]
		then
			if [ "$PROCESS_UNDER_PROFILER" == "EncryptionKeyServer" ]
			then
				Scripts/Timer.sh 480 & # 8 minutes timeout for Encryption under valgrind test
			else
				if [ "$PROCESS_UNDER_PROFILER" == "CDR" ]
				then
					Scripts/Timer.sh 600 & # 10 minutes timeout for each valgrind test
				else
					Scripts/Timer.sh 300 & # 5 minutes timeout for each valgrind test
				fi
			fi
		fi  
		#CurrentScriptTimeRun="$(date +%s)"

		echo "RunTestReport: $cleanupCMD"
		eval "$cleanupCMD"
		echo "RunTestReport: $startupCMD $process"
		if [[ "$SOFT_MCU" == "YES" || "$SOFT_MCU_EDGE" == "YES" || "$SOFT_MCU_MFW" == "YES" ]]; then
			SOFT_VALGRIND_PROC=$process $startupCMD
		else
			eval "$startupCMD $process prof"
    	fi
    	
		echo "---------------"
		echo
		echo "RunTestReport: Executing $1 with $process unger profiler"
		echo "`ps | grep ConfParty`"

		profiler_report_file="TestResults/$process.callgrind.out.`ps | grep "callgrind" | awk '{print $1}'`"
		echo "RunTestReport: profiler_report_file: $profiler_report_file"
		echo "RunTestReport: dump profiler startup report"
		callgrind_control -d
		
		sleep 2

		$1

		Scripts/EndTest.sh $1"_With_"$process"_under_profiler" $? "$process"
		if [ "$4" != "" ]
		then
			exiterror=$?
			echo "call nt_results_db.py AddValgrindStatus $1 $exiterror $4 $process"
			Scripts/nt_results_db.py AddValgrindStatus $1 $exiterror $4 $process
		else
			echo "group is empty not a night test run"
		fi

		RemoveCallgrindCores $process
		
		ProfilerProcesses="${ProfilerProcesses} ${process}"

		#echo "RunTestReport: Analyzing valgrind file $profiler_report_file"

		#result=`Scripts/AnalizeValgrindFile.sh $profiler_report_file $1 $2`
		#if [ "$4" != "" ]
		#then
			#echo "$result vs $exitValgrind" 
			#if [ "$result" -gt "$exitValgrind" ]
			#then
				#exitValgrind=$result
			#else
				#echo " exitValgrind $exitValgrind  >  result $result " 
			#fi
		#else
			#echo "group is empty not a night test run"
		#fi
		#RenameCore $1

		CopyCallgrindFiles $1 $profiler_report_file
		#RenameValgrindCore  $1 $process   

		if [ "$data" == "" ] #Regular script kill timer
		then
			echo "Kill script  Timer"
			killall Timer.sh
			killall sleep
		fi
	done
	#if [ "$4" != "" ]
	#then
		#	echo "call nt_results_db.py AddValgrindError  $1 $exitValgrind $4"
		#	Scripts/nt_results_db.py AddValgrindError $1 $exitValgrind $4
	#else
		#	echo "group is empty not a night test run"
	#fi
else
	echo "no profiler required"
fi
#VASILY end

if [ $exiterror -eq 0 ]
then 
  echo $COLORGREEN
else
  echo $COLORRED
fi


OverAllScriptTimeRun="$(($(date +%s)-OverAllScriptTimeRun))"
currentScriptName="$(basename $1 .py)"

ScripTimeRunnings="$currentScriptName: Overall Running Time - ${OverAllScriptTimeRun} sec $ScripTimeRunnings; ValdgrindProcesses: '$ValdgrindProcesses'; ProfilerProcesses: '$ProfilerProcesses'" 

echo $ScripTimeRunnings >> TestResults/ScriptResult.log


#echo "RunTestReport End: $1 exitCode $exiterror `date` ; `hostname` ; `whoami` ;  "
echo "RunTestReport End ($exiterror) : $1"
echo "System info : `whoami` ; `date` ; `hostname` "
echo $TXTRESET

exit $exiterror
