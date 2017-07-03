#!/bin/sh


echo "RunTestReport: Start: `date` ; `hostname` ; `whoami` ; $1 "

# print script header
Scripts/Header.sh  $1

# clean processes and timers
Scripts/Cleanup.sh
killall Timer.sh
killall sleep

usleep 200000


# global variables
SCRIPT_COMMAND=$1

PROCESSES='Authentication McuMngr Cards Resource MplApi CSApi CSMngr GideonSim 
           EndpointsSim ConfParty QAAPI CDR SipProxy DNSAgent Faults Gatekeeper
           Logger Configurator EncryptionKeyServer CertMngr'


isValgrindProcess="NO"
for process in $PROCESSES
do
  if [ $2 == $process ]
  then
      isValgrindProcess="YES"
      echo "valgrind process found:$isValgrindProcess"
  fi
done

if [ $isValgrindProcess == "YES" ]
then
    VALGRIND_PROCESS=$2
    RESULTS_DIR=$3
else
    RESULTS_DIR=$2
fi

if [ "$RESULTS_DIR" == "" ]
then
    RESULTS_DIR="TestResults"
fi

echo "SCRIPT_COMMAND: $SCRIPT_COMMAND"
echo "VALGRIND_PROCESS: $VALGRIND_PROCESS"
echo "RESULTS_DIR: $RESULTS_DIR"
EXIT_VALUE=0
USING_SCRIPT_TIMER=1
VALGRIND_RESULTS_FILE=""

# constants



# functions
RenameCore() {
    echo "rename core"
    #scriptName=`echo $1 | awk '{line=$0}; {split(line,names,"/")};END {print names[2]}' | awk '{line=$0}; {split(line,names,".")};END {print names[1]}'`
    scriptName=$(basename $1 .py)
    #echo $scriptName
   
    coreFiles=`find -maxdepth 1 -name "*.core.*" | grep -v ".dump" | grep -v ".Te*"`
    for coreFile in $coreFiles
    do
        echo "RunTestReport: RenameCore $coreFile $scriptName"
	#newFileName=`echo $coreFile"_"$scriptName".dump"`
	newFileName=$coreFile"_"$scriptName".dump."${HOSTNAME%%.*}
	mv $coreFile $newFileName
    done
}

CopyLogFiles() {
echo "RunTestReport: CopyLogFiles"
Scripts/Flush.sh

for log_file_created in `ls LogFiles/Log_*.txt`
 do
  if [ "$VALGRIND_PROCESS" != "" ]
  then
      cat $log_file_created >> "$RESULTS_DIR/$(basename $SCRIPT_COMMAND .py)_"$VALGRIND_PROCESS".log"
  else
      cat $log_file_created >> $RESULTS_DIR/$(basename $SCRIPT_COMMAND .py).log
  fi
  #echo "copy log fils $log_file_created to TestResults/$(basename $1 .sh).log" 
  #cat $log_file_created >> TestResults/$(basename $1 .sh).log
 done
}

ExitFromScript() {
if [ $USING_SCRIPT_TIMER == 1 ] #Regular script kill timer
then
    echo "Kill script  Timer"
    killall Timer.sh
    killall sleep
fi
echo copy log to the test resutls folder
#was cp -fp $MCU_HOME_DIR/tmp/loglog.txt TestResults/$(basename $1 .sh).log
CopyLogFiles $1
RenameCore $1
RenameValgrindResultsFile

echo "RunTestReport: End: `date` ; `hostname` ; `whoami` ; $SCRIPT_COMMAND "
echo "exit with status $EXIT_VALUE"
exit $EXIT_VALUE
}

GetValgrindResultsFileName() {
    if [ "$VALGRIND_PROCESS" != "" ]
    then
	valgrind_process_id="`ps | grep valgrind | awk '{print $1}'`"
	#echo "valgrind_process_id: $valgrind_process_id"
	
	valgrind_file_name="$VALGRIND_PROCESS.pid$valgrind_process_id"
	#echo "valgrind_file_name: $valgrind_file_name"

	VALGRIND_RESULTS_FILE=$valgrind_file_name
	if [ "$VALGRIND_RESULTS_FILE" == "" ]
	then
	    valgrind_process_id="`ps | grep memcheck | awk '{print $1}'`"
	    VALGRIND_RESULTS_FILE="$VALGRIND_PROCESS.pid$valgrind_process_id"
	fi
	if [ "$VALGRIND_RESULTS_FILE" == "" ]
	then
	    echo "GetValgrindResultsFileName: failed to find VALGRIND_RESULTS_FILE"
	else
	    echo "VALGRIND_RESULTS_FILE found: $VALGRIND_RESULTS_FILE"
	fi   
    fi
}

RenameValgrindResultsFile() {
    if [ "$VALGRIND_PROCESS" != "" ]
    then
	if [ "$VALGRIND_RESULTS_FILE" == "" ]
	then
	    echo "CopyValgrindResultsFileName: failed to find VALGRIND_RESULTS_FILE"
	else
	    old_file="TestResults/$VALGRIND_RESULTS_FILE"
	    new_file="$RESULTS_DIR/$(basename $SCRIPT_COMMAND .py)_ValgrindResults_$VALGRIND_RESULTS_FILE"
	    echo "rename valgrind file from: $old_file to $new_file"
	    cat $old_file >> $new_file
	    rm -f  $old_file
	fi   
    fi
}


data=$(grep "#-LONG_SCRIPT_TYPE" $1)
if [ "$data" == "" ] #Regular script activate timer
then

    echo "Activate script Timer"
    Scripts/Timer.sh 300 &	# 5 minutes timeout for each test
else
    $USING_SCRIPT_TIMER=0
fi

# try to activate prerun utility.
X=$(grep "#*PRERUN_SCRIPTS" $1 | awk -F "=" '{ print $2 }')
if [ "$X" != "" ]
    then
    echo "Run Prerun utility : " $X
    ./Scripts/$X
fi

if [ "$VALGRIND_PROCESS" != "" ]
then
    echo "RunTestReport: Startup.sh with $VALGRIND_PROCESS under valgrind"
    if Scripts/Startup.sh $VALGRIND_PROCESS;
    then
	echo "RunTestReport: startup exit ok"
    else
	echo "RunTestReport: startup exit with error"
	GetValgrindResultsFileName
	EXIT_VALUE=102
	ExitFromScript
	
    fi
else
    echo "RunTestReport: Startup.sh(2)"
    if Scripts/Startup.sh;
    then
	echo "RunTestReport: startup exit ok"
    else
	echo "RunTestReport: startup exit with error"
	EXIT_VALUE=102
	ExitFromScript
    fi

fi
 
# echo "RunTestReport: Executing test $1"
# echo "`ps | grep memcheck`"
# echo "$2.`ps | grep memcheck | awk '{print $1}'`"

# run script
if $SCRIPT_COMMAND;
then
    GetValgrindResultsFileName
    Scripts/EndTest.sh $SCRIPT_COMMAND 0 "$VALGRIND_PROCESS"
else
    # test failed
    GetValgrindResultsFileName
    EXIT_VALUE=101
    Scripts/EndTest.sh $SCRIPT_COMMAND 1 "$VALGRIND_PROCESS"
fi


ExitFromScript

#echo "RunTestReport: End: `date` ; `hostname` ; `whoami` ; $1 "
echo " should not be here: should exit on ExitFromScript"
exit 0
