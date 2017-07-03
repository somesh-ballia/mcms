#!/bin/sh

# night test scripts framework

# Changes of Boris Legachov
# $1 is parameter with values 'short' or 'valgrind'
# $2 is mail address or 'NoMail'. If there is no parameter then mailAddr will be set as 'yossi.glick@Polycom.com'

#echo "param1="$param1
#echo "param2="$param2
#exit 1
# End of changes

################################
# ADD NEW SCRIPTS TO RUN HERE  #
################################
#VERSION=$1 # version e.g. 7.7  



MAIL=$1

if [ $# -gt 1 ];
then 
	DOCOMPILE=$2
else
	DOCOMPILE='YES' 
fi


if [ $# -gt 2 ];
then
	DOUPDATE=$3
else	
	DOUPDATE='NO' 
fi

if [ $# -gt 3 ];
then
        GROUPTORUN=$4
else
        GROUPTORUN=''
fi

if [ $# -gt 4 ];
then
        VALGRIDRUN=$5
else
        VALGRIDRUN='YES'
fi

if [ $# -gt 5 ];
then
        
        if [ -d $6 ]; then
        	CS_PATH=$6 
        	echo "$CS_PATH"
        	export CS_DIR=$6
        	IS_CS='YES'
        	rm $MCU_HOME_DIR/tmp/cs
        fi
        
else
        IS_CS='NO'
fi



echo "DOCOMPILE:$DOCOMPILE, DOUPDATE:$DOUPDATE, GROUPTORUN:$GROUPTORUN, VALGRIDRUN:$VALGRIDRUN."
if [[ ($DOCOMPILE == 'NO') && ($DOUPDATE == 'YES') ]];then
	echo "DOUPDATE cant be YES if DOCOMPILE is NO"
	exit
fi

VERSION='NA'

ulimit -c unlimited

CURRENT_BASELINE=""
LAST_BASELINE_FILE=night_test_last_baseline.txt
NIGHTTEST_PERSISTENT_LOG=night_test_persistent_log.txt

SOURCE="${BASH_SOURCE[0]}"
DIR_SCRIPT="$( dirname "$SOURCE" )"
echo "SOURCE:$SOURCE, DIR_SCRIPT:$DIR_SCRIPT"
if [ $DIR_SCRIPT != "Scripts" ]; then

	MainDir=`expr match "$DIR_SCRIPT" '^\(.*\)/Scripts'`
	if [ ! -d $MainDir ] ;
	then
		echo "$MainDir does not exist" 1>&2
		exit 1;
	fi

	echo "cd  $MainDir"

	cd  $MainDir
	rc=$?
	if [ $rc != 0 ] ; then
			echo "Failed to enter $MainDir" 1>&2

			exit 1
	fi	
else
	echo "We already in main directory"
fi

LOGS_DIRECTORY='log_files_for_analyze'

source Scripts/RunNightTestHelpers.sh

#if [ -n "$VERSION" ]  &&  [ -n "$MAIL" ]
if [ -n "$MAIL" ]
then 
   echo ""
else
   echo "usage: run_night_test.sh <email address>"   
	./AutoTestScripts/Tools/SendSimpleTextMail.py Yael.Azulay@polycom.com "Problem occured on night test running" "Wrong execution: Usage:<br>run_night_test.sh &lt;email address&gt;" 

   exit 1
fi

OUTDIR=TestResults

#remove current TestResults
if [ -d $OUTDIR ] ;
then
	
	echo "rm -rf $OUTDIR/*"
	rm -rf $OUTDIR/*
	
	rc=$?
	ExitIfFailed $rc  "Cannot remove TestResults direcroty" 
fi


IsSVN
isSVNVersion=$?

if [ $DOUPDATE == 'NO' ];
then
        echo "No need to update. mkdir $OUTDIR"
		mkdir $OUTDIR
else
		echo "Update CS if Needed"
	    if [ $IS_CS == 'YES' ];then		
			cd $CS_PATH
			if [ "$isSVNVersion" == 1 ]; then
				UpdateSVNCSView 
			else
				UpdateClearCaseCSView  
			fi			
			cd -
		fi
		if [ "$isSVNVersion" == 1 ]; then
			UpdateSVNWorkspace
		else
			UpdateClearCaseView
		fi		
fi

if [ "$isSVNVersion" == 1 ]; then
	SetSVNVersion	
	echo "SVN VERSION $CURRENT_BASELINE"
	
else
	SetClearCaseVersion
	echo "CLEAR CASE VERSION $CURRENT_BASELINE"
fi


echo "DOCOMPILE: $DOCOMPILE"
if [ $DOCOMPILE == "YES" ]; then

	echo "Compile CS if Needed"
	if [ $IS_CS == 'YES' ]
	then
		cd $CS_PATH
		make clean
		./csmake -C
		cd  -
	fi

	echo "Compiling"
#	sudo rmtmp.sh $MCU_HOME_DIR/tmp/*
#	make clean
#	make mrproper
#	make active

	./fast_clean.sh
	./make.sh
	rc=$?
	ExitIfFailed $rc "Compilation failed."

	echo "Compiling parser"
	cd "AutoTestScripts/Tools/McmsLogParser"
	./makeMcmsLogParser
	rc=$?
	ExitIfFailed $rc "Log mcms parser compilation failed."
	cd -
	
fi


httpFile=`file $(readlink  Bin.i32ptx/httpd)`
echo "httpFile $httpFile"
if [[ ! "$httpFile" =~ "ELF 32-bit" ]];
then
	echo "Http file is not 32 bit: $httpFile"
	SendFailedNightTestEmail "Wrong httpd type. Should be for 32 bit." 
	exit 1;
fi

if [ ! -d $OUTDIR ] ;
then
	echo "$OUTDIR does not exist" 1>&2
	SendFailedNightTestEmail "$OUTDIR was not created." 
	exit 1;

fi


RESULT_PATH='result' 
DeleteOldResulsIfQuotaExceeds
SetResultPath
IncreaseProcessFDLimit


#echo Starting night test ..
#grep MCMS-Carmel VERSION.LOG | tail -1
#echo scripts to run : $SCRIPTS_TO_RUN 
#echo

	AMIR_SCRIPTS="FullStatisticsTest HD_force_party_level UndefinedDialIn AddRemoveConfTemplateNew VideoConfWith40Participants ResourceSharedRsrcList AddDeleteEqService AddRemoveResNew SetPartyPivateLayout AutoLayout Hd1080Vsw HD GW_2 ISDN_MultiRate ISDN_ReconnectParty AwakeMrByUndef SpeakerChange AddRemoveMrNew EncryConf AddDeleteNewIvr PresentationMode ISDN_DirectDialIn ISDN_DialInDialOut Add20ConferenceNew AddRemoveConfWrongMonitorID AddRemovePartyConfMissing UpdateParty AutoRealVideo ReconnectParty UpdateProfile CheckLayoutInRoomSwitchMode ISDN_Move RmxMpmRx_ConfParty_MaxConferenceTest RmxMpmRx_ConfParty_MaxEQsTest RmxMpmRx_ConfParty_MaxMixParticipantsTest RmxMpmRx_ConfParty_MaxProfilesTest RmxMpmRx_ConfParty_MaxSipFactoriesTest RmxMpmRx_ConfParty_MaxVideoParticipantsTest RmxMpmRx_ConfParty_MaxVideoPartiesInConf RmxMpmRx_ConfParty_MaxVoipParticipantsTest RmxMpmRx_ConfParty_MaxVoipPartiesInConf RmxMpmRx_ConfParty_MaxMRsTest RmxMpmx_ConfParty_MaxConferenceTest RmxMpmx_ConfParty_MaxEQsTest RmxMpmx_ConfParty_MaxMixParticipantsTest RmxMpmx_ConfParty_MaxProfilesTest RmxMpmx_ConfParty_MaxSipFactoriesTest RmxMpmx_ConfParty_MaxVideoParticipantsTest RmxMpmx_ConfParty_MaxVideoPartiesInConf RmxMpmx_ConfParty_MaxVoipParticipantsTest RmxMpmx_ConfParty_MaxVoipPartiesInConf RmxMpmx_ConfParty_MaxMRsTest EncrypValidity EncryDialIn CheckForcedPartiesOnConfMrTmplRsrv ContentPresentation ContentHighestCommon H264_H263_Content"
	#removed: ISDN_MultyTypeConf, 
	
	AMIR_SCRIPTS+=" RmxMpmRx_ConfParty_ResourceResolutionSlider RmxMpmRx_ConfParty_ResourceTestForSVC ResourceResolutionSlider ResourceTestForSVC"
	
	SHIRA_SCRIPTS="addH323PartyWithH263Cap SipAutoRealVideo MixAutoRealVideo UndefinedSIPDialIn SipDialInToMR CheckTypesOfDialInCalls AutoRealVoip IPPartyMonitoring AutoRealVideoDialInMixedV4V6 AutoRealVideoV6 SipMuteAudioDirection SipMuteAudioinCpConf SipMuteAudioInactive SipMuteAudioPort SipMuteVideoPort AddDeleteFactoryWithSipParty AddSipBfcpDialIn CallsAndScpTestForSvcOnlyConf CallsAndScpTestForMixConf RmxMpmRx_CallsAndScpTestForMixConf_1080SVC SipDialInToMROfSvcOnly SipDialInToMROfMix AVCPartyDialInToSvcOnlyConf CheckLprCpConf"
	SHIRA_SCRIPTS+=" cs_simple_unit_tests"

	KOBI_SCRIPTS="TestMIBFile Faults_Test_5_cyclic_faults_after_startup ExternalDbOperations InstallTest BackupAndRestore TestBigCDRFileRetrieve TestMaxOpenHttpSockets AllUnicodeTests DowngradeToV7_8 TestExchangeModuleCfg TestLdapModule TestCdrService"

    #ORI_SCRIPTS="TestRestApi SoftMcuStartup SoftMcuMcuTypesEdge SoftMcuMcuTypesMFW"   
	ORI_SCRIPTS="TestRestApi TestRestSnmpMfw"

	SPECIFIC_SCRIPTS="AutoRealVideo ReconnectParty"

#	Conference
	CONF="FullStatisticsTest HD_force_party_level UndefinedDialIn AddRemoveConfTemplateNew VideoConfWith40Participants ResourceSharedRsrcList AddDeleteEqService AddRemoveResNew SetPartyPivateLayout AutoLayout Hd1080Vsw HD GW_2 ISDN_MultiRate ISDN_ReconnectParty AwakeMrByUndef SpeakerChange AddRemoveMrNew EncryConf AddDeleteNewIvr PresentationMode ISDN_DirectDialIn ISDN_DialInDialOut Add20ConferenceNew AddRemoveConfWrongMonitorID AddRemovePartyConfMissing UpdateParty AutoRealVideo ReconnectParty UpdateProfile CheckLayoutInRoomSwitchMode ISDN_Move RmxMpmRx_ConfParty_MaxConferenceTest RmxMpmRx_ConfParty_MaxEQsTest RmxMpmRx_ConfParty_MaxMixParticipantsTest RmxMpmRx_ConfParty_MaxProfilesTest RmxMpmRx_ConfParty_MaxSipFactoriesTest RmxMpmRx_ConfParty_MaxVideoParticipantsTest RmxMpmRx_ConfParty_MaxVideoPartiesInConf RmxMpmRx_ConfParty_MaxVoipParticipantsTest RmxMpmRx_ConfParty_MaxVoipPartiesInConf RmxMpmRx_ConfParty_MaxMRsTest RmxMpmx_ConfParty_MaxConferenceTest RmxMpmx_ConfParty_MaxEQsTest RmxMpmx_ConfParty_MaxMixParticipantsTest RmxMpmx_ConfParty_MaxProfilesTest RmxMpmx_ConfParty_MaxSipFactoriesTest RmxMpmx_ConfParty_MaxVideoParticipantsTest RmxMpmx_ConfParty_MaxVideoPartiesInConf RmxMpmx_ConfParty_MaxVoipParticipantsTest RmxMpmx_ConfParty_MaxVoipPartiesInConf RmxMpmx_ConfParty_MaxMRsTest EncrypValidity EncryDialIn CheckForcedPartiesOnConfMrTmplRsrv ContentPresentation ContentHighestCommon H264_H263_Content"
	CONF+=" RmxMpmRx_ConfParty_ResourceResolutionSlider RmxMpmRx_ConfParty_ResourceTestForSVC ResourceResolutionSlider ResourceTestForSVC"
	
#   Management

	MNGMNT="TestMIBFile Faults_Test_5_cyclic_faults_after_startup ExternalDbOperations InstallTest BackupAndRestore TestBigCDRFileRetrieve TestMaxOpenHttpSockets AllUnicodeTests DowngradeToV7_8 TestExchangeModuleCfg TestLdapModule TestCdrService"

# 	Call Control
	CALL_CONTROL="addH323PartyWithH263Cap SipAutoRealVideo MixAutoRealVideo UndefinedSIPDialIn SipDialInToMR CheckTypesOfDialInCalls AutoRealVoip IPPartyMonitoring AutoRealVideoDialInMixedV4V6 AutoRealVideoV6 SipMuteAudioDirection SipMuteAudioinCpConf SipMuteAudioInactive SipMuteAudioPort SipMuteVideoPort AddDeleteFactoryWithSipParty AddSipBfcpDialIn CallsAndScpTestForSvcOnlyConf CallsAndScpTestForMixConf RmxMpmRx_CallsAndScpTestForMixConf_1080SVC SipDialInToMROfSvcOnly SipDialInToMROfMix AVCPartyDialInToSvcOnlyConf CheckLprCpConf"
	CALL_CONTROL+=" cs_simple_unit_tests"

#   Framework
	FRAMEWORK="TestMIBFile" # ExternalDbOperations" # ReconnectParty" # AutoRealVideo ReconnectParty"

#   System
	#SYSTEM="TestRestApi SoftMcuStartup SoftMcuMcuTypesEdge SoftMcuMcuTypesMFW"
 SYSTEM="TestRestApi"

WriteFrameworkScritpsToFIle
WriteCallControlScritpsToFIle
WriteManegmentScritpsToFIle
WriteConfScritpsToFIle
WriteSystemScriptsToFile
Scripts/nt_results_db.py "init" $CURRENT_BASELINE


if [ "$GROUPTORUN" != "" ]
then
	if [ "$GROUPTORUN" == "CONF" ];then
		SCRIPTS_TO_RUN=$AMIR_SCRIPTS
	elif [ "$GROUPTORUN" == "PARTY" ];then
		SCRIPTS_TO_RUN=$SHIRA_SCRIPTS
	elif [ "$GROUPTORUN" == "MNGMT" ];then
		SCRIPTS_TO_RUN=$KOBI_SCRIPTS
	elif [ "$GROUPTORUN" == "SYSTEM" ];then
		SCRIPTS_TO_RUN=$ORI_SCRIPTS
	elif [ "$GROUPTORUN" == "SPECIFIC" ];then
		SCRIPTS_TO_RUN=$SPECIFIC_SCRIPTS
	elif [ "$GROUPTORUN" == "FRAMEWORK" ];then
		SCRIPTS_TO_RUN=$FRAMEWORK
	elif [ "$GROUPTORUN" == "NO" ];then
		SCRIPTS_TO_RUN=""
	else 
	        echo "GROUPTORUN is not valid $GROUPTORUN" 1>&2
	        SendFailedNightTestEmail "GROUPTORUN is not valid $GROUPTORUN" 
	        exit 1
	fi
fi

if [ "$SCRIPTS_TO_RUN" == "" ]
then    
	#run Management scripts
	#	GROUPTORUN = "Management";
	#SCRIPTS_TO_RUN=$MNGMNT
	#RunTestsScripts
	#GROUPTORUN = "CallControl";
	#SCRIPTS_TO_RUN=$CALL_CONTROL
	#RunTestsScripts
	#GROUPTORUN = "Conference";
	#SCRIPTS_TO_RUN=$CONF
	#RunTestsScripts
	#GROUPTORUN = "Framework";
	#SCRIPTS_TO_RUN=$FRAMEWORK
	SCRIPTS_TO_RUN="$AMIR_SCRIPTS $SHIRA_SCRIPTS $KOBI_SCRIPTS $ORI_SCRIPTS"
	GROUPTORUN="All"	
	#SCRIPTS_TO_RUN="TestCdrService"
	RunTestsScripts	

else

	for ScriptName in $SCRIPTS_TO_RUN  #Scripts/run_night_test2.sh: line 77: 18319 Terminated              sleep 1 ???? 
	   do
	      echo ---
	      date
	      echo -n "running " $ScriptName " script... "
	      mkdir $OUTDIR/$ScriptName
	
	    # (sleep 1; xterm -e tail -f $OUTDIR/$ScriptName/test_run.txt)&  
		sleep 1; 
	
		
	    if [[ $VALGRIDRUN == 'NO' ]];
	        then
	            echo -n "running " $ScriptName " with no valgrind "
	            USE_ALT_IP_SERVICE=VersionCfg/DefaultIPServiceListWithDNSWithProxyAndGk.xml Scripts/RunTest.sh Scripts/$ScriptName.py "" 40  >  $OUTDIR/$ScriptName/test_run.txt
	        else
			GetIsValgrindParamter "Scripts/$ScriptName.py"
			
			if [ $? == 0 ]	
			then		
				echo -n "running " $ScriptName " with no valgrind "
				USE_ALT_IP_SERVICE=VersionCfg/DefaultIPServiceListWithDNSWithProxyAndGk.xml Scripts/RunTest.sh Scripts/$ScriptName.py "" 40  >  $OUTDIR/$ScriptName/test_run.txt
			else
				echo -n "running " $ScriptName " under valgrind "
		                USE_ALT_IP_SERVICE=VersionCfg/DefaultIPServiceListWithDNSWithProxyAndGk.xml Scripts/RunTest.sh Scripts/$ScriptName.py valgrind 40 >  $OUTDIR/$ScriptName/test_run.txt
			fi
	    fi
	
	# get result from run test	
	
		echo -n "creating report... "  
	    Scripts/create_report.sh $ScriptName test_run.txt >>  $OUTDIR/$ScriptName/test_report.txt 
	    # killall xterm  
	
		log_name="Log_$ScriptName.log"
	
		file_to_copy="$OUTDIR/$ScriptName/$ScriptName.py.log"
		echo "copying $file_to_copy to  $RESULT_PATH/$LOGS_DIRECTORY/$log_name"
		cp $file_to_copy   $RESULT_PATH/$LOGS_DIRECTORY/$log_name
	    
	    if [ $IS_CS == 'YES' ];then
	    			
		   if [  -d "$MCU_HOME_DIR/cs" ]; then
	  			cd $MCU_HOME_DIR/cs
	  				echo "copying cs log files to $OUTDIR/$ScriptName/"
	  				cp -R logs/cs1/ "$OUTDIR/$ScriptName/"
	  				rm -Rf logs/cs1/*
	  				
	  				cp logs/output/cs_console.txt  "$OUTDIR/$ScriptName/"
					rm logs/output/cs_console.txt   				
	  			
	  			cd -
			fi
	  
		fi
		 
		mv "$OUTDIR/$ScriptName" "$RESULT_PATH/$DAYDIR"
		
		echo $ScriptName "done. "
	      
	   done
fi

Scripts/SummarizeResults.sh  "$RESULT_PATH" "$DAYDIR" "$LOGS_DIRECTORY" "$CURRENT_BASELINE" "$MAIL" > $RESULT_PATH/report_dump.log

#cd  $MainDir

if [ CURRENT_BASELINE!="" ];then
	echo $CURRENT_BASELINE > $LAST_BASELINE_FILE
fi

echo
echo end of night test.
date
echo `date` " End night test" >> $NIGHTTEST_PERSISTENT_LOG



