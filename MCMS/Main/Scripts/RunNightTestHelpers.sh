#!/bin/sh


SendFailedNightTestEmail()
{
	./AutoTestScripts/Tools/SendSimpleTextMail.py $MAIL "Problem occured on night test running" "$1"	
}

GetIsValgrindParamter()
{
        TEST_FNAME=$1
        echo "grep \#-LONG_SCRIPT_TYPE $TEST_FNAME"
        IS_LONG=`grep \#-LONG_SCRIPT_TYPE $TEST_FNAME`

        RES=$?
        if [ $RES != "0" ]; then
                if [ $RES != "1" ]; then

                    echo  unable to read file $TEST_FNAME
                        SendFailedNightTestEmail "unable to read file $TEST_FNAME"
                    exit 1
                fi
        fi

        #echo "IS_LONG=$IS_LONG"

        # add valgrind if a test has LONG_SCRIPT_TYPE
        if [[ -z $IS_LONG ]]; then
                return 0
        else

                return 1
        fi
}

ExitIfFailed()
{
	if [ $1 != 0 ] ; then
		    echo "Failure: $2" 1>&2
			SendFailedNightTestEmail "$2"
		    exit 1
	fi
}

DeleteOldResulsIfQuotaExceeds()
{
	currentQuota=`quota -w  | grep nethome | awk '{print $2}'`
	rc=$?
	ExitIfFailed $rc "Failed get Quota"

	limitQuota=`quota -w  | grep nethome | awk '{print $3}'`

	if [ $limitQuota -le 1 ];
	then	
		ExitIfFailed 10 "Failed to get Quota limitation (limitQuota $limitQuota)"
	fi
	
	leftQuotaPrecentage=`expr "$currentQuota" \* 100`
	leftQuotaPrecentage=`expr "$leftQuotaPrecentage" / "$limitQuota"`

	if [ $leftQuotaPrecentage -gt 89 ];
	then	
		echo "Deletinig old directories" >> $NIGHTTEST_PERSISTENT_LOG
		echo "deleting directories older then 20 days. currentQuota $currentQuota limitQuota $limitQuota"
		find . -maxdepth 1 -name 'Result-*-*'  -mtime +20 -exec rm -rf {} \;
		rc=$?
		ExitIfFailed $rc "Failed to delete old result directories"
	fi

}

SetResultPath()
{
	declare -i maxDirNum=0
	declare -i currentDirNum=0

	resultDirs=`ls | grep Result-`

	for  resultDir in $resultDirs
	do
		currentDirNum=`expr match "$resultDir" '^Result-\([0-9]*\)-'`

		#echo "resultDir $resultDir currentDirNum $currentDirNum maxDirNum $maxDirNum"
		if [ $currentDirNum -gt $maxDirNum ];
		then
				maxDirNum=currentDirNum
		fi
	done
	maxDirNum=maxDirNum+1

	dateExtension=`date +"%d.%m.%Y"`
	RESULT_PATH="Result-$maxDirNum-$dateExtension"

	echo "creating result directory: $resultDir"
	mkdir $RESULT_PATH
	rc=$?
	ExitIfFailed $rc "Failed to create result directory $RESULT_PATH"

	DAYDIR=`date +%A`
	echo "mkdir $RESULT_PATH/$DAYDIR"
	mkdir $RESULT_PATH/$DAYDIR
	rc=$?
	ExitIfFailed $rc  "Failed to create day result directory $RESULT_PATH/$DAYDIR"

	echo "mkdir $RESULT_PATH/$LOGS_DIRECTORY"
	mkdir $RESULT_PATH/$LOGS_DIRECTORY
	rc=$?
	ExitIfFailed $rc  "Failed to create log result directory $RESULT_PATH/$LOGS_DIRECTORY"
		
}


IsSVN()
{
	svninfo=`svn info`
	rc=$?
	#if [[ $svninfo =~ "is not a working copy" ]];
	if [ $rc != 0 ] || [[ $svninfo == "" ]];
	then
			   echo "NOT SVN"
			return 0
	else
		echo "SVN!!!!!!"
		return 1
	fi
}

WriteFrameworkScritpsToFIle()
{
	for Script in $FRAMEWORK
	do
		
		echo $Script >>  $OUTDIR/framework_scripts.txt
	done
}

WriteSystemScriptsToFile()
{
	for Script in $SYSTEM
	do
		
		echo $Script >>  $OUTDIR/system_scripts.txt
	done
}

WriteCallControlScritpsToFIle()
{
	for Script in $CALL_CONTROL
	do
		
		echo $Script >>  $OUTDIR/call_control_scripts.txt
	done
}

WriteManegmentScritpsToFIle()
{
	for Script in $MNGMNT
	do
		
		echo $Script >>  $OUTDIR/mngnt_scripts.txt
	done
}

WriteConfScritpsToFIle()
{
	for Script in $CONF
	do
		
		echo $Script >>  $OUTDIR/conf_scripts.txt
	done
}

UpdateSVNWorkspace()
{
    echo "UpdateSVNWorkspace"
#svnversion
#	svn revert -R .
#svn info |grep Revision: |cut -c11-
	
	CURRENT_BASELINE=""
	LAST_BASELINE_FILE=night_test_last_baseline.txt
	#svn update --force
	
	svn revert ../../ --recursive
	svn update ../../
    rc=$?
	ExitIfFailed $rc "Failed to update SVN workspace view" 			

}

SetSVNVersion()
{
    echo "SetSVNVersion"
    	
	CURRENT_BASELINE=""
	LAST_BASELINE_FILE=night_test_last_baseline.txt

	CURRENT_BASELINE=`svnversion`
	

	if [ -f $LAST_BASELINE_FILE ] ;	then
		LAST_BASELINE=`cat night_test_last_baseline.txt`
		if [ "$LAST_BASELINE" != "" ] ; then
			if [ "$LAST_BASELINE" == $CURRENT_BASELINE ]; then

				echo "Current SVN revision is identical to last revision: $CURRENT_BASELINE "

				./AutoTestScripts/Tools/SendSimpleTextMail.py $MAIL "Night test: Current SVN revision is identical to last revision. Test was not run. " "Current SVN revision is identical to last revision: $CURRENT_BASELINE.<br>Test was not run "
				exit 1;
			fi

			echo "Last revision $LAST_BASELINE  current revision $CURRENT_BASELINE "	

		fi
	else
		echo "Last SVN revision file not exits. CURRENT_BASELINE  $CURRENT_BASELINE "
	fi
	echo "" > $LAST_BASELINE_FILE	
}

UpdateClearCaseView()
{
	echo "UpdateClearCaseView"
	CURRENT_BASELINE=""
	LAST_BASELINE_FILE=night_test_last_baseline.txt

	#if [ -z `cleartool lsview -s -cview | grep int` ];
	#then
	#	    echo "not integration view. mkdir $OUTDIR"
	#		mkdir $OUTDIR
	#else
    #echo "Integration view - updating"
	CLEARTOOLPATH=cleartool
    $CLEARTOOLPATH update -force -overwrite
    rc=$?
	if [ $rc != 0 ] ; then
		CLEARTOOLPATH=/usr/atria/bin/cleartool
	    $CLEARTOOLPATH update -force -overwrite
	fi
    rc=$?
	ExitIfFailed $rc "Failed to update integration view: $rc" 
}

SetClearCaseVersion()
{
	echo "SetClearCaseVersion"
	CURRENT_BASELINE=""		
	LAST_BASELINE_FILE=night_test_last_baseline.txt
						
	CLEARTOOLPATH=cleartool	    
	CURRENT_BASELINE=`$CLEARTOOLPATH lsbl -s -cview | tail -1`		
    rc=$?    		
    
	if [ $rc != 0 ] || [[ $CURRENT_BASELINE == "" ]]; then		
		CLEARTOOLPATH=/usr/atria/bin/cleartool
	    CURRENT_BASELINE=`$CLEARTOOLPATH lsbl -s -cview | tail -1`
	fi
	
	if [ -f $LAST_BASELINE_FILE ] ;	then
		LAST_BASELINE=`cat night_test_last_baseline.txt`
		if [ $LAST_BASELINE != "" ] ; then
			if [ $LAST_BASELINE == $CURRENT_BASELINE ]; then

				echo "Current baseline is identical to last baseline: $CURRENT_BASELINE "

				./AutoTestScripts/Tools/SendSimpleTextMail.py $MAIL "Night test: Current baseline is identical to last baseline. Test was not run. " "Current baseline is identical to last baseline: $CURRENT_BASELINE.<br>Test was not run "
				exit 1;
			fi

			echo "Last baseline $LAST_BASELINE  current baseline $CURRENT_BASELINE "	

		fi
	else
		echo "Last Baseline file not exits. CURRENT_BASELINE  $CURRENT_BASELINE "
	fi
	echo "" > $LAST_BASELINE_FILE
}

UpdateSVNCSView()
{
    echo "UpdateSVNCSView"
    
    svn revert . --recursive    
	svn update 
			
    rc=$?
	ExitIfFailed $rc "Failed to update SVN workspace view" 			
}

UpdateClearCaseCSView()
{
    #echo "Integration view - updating"
	CLEARTOOLPATH=cleartool
    $CLEARTOOLPATH update -force -overwrite
    rc=$?
	if [ $rc != 0 ] ; then
		CLEARTOOLPATH=/usr/atria/bin/cleartool
	    $CLEARTOOLPATH update -force -overwrite
	fi
}


IncreaseProcessFDLimit()
{
	
	currentLimit=`ulimit -n`
	if [ $currentLimit -ne 3072 ];
	then
        	ulimit -n 3072
        	rc=$?
      		ExitIfFailed $rc "Failed to increase FD limit to 3072"

	fi

	echo "FD limit is " `ulimit -n`

}


RunTestsScripts()
{
 
echo "running group name $GROUPTORUN"
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
            USE_ALT_IP_SERVICE=VersionCfg/DefaultIPServiceListWithDNSWithProxyAndGk.xml Scripts/RunTest.sh Scripts/$ScriptName.py "" 40  $GROUPTORUN >  $OUTDIR/$ScriptName/test_run.txt
        else
		GetIsValgrindParamter "Scripts/$ScriptName.py"
		
		if [ $? == 0 ]	
		then		
			echo -n "running " $ScriptName " with no valgrind "
			USE_ALT_IP_SERVICE=VersionCfg/DefaultIPServiceListWithDNSWithProxyAndGk.xml Scripts/RunTest.sh Scripts/$ScriptName.py "" 40  $GROUPTORUN >  $OUTDIR/$ScriptName/test_run.txt
		else
			echo -n "running " $ScriptName " under valgrind "
	                USE_ALT_IP_SERVICE=VersionCfg/DefaultIPServiceListWithDNSWithProxyAndGk.xml Scripts/RunTest.sh Scripts/$ScriptName.py valgrind 40 $GROUPTORUN >  $OUTDIR/$ScriptName/test_run.txt
		fi
    fi

	# get result from run test	

	echo -n "creating report... "  
     Scripts/create_report.sh $ScriptName test_run.txt >>  $OUTDIR/$ScriptName/test_report.txt 
     # killall xterm  

	log_name="Log_$ScriptName.log"

	file_to_copy="$OUTDIR/$ScriptName/$ScriptName.py.log"
	echo "copying $file_to_copy to  $OUTDIR/$LOGS_DIRECTORY/$log_name"
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
}
