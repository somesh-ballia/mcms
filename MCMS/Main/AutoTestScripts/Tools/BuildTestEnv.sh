#!/bin/sh

function remove_jobs {
    echo remove jobs: atrm `atq | cut -f 1`
    atrm `atq | cut -f 1`
}

# print out all input parameters
echo $# params: $@
echo "`date` ; `hostname` ; `whoami`"

TEST_NAME=$1
BUILD_PATH=$PWD/$2
TEST_PATH=$PWD/$3/$TEST_NAME
RESULT_PATH=$PWD/$4
CARD_TYPE=$5
MODE_TYPE=$6
TIMEOUT=$7
LOGS_DIRECTORY=$8
STARTUP_TIMEOUT=$9

# check validity of input parameters
if [ -z $1 ]; then
    echo build error: invalid parameter TEST_NAME
    exit 1
fi
if [ -z $2 ]; then
    echo build error: invalid parameter BUILD_PATH
    exit 1
fi
if [ -z $3 ]; then
    echo build error: invalid parameter TEST_PATH
    exit 1
fi
if [ -z $4 ]; then
    echo build error: invalid parameter RESULT_PATH
    exit 1
fi
if [ -z $CARD_TYPE ]; then
    CARD_TYPE=MPMX
    echo unknown card type, set to default
fi
if [ -z $MODE_TYPE ]; then
    MODE_TYPE=DEFAULT
    echo unknown feature type, set to default
fi
if [ -z $TIMEOUT ]; then
    TIMEOUT=120
    echo unknown timeout, set to default
fi

echo TEST_NAME: $TEST_NAME
echo BUILD_PATH: $BUILD_PATH
echo TEST_PATH: $TEST_PATH
echo RESULT_PATH: $RESULT_PATH
echo CARD_TYPE: $CARD_TYPE
echo MODE_TYPE: $MODE_TYPE
echo TIMEOUT: $TIMEOUT min

if [ "`whoami`" != "carmel" ]; then
    echo build error: must be user carmel
    exit 1
fi

# clean out all jobs 
remove_jobs

if [ -d $TEST_PATH ]; then
    echo remove existed $TEST_PATH
    rm -rf $TEST_PATH

    if [ "$?" != "0" ]; then
        echo build error: unable to remove old $TEST_PATH
        exit 2
    fi
fi

echo create directory $TEST_PATH
mkdir -p $TEST_PATH
cd $TEST_PATH

mkdir Bin
mkdir Cfg
ln -sf $BUILD_PATH/common.mk
mkdir Cores
ln -sf $BUILD_PATH/EMA/
mkdir Faults 
mkdir IVRX
ln -sf $BUILD_PATH/IVRX_Save/
mkdir Keys
ln -sf $BUILD_PATH/Libs/
ln -sf $BUILD_PATH/Links/
ln -sf $BUILD_PATH/Main.mk
ln -sf $BUILD_PATH/Makefile
ln -sf $BUILD_PATH/MIBS/
ln -sf $BUILD_PATH/python
ln -sf $BUILD_PATH/Processes/
mkdir Restore
ln -sf $BUILD_PATH/schemas/
ln -sf $BUILD_PATH/Scripts/
ln -sf $BUILD_PATH/AutoTestScripts/
mkdir Simulation
mkdir States
ln -sf $BUILD_PATH/StaticCfg/
mkdir StaticStates
ln -sf $BUILD_PATH/Stripped/
mkdir TestResults
ln -sf $BUILD_PATH/Utils/
ln -sf $BUILD_PATH/VersionCfg/
ln -sf $BUILD_PATH/Bin.i32cent56/
ln -sf $BUILD_PATH/Bin.i32ptx/

# check file existence
if [ ! -f Scripts/Cleanup.sh ]; then
    echo build error: unable to find Scripts/Cleanup.sh
    exit 3
fi

if [ ! -f Scripts/RunTest.sh ]; then
    echo build error: unable to find Scripts/RunTest.sh
    exit 4
fi

# check existence of the test file
TEST_FNAME=AutoTestScripts/${TEST_NAME}.py

# first see at AutoTestScripts
if [ ! -f $TEST_FNAME ]; then
   TEST_FNAME=Scripts/${TEST_NAME}.py
fi

# see at Scripts
if [ ! -f $TEST_FNAME ]; then
   echo build error: unable to find $TEST_NAME
   exit 5
fi

echo TEST_NAME: $TEST_NAME:

# define if test has LONG_SCRIPT_TYPE
echo "grep \#-LONG_SCRIPT_TYPE $TEST_FNAME"
IS_LONG=`grep \#-LONG_SCRIPT_TYPE $TEST_FNAME`
RES=$?
if [ $RES != "0" ]; then
    if [ $RES != "1" ]; then
        echo build error: unable to read file $TEST_FNAME
        exit 6
    fi
fi

echo "IS_LONG=$IS_LONG"

# add valgrind if a test has LONG_SCRIPT_TYPE
if [ -z $IS_LONG ]; then
    VALGRIND=""
else
    VALGRIND=valgrind
fi

echo "VALGRIND: $VALGRIND"

case $CARD_TYPE in
    MPM)
        export SYSTEM_CARDS_MODE_FILE='VersionCfg/SystemCardsMode_mpm.txt'
        export MPL_SIM_FILE='VersionCfg/MPL_SIM_2.XML'
        export LICENSE_FILE='VersionCfg/Keycodes_9251aBc471_40_YES_v100.0.cfs'
        ;;

    MPMP)
        export SYSTEM_CARDS_MODE_FILE='VersionCfg/SystemCardsMode_mpmPlus.txt'
        export MPL_SIM_FILE='VersionCfg/MPL_SIM_BARAK.XML'
        export LICENSE_FILE='VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs'
        export RESOURCE_SETTING_FILE='VersionCfg/Resource_Setting_144Video_80Audio.xml'
        ;;

    MPMX)
        export SYSTEM_CARDS_MODE_FILE='VersionCfg/SystemCardsMode_breeze.txt'
    export MPL_SIM_FILE='VersionCfg/MPL_SIM_BREEZE.XML'
        export LICENSE_FILE='VersionCfg/Keycodes_9251aBc471_60HD_v100.0.cfs'
        export RESOURCE_SETTING_FILE='VersionCfg/Resource_Setting_Default_Breeze.xml'
        ;;

    *)
        echo unknown card type $CARD_TYPE
        ;;
esac

case $MODE_TYPE in
DEFAULT)
    ;;

EVENT_MODE)
    export LICENSE_FILE='VersionCfg/Keycodes_9251aBc471_120COP_v100.0.cfs'
    ;;

*)
    echo unknown mode type $MODE_TYPE
    ;;
esac

echo $MODE_TYPE
echo SYSTEM_CARDS_MODE_FILE: $SYSTEM_CARDS_MODE_FILE
echo MPL_SIM_FILE: $MPL_SIM_FILE
echo LICENSE_FILE: $LICENSE_FILE

export CLEAN_CFG=YES

Scripts/Cleanup.sh

make active

echo "mkdir -p TestResults/$TEST_NAME"
mkdir -p TestResults/$TEST_NAME

TEST_RUN_TXT=TestResults/$TEST_NAME/test_run.txt
TEST_REPORT_TXT=TestResults/$TEST_NAME/test_report.txt

# set autodestruct timer before test starting
echo set autodestruct job in $TIMEOUT minutes
echo "Scripts/batchkill.sh; \
      killall -q -9 $MCU_HOME_DIR/mcms/python/bin/python; \
      killall -q -9 RunTest.sh; \
      killall -q -9 BuildTestEnv.sh \
      sleep 60" | at now + $TIMEOUT min

# run test
echo "Scripts/RunTest.sh $TEST_FNAME $VALGRIND 2>&1 | tee $TEST_RUN_TXT"
Scripts/RunTest.sh $TEST_FNAME $VALGRIND $STARTUP_TIMEOUT 2>&1 | tee $TEST_RUN_TXT
RunTestExitStatus=$?

# clean out scheduled job
remove_jobs

# clean after finish
Scripts/Cleanup.sh

#Wait 29 before next test
sleep 30 

# create report files
echo "Scripts/create_report.sh $TEST_NAME test_run.txt 2>&1 | tee $TEST_REPORT_TXT"
Scripts/create_report.sh $TEST_NAME test_run.txt 2>&1 | tee $TEST_REPORT_TXT

ls *.core.*_${TEST_NAME}.*
if [ $? == "0" ]; then
    echo "cp *.core.*_${TEST_NAME}.* TestResults/$TEST_NAME"
    cp *.core.*_${TEST_NAME}.* TestResults/$TEST_NAME
else
    echo there are no core dump files for $TEST_NAME
fi

# keep host and CC info
INFO_FNAME=TestResults/$TEST_NAME/`hostname -s`_Data.log
echo #ClientSpcifiedInfo:`hostname -s`:$TEST_PATH: > $INFO_FNAME
#echo `cleartool lsbl -s | tail -1` >> $INFO_FNAME
#echo `cleartool lsstream -fmt %[found_bls]Np -cview | fgrep MCMS` >> $INFO_FNAME

#copy test results to common directory
TEST_RESULT_PATH=$RESULT_PATH/`date +%A`/$TEST_NAME

echo "mkdir -p $TEST_RESULT_PATH"
mkdir -p $TEST_RESULT_PATH

PROCESSES='Authentication McuMngr Cards Resource MplApi CSApi CSMngr GideonSim EndpointsSim ConfParty QAAPI CDR SipProxy DNSAgent Faults Gatekeeper Logger Configurator EncryptionKeyServer McmsDaemon Installer IPMCInterface Collector SystemMonitoring Auditor CertMngr'
echo "cp -r TestResults/$TEST_NAME/* $TEST_RESULT_PATH"
cp -r TestResults/$TEST_NAME/* $TEST_RESULT_PATH
#echo "cp -r TestResults/$TEST_NAME*.valgrind $TEST_RESULT_PATH"
#cp -r TestResults/$TEST_NAME*.valgrind $TEST_RESULT_PATH
for process in $PROCESSES
	do
		valgrind_file_to_copy=`ls -altr TestResults/$process* | awk '{print $9}'`
		if [ "$valgrind_file_to_copy" != "" ]; then
			valgrind_file_dest_name=$(echo $valgrind_file_to_copy | sed 's/TestResults\//valgrind./g')
			echo "mv $valgrind_file_to_copy $TEST_RESULT_PATH/$valgrind_file_dest_name"
			mv $valgrind_file_to_copy $TEST_RESULT_PATH/$valgrind_file_dest_name
			chmod 777 $TEST_RESULT_PATH/$valgrind_file_dest_name
		fi
	done

#copy the logs to analyze logs directory
mkdir $RESULT_PATH/$LOGS_DIRECTORY
log_name="Log_$TEST_NAME.log"
file_to_copy="$TEST_RESULT_PATH/$TEST_NAME.py.log"
echo "copying $file_to_copy to $RESULT_PATH/$LOGS_DIRECTORY/$log_name"
cat $file_to_copy >> $RESULT_PATH/$LOGS_DIRECTORY/$log_name

exit $RunTestExitStatus
