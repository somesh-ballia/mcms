#!/bin/sh

echo "*************************************"
echo "Running Single Stream developer tests"
echo "*************************************"

blue=$(tput setaf 4)
normal=$(tput sgr0)
magenta=$(tput setaf 5)
green=$(tput setaf 2)

################################ CM Add-On: Used to create uniqur log file ##################################
SVN_URL=`svn info | grep "URL: h" | gawk '{print $2}'`

echo "SVN_URL: ${SVN_URL}"

export REPO=`echo $SVN_URL | awk -F"/" '{print $5}'`
export VERSION=`echo $SVN_URL | awk -F"/" '{print $6}'`
export BUILD_TYPE=`echo $SVN_URL | awk -F"/" '{print $7}'`

if [ ${BUILD_TYPE} == "branches" ]
then
       export BRANCH=`echo $SVN_URL | awk -F"/" '{print $8}'`
       export LEAF="${VERSION}_${BRANCH}"
else
       export BRANCH=""
       export LEAF="$VERSION"
fi
#############################################################################################################

local_storage=/misc/SST_LOGS/$USER/${LEAF}
mkdir -p $local_storage
chmod -R 777 /misc/SST_LOGS/$USER > /dev/null 2>&1
static_test_log_path=$local_storage/StaticTest.log
simulation_test_log_path=$local_storage/SimulationTest.log
pre_delivery_log_path=$local_storage/PreDeliverySmoke.log
SingleStreamTestResult=$local_storage/SingleStreamTestResult.log

static_test_result="no_run"
static_test_duration=""
simulation_test_result="no_run"
simulation_test_duration=""
real_ep_test_result="no_run"
real_ep_test_duration=""

#MIN_MEMORY=12198100
MIN_CORES=3

RECOMMANDED_MEMORY=16198100
RECOMMANDED_CORES=8

export smokeGhostMode=${SmokeGhostMode:="false"}
if  [[ "$1" == *run_pre_only* ]] ; then
	export smokeGhostMode=false;
fi


################################ HELP FOR PRE DELIVERY OPTIONS ##################################
if  [[ "$1" == *help* ]] ; then
echo "Test usage: SingleStreamTest.sh COMMAND or null"
	echo "command options:"
	echo "#####################"
	echo "1. no command - if no argument is provided then it will 1.Stop soft mcu if its up, 2.Run static test 3.Run RMX simulation, 4.Start soft mcu, 5. Run pre smoke"
	echo "2. run_pre_only - Run only pre delivery test only - Note that youll have to start soft mcu mannualy before run"
	echo "3. static_only -  Run only static test "
	echo "4. start_vm_run_pre_only - 1.Start foft mcu, 2.Run only pre delivery test"
        echo "5. static_only_no_valgrind -  Run only static test, without ConfParty under valgrind.(By default, we will run ConfParty under valgrind for static test.)"
	echo "For more info checkout this wiki http://isrmatlab/wikimedia/index.php/PreDelivery_Smoke_Troubleshooting"
	exit
fi


################################ PROCESS STATIC VALGRIND ##################################
if [[ "$1" == "static_only_no_valgrind" ]] ; then
        export VALGRIND_PROCESS_FOR_STATIC=""
else
        export VALGRIND_PROCESS_FOR_STATIC="ConfParty"
fi

############################### CHECK MEMORY AND CPU REQUIRMENTS ###################
if [[ "$1" != *"static_only"* ]] ; then

	RAM_Size=`free | head -2 | tail -1 | tr -s ' ' | cut -d' ' -f2`
	CPU_Num=`nproc`

	if (( $RAM_Size < $RECOMMANDED_MEMORY )); then
		echo  -e ${magenta}"Your available memory (" $RAM_Size ") is lower than recommended memory for this test (recommended" $RECOMMANDED_MEMORY ")" ${normal}
		echo "Are you sure you want to continue ? (y,n)"
		read continue_low_memory
		if [[ "$continue_low_memory" == "n" ]] ; then
			exit
		fi	
	fi

	#if (( $CPU_Num < $MIN_CORES)); then
	#	echo  -e ${magenta}"Two cores is not enough cores to run test (Three cores min and eight cores recommnaded) !!" ${normal}
	#	exit
	#fi

	if (( $CPU_Num < $RECOMMANDED_CORES )); then
		echo  -e ${magenta}"Number of cores" $CPU_Num " is lower than recommended for this test (recommended " $RECOMMANDED_CORES " cores)" ${normal}
		echo "Are you sure you want to continue ? (y,n)"
		read continue_low_cpu
		if [[ "$continue_low_cpu" == "n" ]] ; then
			exit
		fi	
	fi

fi

################################ CHECK KS VERSION ##################################
Current_KS=`grep 'Version: ' /etc/ks_ver.txt | cut -d':' -f3 | tr -d ' ' | cut -d'.' -f4 | cut -d'_' -f1`
Required_KS=`cat ks_ver.txt | cut -d'.' -f4`
	
if (( ${Required_KS} <= ${Current_KS} )); then
	echo -n -e ${green}"KS OK "${normal}
        echo "( $Current_KS )"
else
        echo  -e ${magenta}"KS Check Fail !!! "${normal}
        echo "Your KS version ( $Current_KS ) is not updated to latest version ( $Required_KS ) "
        echo "Are you sure you want to continue ? (y,n)"
        read continue_ks_not_updated
        if [[ "$continue_ks_not_updated" == "n" ]] ; then
                exit
        fi
fi


############################### READ OFFICIAL DIR ################################################
	
#if OFFICIAL_DIR already has a value display it and ask if to replace

if [[ ! -d $OFFICIAL_DIR || "$OFFICIAL_DIR" == "" ]] ; then
	SVN_URL=`svn info | grep URL | head -1 | gawk '{print $NF}'`

	export REPO=`echo $SVN_URL | awk -F"/" '{print $5}'`
	export VERSION=`echo $SVN_URL | awk -F"/" '{print $6}'`
	export BUILD_TYPE=`echo $SVN_URL | awk -F"/" '{print $7}'`

	if [ ${BUILD_TYPE} == "branches" ]
	then
       		export BRANCH=`echo $SVN_URL | awk -F"/" '{print $8}'`
		export BRANCH="${BRANCH}/"
	else
       		export BRANCH=""
	fi
	
	export OFFICIAL_DIR="/Carmel-Versions/SVN/Builds/$REPO/$VERSION/${BUILD_TYPE}/${BRANCH}last/"
fi

read_new_path="n"
if [[ "$OFFICIAL_DIR" != "" ]] ; then
	echo "current version path is :"  $OFFICIAL_DIR " Are you sure you want to continue (y-continue, n-enter new path)? [y,n]"
	read read_new_path
fi

if [[ "$read_new_path" == "n" ]] ; then
	echo -n "Enter OFFICIAL DIR (for example. on stream 100 /Carmel-Versions/SVN/Builds/prod/Main/trunk/RMX_x.x.x.x/ or on stream 500 /Carmel-Versions/SVN/Builds/prod/Main/branches/500/last) and press [ENTER]:"
	read OFFICIAL_DIR
fi
       
if [[ ! -d $OFFICIAL_DIR ]] ; then
       	echo "Invalid OFFICIAL_DIR : $OFFICIAL_DIR, please check."
        exit
fi

export OFFICIAL_DIR

echo "###########  COMPONENTS VERSION PATH ##########################################"
echo "OFFICIAL_DIR="$OFFICIAL_DIR
if [[ "$MCMS_DIR" == "" ]] ; then
	echo "MCMS_DIR="$OFFICIAL_DIR
else 
	echo "MCMS_DIR="$MCMS_DIR
fi
if [[ "$CS_DIR" == "" ]] ; then
	echo "CS_DIR="$OFFICIAL_DIR
else 
	echo "CS_DIR="$CS_DIR
fi
if [[ "$ENGINE_DIR" == "" ]] ; then
	echo "ENGINE_DIR="$OFFICIAL_DIR
else 
	echo "ENGINE_DIR="$ENGINE_DIR
fi
if [[ "$MRMX_DIR" == "" ]] ; then
	echo "MRMX_DIR="$OFFICIAL_DIR
else 
	echo "MRMX_DIR="$MRMX_DIR
fi
if [[ "$MPMX_DIR" == "" ]] ; then
	echo "MPMX_DIR="$OFFICIAL_DIR
else 
	echo "MPMX_DIR="$MPMX_DIR
fi
echo "#################################################################################"

echo "Are you sure you want to continue ? (y,n)"
	read continue_components_version
	if [[ "$continue_components_version" == "n" ]] ; then
		exit
	fi	

sleep 3


######## CALL PRE DELIVERY TEST CONFIGURATION #################################

. ./Scripts/PreDeliverySmoke.sh "Initialize"
noRunPreDeliverySmoke=false
if [[ "$smokeGhostMode" != true ]] ; then
	if [ $PreDeliverySmokeStatus == "FAILED" ] ; then
		noRunPreDeliverySmoke=true
	fi
fi

############ START STATIC & SIMULATION TESTS ##############################
HaveValgrindError() {
   LEAK="`cat $1 | grep "definitely lost:" | grep -v "definitely lost: 0 bytes"`";
   HAVE_ERROR="NO"
   if [ "$LEAK" != "" ]
   then
     HAVE_ERROR="YES"
     echo "LEAK: "$LEAK",file:"$1;
   fi
   
   #echo ===$1 Memeory Errors===
   MEMERR="`cat $1 | grep "ERROR SUMMARY:" | grep -v "0 contexts"`";
   if [ "$MEMERR" != "" ]
   then
     HAVE_ERROR="YES"
     echo "MEMORY ERROR: "$MEMERR",file:"$1;
   fi

   #echo ===$1 Core Dumps===
   COREDUMP="`cat $1 | grep "SystemCoreDump"`";
   if [ "$COREDUMP" != "" ]
   then
     HAVE_ERROR="YES"
     echo "CORE DUMP: "$COREDUMP",file:"$1;
   fi

   #echo ===$1 Process terminating===
   SIGSEGV="`cat $1 | grep "(SIGSEGV)"`";
   if [ "$SIGSEGV" != "" ]
   then
     HAVE_ERROR="YES"
     echo "SIGSEGV: "$SIGSEGV",file:"$1;
   fi

   if [[ $HAVE_ERROR == "YES" ]]; then
     #echo "Have errors in $1"
     return 1;
   else
     return 0;
   fi
}
#if [[ true == false ]] ; then
if [[ $# -eq 0 ]] ||  [[ $# -eq 1  &&  ("$1" != *run_pre_only*)  ]] ; then
	make active
	if [[ ! -d LogFiles ]]; then
		mkdir LogFiles
	fi
	if [[ -f ./Scripts/FixMIBFile.py ]]; then
		echo "Applying MIB fix"
		if [[ ! -e python ]]; then
			ln -s /opt/python2.5_snmp $MCU_HOME_DIR/mcms/python
		fi
		./Scripts/FixMIBFile.py
	fi

	if [[ "$MCMS_DIR" != "" ]] ; then

		#make test_scripts && ./Scripts/soft.sh test

		######################## SIMULATION TEST ########################################
		if  [ $? -eq 0 ] ; then
		if [[ "$1" != *"static_only"* ]] ; then
			T="$(date +%s%N)"
			echo "Run RMX simulation test"

			cd ${MCMS_DIR}
			make test_scripts    >&1 | tee $simulation_test_log_path

			#static_test_result=$?
			echo "SIMULATION TEST RETURN CODE:" $?	
			simulation_test_result="Fail"
			if [ $? -eq 0 ] ; then
				simulation_test_result="Pass"
			fi 

		        SIMULATION_TEST_ERROR="`cat $simulation_test_log_path | grep "Scripts/AllVersionTestsBreezeModeCP.sh Test FAILED"`"
		        if [ "$SIMULATION_TEST_ERROR" != "" ]; then
		                simulation_test_result="Fail"
		                echo "FOUND SIMULATION TEST FAIL LOG!";
		        fi
		        
		        if [[ "$VALGRIND_PROCESS_FOR_STATIC" == "ConfParty" ]]; then
		                  echo "SIMULATION TEST : checking valgrind errors..."
		                  HaveValgrindError "TestResults/ConfParty"
		                  if [[ "$?" -eq "1" ]]; then
		                    echo ""  >> $simulation_test_log_path
		                    echo "||"  >> $simulation_test_log_path
		                    echo "||   SIMULATION TEST failed due to finding valgrind errors for ConfParty, see more information below."  >> $simulation_test_log_path
		                    echo "||"  >> $simulation_test_log_path
		                    echo ""  >> $simulation_test_log_path
		                    cat TestResults/ConfParty >> $simulation_test_log_path
		                            
		                    simulation_test_result="Fail"
		                    echo ""
		                    echo "SIMULATION TEST FAILED: we have valgrind errors to fix!"
		                    echo ""
		                  fi
		        fi

			T="$(($(date +%s%N)-T))"
			S="$((T/1000000000))"
			simulation_test_duration=$(printf "%02d:%02d" "$((S/60%60))" "$((S%60))")
		fi
		fi

		######################## STATIC TEST ###########################################
		T="$(date +%s%N)"
		echo "Run Static test"

		./Scripts/soft.sh test  >&1 | tee $static_test_log_path
		echo "STATIC TEST RETURN CODE:" $?
		static_test_result="Fail"
		if  grep "RUNNING TEST(S) SUCCEEDED" $static_test_log_path  ; then
			static_test_result="Pass"
		fi

		######################## STOPING MCU ###########################################
		echo "Test is done...Stopping soft mcu..."
		./Scripts/soft.sh stop >& /dev/null


		T="$(($(date +%s%N)-T))"
		S="$((T/1000000000))"
		static_test_duration=$(printf "%02d:%02d" "$((S/60%60))" "$((S%60))")
	fi #end if MCMS_DIR!=""	
fi #endif check if not run pre onl


############ START PRE DELIVERY TEST ###################
if [[ $noRunPreDeliverySmoke != true ]] ; then
		T="$(date +%s%N)"	

		. ./Scripts/PreDeliverySmoke.sh "RunTest" >&1 | tee $pre_delivery_log_path		
		if [[ "$smokeGhostMode" != true ]] ; then
			 real_ep_test_result="Fail"
			 if  grep "## Test Passed! ##" $pre_delivery_log_path  ; then
				real_ep_test_result="Pass"
			 fi
			 if  grep "## Test Failed! ##" $pre_delivery_log_path  ; then
				real_ep_test_result="Fail"
			 fi
		fi

		T="$(($(date +%s%N)-T))"
		S="$((T/1000000000))"
		real_ep_test_duration=$(printf "%02d:%02d" "$((S/60%60))" "$((S%60))")
fi


###################### CREATE SUMMARY REPORT ####################################################
static_color=$blue
simulation_color=$blue
pre_color=$blue

#### Pilot ####
echo Pass > $SingleStreamTestResult ; chmod 664 $SingleStreamTestResult 
if [[ "$static_test_result" == "Pass" ]] ; then
	static_color=$green
elif  [[ $static_test_result == "Fail" ]] ; then
	 static_color=$magenta
	 echo Fail > $SingleStreamTestResult
	 else echo NotFullyRun > $SingleStreamTestResult
fi

if [[ "$simulation_test_result" == "Pass" ]] ; then
	simulation_color=$green
elif [[ "$simulation_test_result" == "Fail" ]] ; then
	 simulation_color=$magenta
	 echo Fail > $SingleStreamTestResult
	 else echo NotFullyRun > $SingleStreamTestResult
fi

if [[ "$real_ep_test_result" == "Pass" ]] ; then
	 pre_color=$green
elif [[ "$real_ep_test_result" == "Fail" ]] ; then
         pre_color=$magenta
	 echo Fail > $SingleStreamTestResult
	 else echo NotFullyRun > $SingleStreamTestResult
fi

chmod -R 777 /misc/SST_LOGS/$USER > /dev/null 2>&1

clear


echo  "########## TESTS RESULTS ##################"
echo "-----------------------------------------------------------------------------------------------------------------"
printf "| %-20s| %-14s| %-10s| %-60s|\n" "TEST NAME"  "TEST RESULT" "DURATION"   "LOG PATH"
echo "-----------------------------------------------------------------------------------------------------------------"
printf "| %-20s| ${simulation_color}%-14s${normal}| %-10s| %-60s|\n"  "RMX Simulation test"  "$simulation_test_result"  "$simulation_test_duration"  "$simulation_test_log_path"
printf "| %-20s| ${static_color}%-14s${normal}| %-10s| %-60s|\n"      "Static test"      "$static_test_result" "$static_test_duration"   "$static_test_log_path"
printf "| %-20s| ${pre_color}%-14s${normal}| %-10s| %-60s|\n"         "Real EP test"     "$real_ep_test_result"   "$real_ep_test_duration" "$pre_delivery_log_path"
echo "-----------------------------------------------------------------------------------------------------------------"

echo "######################################"
echo "KS number:"$Current_KS
echo "Date     : $(date)"
echo "######################################"


#########################################################################################


if [[ ("$1" == *static_only*) ]] ; then
	if [[ "$simulation_test_result" != "Pass" ]] || [[ "$static_test_result" != "Pass" ]] ; then
		./AutoTestScripts/Tools/SendSimpleTextMail.py "kobi.ginon@polycom.co.il,shimon.tanny@polycom.co.il,uri.avni@polycom.co.il" "User: `whoami` ----   has failed to Run SingleStreamTest Please watch this delivery!   pwd: `pwd`"	 "<br> Single Stream Results <br> ------------------ <br> `cat $simulation_test_log_path` <br><br><br> `cat $static_test_log_path` "
	fi
fi
