#!/bin/bash
# scriptname - NightTestsDefs.sh

#softMcuTestResultsDir=TestResults in SoftMcuTests.sh
export softMcuTestResultsDir=$MCU_HOME_DIR/mcms/LogFiles/softMcuTestResults

. ./Scripts/CommonTestsDefs.sh

#
# List of public functions of the file
#
# Init_NightTest  ===============================
Init_NightTest (){
  cd $MCMS_DIR
  export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin
  ulimit -c unlimited
  export CLEAN_CFG=YES
}

# Return_From_NightTest  ========================
Return_From_NightTest (){
  ret=$1
  
  #echo $@

  #Test was run as alone
  if [ "$2" == "true" ] 
  then
	EndMCMSPythonTests
  else
	MoveAllFilesToRelativePaths
  fi

  if [ $ret -eq 0 ]
  then 
    echo $TXTBOLD$COLORWHITE$BGCOLORGREEN
    echo "********** RUNNING TEST(S) SUCCEEDED **************"
    echo $TXTRESET
  else
    echo $TXTBOLD$COLORWHITE$BGCOLORRED
    echo "********** RUNNING TEST(S) FAILED ($ret) **********"
    echo $TXTRESET
  fi
  return $ret
}

# RunOneSetOfTests  =============================
RunOneSetOfTests  (){
  retCode=0
  CreateDirTreeForOneTest $2

  START=$(date +%s)
  #$1 Return_From_NightTest false &>$UDT_DirRsltFiles/stdout$2.log || retCode=$?
  $1 Return_From_NightTest false || retCode=$?
  #echo "$1 Return_From_NightTest false" || retCode=$?
  END=$(date +%s)
  DURATION=$(($END-$START))

  PrintHTMLTblEntry $retCode $2 "$3" $DURATION
  if [ $retCode -eq 0 ]
  then 
	Return_From_NightTest 0 false
  fi
}

# SendMail  =====================================
SendMail (){
MAILSRV_TYPENAME=${MAILSRV_TYPENAME-false}
MAILFROM_ADDR=${MAILFROM_ADDR-false}
MAILTO_ADDR=${MAILTO_ADDR-false}

if [ $MAILSRV_TYPENAME == false -o $MAILFROM_ADDR == false -o $MAILTO_ADDR == false ]
then
	echo $TXTBOLD$COLORWHITE$BGCOLORBROWN
	echo "##############################################################"
	echo "#####  night test mail has not been sent !!!"
	echo "#####  html output file: $1 "
	echo "##############################################################"
	echo $TXTRESET
else
	(
	echo "From: $MAILFROM_ADDR "
	echo "To: $MAILTO_ADDR "
	echo "MIME-Version: 1.0"
	echo "Content-Type: multipart/alternative; " 
	echo " boundary=some.unique.value.$MAILSRV_TYPENAME"
	echo "Subject: Night Tests Reults." 
	echo "" 
	echo "This is a MIME-encapsulated message" 
	echo "" 
	echo "--some.unique.value.$MAILSRV_TYPENAME" 
	echo "Content-Type: text/html" 
	echo "" 
	cat $1
	echo "--some.unique.value.$MAILSRV_TYPENAME"
	) | /usr/sbin/sendmail -t
fi
}

# CreateDir  ====================================
CreateDir (){
  retCode=0
  #echo $BGCOLORCYAN"CreateDir: $1 is being created"$TXTRESET
  mkdir -p -m 700 $1 2>/dev/null || retCode=$?

  if [ $retCode -ne 0 ]
  then
	echo $TXTBOLD$COLORWHITE$BGCOLORRED
	echo "Create directory '$1' has been failed with err $retCode"
	echo $TXTRESET
	return $retCode
  fi
}

# CreateOneStreamSubDirs  =======================
CreateOneStreamSubDirs (){
  cd $UDT_Directory/$3
  CreateDir $1 || exit $?
  cd $1

  # Run 'Create?SetSubdirectories' with parameter
  $2 $3
}

# CreateDirTreeForOneTest  ======================
CreateDirTreeForOneTest (){
  CurrentPath=`pwd`
  cd $UDT_Directory
  CreateDir $1 || exit $?
  CreateOneStreamSubDirs $MCMS_Version Create_MCMS_SetSubdirectories $1
  CreateOneStreamSubDirs $EngineMRM_Version Create_EngineMRM_SetSubdirectories $1
  CreateOneStreamSubDirs $MRMX_Version Create_MRMX_SetSubdirectories $1
  CreateOneStreamSubDirs $MPMX_Version Create_MPMX_SetSubdirectories $1
  CreateOneStreamSubDirs $CS_Version Create_CS_SetSubdirectories $1

  # Additional directories
  Dir_ResultFiles="ResultFiles"
  CurTestRootDir=$UDT_Directory/$1
  cd $CurTestRootDir
  CreateDir $Dir_ResultFiles || exit $?
  export UDT_DirRsltFiles=$UDT_Directory/$1/$Dir_ResultFiles
  echo $COLORCYAN"UDT_DirRsltFiles: $COLORBROWN$UDT_DirRsltFiles"$TXTRESET

  cd $CurrentPath
}

# MakeExportForStream  ==========================
MakeExportForStream (){
  cd $1
  strVersion=$($2 $3)
  export $3_Version=$strVersion
  echo $1
  echo $COLORMGNTA"$3_Version='$strVersion'"$TXTRESET
}

# CreateRelevantIssuesForNightTests  ============
CreateRelevantIssuesForNightTests (){
  RootDirName="NightTests"
  UsrDateTime=`whoami`_`date '+%Y.%m.%d_%H:%M:%S'`
  CurrentPath=`pwd`

  #rm -R $RootDirName  2>/dev/null
  cd ~
  CreateDir $RootDirName || exit $?
  cd $RootDirName
  CreateDir $UsrDateTime || exit $?
  
  # Create export variables for every stream
  MakeExportForStream $MCMS_DIR GetVersionFrom_VERSIONLOG "MCMS"
  MakeExportForStream $ENGINE_DIR GetVersionFrom_VERSIONLOG "EngineMRM"
  MakeExportForStream $MRMX_DIR GetVersionFrom_VERSIONLOG "MRMX"
  MakeExportForStream $MPMX_DIR GetVersionFrom_VersionH "MPMX"
  MakeExportForStream $CS_DIR GetVersionFrom_VersionH "CS"

  # Create HTML file and put there inception part
  export RootDirectory=~/$RootDirName
  export UDT_Directory=~/$RootDirName/$UsrDateTime
  export NightReportF=$UDT_Directory/NightMain.html
  PrintHTMLHeader > $NightReportF
  
  cd $CurrentPath
}

#
# Set of functions which has specific behavor per Test or stream
#
# Create_MCMS_SetSubdirectories  ================
Create_MCMS_SetSubdirectories (){
  SubDir_Audit="Audit"
  SubDir_CdrFiles="CdrFiles"
  SubDir_Main="Main"
  SubDir_Cores="Cores"
  SubDir_Faults="Faults"
  SubDir_LogFiles="LogFiles"
  
  CreateDir $SubDir_Audit || return $?
  CreateDir $SubDir_CdrFiles || return $?
  CreateDir $SubDir_Main || return $?
  CreateDir $SubDir_Cores || return $?
  CreateDir $SubDir_Faults || return $?
  CreateDir $SubDir_LogFiles || return $?

  #echo $COLORMGNTA"The version of MCMS is $COLORBROWN $MCMS_Version"$TXTRESET
  export MCMS_DirAudit="$UDT_Directory/$1/$MCMS_Version/$SubDir_Audit/"
  export MCMS_DirCdrFiles="$UDT_Directory/$1/$MCMS_Version/$SubDir_CdrFiles/"
  export MCMS_DirMain="$UDT_Directory/$1/$MCMS_Version/$SubDir_Main/"
  export MCMS_DirCores="$UDT_Directory/$1/$MCMS_Version/$SubDir_Cores/"
  export MCMS_DirFaults="$UDT_Directory/$1/$MCMS_Version/$SubDir_Faults/"
  export MCMS_DirLogFiles="$UDT_Directory/$1/$MCMS_Version/$SubDir_LogFiles/"

  #echo $COLORCYAN"MCMS_DirAudit: $COLORBROWN"$MCMS_DirAudit $TXTRESET
  #echo $COLORCYAN"MCMS_DirCdrFiles: $COLORBROWN"$MCMS_DirCdrFiles $TXTRESET
  #echo $COLORCYAN"MCMS_DirMain: $COLORBROWN"$MCMS_DirMain $TXTRESET
  #echo $COLORCYAN"MCMS_DirFaults: $COLORBROWN"$MCMS_DirFaults $TXTRESET
  #echo $COLORCYAN"MCMS_DirLogFiles: $COLORBROWN"$MCMS_DirLogFiles $TXTRESET
}

# Create_EngineMRM_SetSubdirectories  ===========
Create_EngineMRM_SetSubdirectories (){
  SubDir_Main="Main"
  #echo $COLORMGNTA"The version of EngineMRM is $COLORBROWN $EngineMRM_Version"$TXTRESET

  CreateDir $SubDir_Main || return $?
  export ENGINE_DirMain="$UDT_Directory/$1/$EngineMRM_Version/$SubDir_Main/"

  #echo $COLORCYAN"ENGINE_DirMain: $COLORBROWN"$ENGINE_DirMain $TXTRESET

  #echo "EngineMRM: Necessary to put sources inside"
}

# Create_MRMX_SetSubdirectories  ===========
Create_MRMX_SetSubdirectories (){
  SubDir_Main="Main"
  #echo $COLORMGNTA"The version of MRMX is $COLORBROWN $MRMX_Version"$TXTRESET

  CreateDir $SubDir_Main || return $?
  export MRMX_DirMain="$UDT_Directory/$1/$MRMX_Version/$SubDir_Main/"

  #echo $COLORCYAN"MRMX_DirMain: $COLORBROWN"$MRMX_DirMain $TXTRESET

  #echo "MRMX: Necessary to put sources inside"
}

# Create_MPMX_SetSubdirectories  ===========
Create_MPMX_SetSubdirectories (){
  SubDir_Main="Main"
  #echo $COLORMGNTA"The version of MPMX is $COLORBROWN $MPMX_Version"$TXTRESET

  CreateDir $SubDir_Main || return $?
  export MPMX_DirMain="$UDT_Directory/$1/$MPMX_Version/$SubDir_Main/"

  #echo $COLORCYAN"MPMX_DirMain: $COLORBROWN"$MPMX_DirMain $TXTRESET

  #echo "MPMX: Necessary to put sources inside"
}

# Create_CS_SetSubdirectories  ===========
Create_CS_SetSubdirectories (){
  SubDir_Main="Main"
  SubDir_Logs=$CS_LogDir

  CreateDir $SubDir_Logs || return $?
  CreateDir $SubDir_Main || return $?

  #echo $COLORMGNTA"The version of CS is $COLORBROWN $CS_Version"$TXTRESET
  export CS_DirDstLogs="$UDT_Directory/$1/$CS_Version/$SubDir_Logs/"
  export CS_DirMain="$UDT_Directory/$1/$CS_Version/$SubDir_Main/"

  #echo $COLORCYAN"CS_DirDstLogs: $COLORBROWN"$CS_DirDstLogs $TXTRESET
  #echo $COLORCYAN"CS_DirMain: $COLORBROWN"$CS_DirMain $TXTRESET
}

# UnsetOfAllRestedExportVars  ===================
UnsetOfAllRestedExportVars (){
  # MCMS
  unset MCMS_DirAudit
  unset MCMS_DirCdrFiles
  unset MCMS_DirMain
  unset MCMS_DirCores
  unset MCMS_DirFaults
  unset MCMS_DirLogFiles
  unset MCMS_Version

  # SoftMCU test
  unset softMcuTestResultsDir
  unset SoftMCUruns

  # EngineMRM
  unset EngineMRM_Version
  unset ENGINE_DirMain

  # MPMX
  unset MPMX_Version
  unset MPMX_DirMain

  # MRMX
  unset MRMX_Version
  unset MRMX_DirMain

  # CS
  unset CS_Version
  unset CS_LogDir
  unset CS_DirDstLogs
  unset CS_DirMain

  #unset RootDirName
  #unset UsrDateTime
  unset UDT_DirRsltFiles
  unset UDT_Directory
  unset RootDirectory
  unset NightReportF
  unset CurTestRootDir
}

# CleanAllRelevantDirs  =========================
CleanAllRelevantDirs (){
  export CurTestRootDir=""

  # MCMS
  rm -fR $MCU_HOME_DIR/mcms/LogFiles/*
  echo $COLORBROWN"$MCU_HOME_DIR/mcms/LogFiles "$COLORBLUE"has been cleaned up"$TXTRESET
  rm -Rf $MCU_HOME_DIR/mcms/Audit/*
  echo $COLORBROWN"$MCU_HOME_DIR/mcms/Audit "$COLORBLUE"has been cleaned up"$TXTRESET
  rm -Rf $MCU_HOME_DIR/mcms/Faults/*
  echo $COLORBROWN"$MCU_HOME_DIR/mcms/Faults "$COLORBLUE"has been cleaned up"$TXTRESET
  rm -Rf $MCU_HOME_DIR/mcms/CdrFiles/*
  echo $COLORBROWN"$MCU_HOME_DIR/mcms/CdrFiles "$COLORBLUE"has been cleaned up"$TXTRESET


  # Relevant to SoftMCU test
  rm -Rf $softMcuTestResultsDir/*
  echo $COLORBROWN"$softMcuTestResultsDir "$COLORBLUE"has been cleaned up"$TXTRESET
  export SoftMCUruns=No


  # EngineMRM


  # CS
  export CS_LogDir="logs"
  rm -Rf $MCU_HOME_DIR/cs/$CS_LogDir/*
  echo $COLORBROWN"$MCU_HOME_DIR/cs/$CS_LogDir "$COLORBLUE"has been cleaned up"$TXTRESET
#  rm -Rf $CS_DIR/$CS_LogDir/*
#  echo $COLORBROWN"$CS_DIR/$CS_LogDir "$COLORBLUE"has been cleaned up"$TXTRESET


  # MRMX


  # MPMX


  CleanCoreDumpsFiles
}

# MoveCoreDumpFiles  ============================
MoveCoreDumpFiles (){
  mv -f $1/* $2 2>/dev/null
  echo $COLORBROWN"CoreDump files from "$COLORBLUE"'$1'"$COLORBROWN" have been moved to "$COLORBLUE"$2"$TXTRESET
}

# MoveAnyDirToAnyDir  ===========================
MoveAnyDirToAnyDir (){
  prm1=$1/*
  mv -f $prm1 $2 2>/dev/null
  echo $COLORBROWN"Files from "$COLORBLUE"$prm1 "$COLORBROWN" have been moved to "$COLORBLUE"$2"$TXTRESET
}

# MoveAllFilesToRelativePaths  ==================
MoveAllFilesToRelativePaths (){
  CurrentPath=`pwd`
  cd ~

  MoveAnyDirToAnyDir $softMcuTestResultsDir $UDT_DirRsltFiles

  if [ $SoftMCUruns == Yes ]
  then
    #TestResults dir is taken from SoftMcuTests.sh
    TestResults=$MCU_HOME_DIR/mcms/LogFiles/softTestResults
    mv -f $TestResults/NIGHT.html $UDT_DirRsltFiles && echo $COLORBROWN"File "$COLORBLUE"$TestResults/NIGHT.html "$COLORBROWN"has been moved to "$COLORBLUE"$UDT_DirRsltFiles"$TXTRESET
  fi

  # MCMS
  MoveAnyDirToAnyDir $MCU_HOME_DIR/mcms/LogFiles $MCMS_DirLogFiles
  MoveAnyDirToAnyDir $MCU_HOME_DIR/mcms/Audit $MCMS_DirAudit
  MoveAnyDirToAnyDir $MCU_HOME_DIR/mcms/Faults $MCMS_DirFaults
  MoveAnyDirToAnyDir $MCU_HOME_DIR/mcms/CdrFiles $MCMS_DirCdrFiles
  MoveCoreDumpFiles $MCU_HOME_DIR/mcms/Cores $MCMS_DirCores

  # EngineMRM
#  mv -f $ENGINE_DIR/night.html $CurTestRootDir && echo $COLORBROWN"File "$COLORBLUE"$ENGINE_DIR/night.html "$COLORBROWN"has been moved to "$COLORBLUE"$CurTestRootDir"$TXTRESET

  # CS
#  MoveAnyDirToAnyDir $CS_DIR/$CS_LogDir $CS_DirDstLogs
  MoveAnyDirToAnyDir $MCU_HOME_DIR/cs/$CS_LogDir $CS_DirDstLogs

  # MRMX

  # MPMX

  cd $CurrentPath
}


#
# Set of functions for bulding HTML file
#
# PrintHTMLHeader  ==============================
PrintHTMLHeader (){
	echo "<html>"

	echo "<title>Night Test Report</title>"
	echo "<head><META HTTP-EQUIV=\"refresh\" CONTENT=\"5\"></head>"
	echo "<body>"

	echo "<b><font color=\"brown\">Night Test Report</font></b>" "<br><br><br><br>"
	echo "Generated at<font color=\"magenta\">" $(date) "</font><br><br>"
	echo "MCMS Version: <font color=\"blue\">"$MCMS_Version"</font><br>"
	echo "EngineMRM Version: <font color=\"blue\">"$EngineMRM_Version"</font><br>"
	echo "MRMX Version: <font color=\"blue\">"$MRMX_Version"</font><br>"
	echo "MPMX Version: <font color=\"blue\">"$MPMX_Version"</font><br>"
	echo "CS Version:   <font color=\"blue\">"$CS_Version"</font><br><br>"
	echo "Host name: <font color=\"brown\">"$(hostname)"</font><br><br>"
	echo "Analysis Info Folder: <font color=\"green\">" $UDT_Directory "</font><br><br>"
	echo "<table border=\"1\" cellpadding=\"5\" cellspacing=\"5\" width=\"100%\">"
	echo "<th>Test name</th><th>Group</th><th>Status</th><th>Valgrinrd</th>"

	export TESTS_PASSED=0
	export TESTS_FAILED=0
}

# PrintHTMLTblEntry  ============================
PrintHTMLTblEntry (){
	export ColorSuccess="Green"
	export ColorFailing="Red"

	echo "<tr>" >> $NightReportF
	#echo "<td width=\"50%\"><a href=\"$UDT_Directory/$2\">"$2"</a></td>" >> $NightReportF
	echo "<td width=\"50%\">"$2"</td>" >> $NightReportF
	echo  "<td>"$3"</td>" >> $NightReportF

	if [ $1 == 0 ]
	then
		echo "<td bgcolor=\"$ColorSuccess\"><font color=\"white\">pass -  $4 seconds</td>" >> $NightReportF
		let TESTS_PASSED=$(($TESTS_PASSED + 1))
	else
		echo "<td bgcolor=\"$ColorFailing\"><font color=\"white\">failed -  $4 seconds</td>" >> $NightReportF
		let TESTS_FAILED=$(($TESTS_FAILED + 1))
	fi
	echo "</tr>" >> $NightReportF
}

# PrintHTMLEnd  =================================
PrintHTMLEnd (){
	echo "</table>" >> $NightReportF
	echo "<br><br>" >> $NightReportF

	echo "<table border=\"1\" cellpadding=\"5\" cellspacing=\"5\" width=\"100%\">" >> $NightReportF
	echo "<th>Group name</th><th>Passed</th><th>Failed</th><th>Valgrind Passed</th><th>Valgrind Failed</th>" >> $NightReportF
	for group in `cat $NightReportF | grep "<td width=" -A 1 | grep "<td>" | cut -f2 -d">" | cut -f1 -d"<" | sort -u | sed 's/ /*/g'`
	do
		echo "<tr>" >> $NightReportF
		echo "<td>" $(basename $group | sed 's/*/ /g') "</td>" >> $NightReportF
		echo "<td align=center><font color=\"green\">" $(cat $NightReportF | grep -A 1 "`echo $group | sed 's/*/ /g'`" | grep "<td bgcolor=\"$ColorSuccess\">" | wc -l) "</font></td>" >> $NightReportF
		echo "<td align=center><font color=\"red\">" $(cat $NightReportF | grep -A 1 "`echo $group | sed 's/*/ /g'`" | grep "<td bgcolor=\"$ColorFailing\">" | wc -l) "</font></td>" >> $NightReportF
		echo "<td align=center><font color=\"green\">" 0 "</font></td>" >> $NightReportF
		echo "<td align=center><font color=\"red\">" 0 "</font></td>" >> $NightReportF
		echo "</tr>" >> $NightReportF
	done	
	echo "<tr>" >> $NightReportF
	echo "<td><b>" TOTAL: "</b></td>" >> $NightReportF
	echo "<td align=center><b><font color=\"green\">"$TESTS_PASSED"</font></b></td>" >> $NightReportF
	echo "<td align=center><b><font color=\"red\">"$TESTS_FAILED"</font></b></td>" >> $NightReportF
	echo "<td align=center><b><font color=\"green\">"0"</font></b></td>" >> $NightReportF
	echo "<td align=center><b><font color=\"red\">"0"</font></b></td>" >> $NightReportF
	echo "</tr>" >> $NightReportF

	echo "</table>" >> $NightReportF
	echo "<br><br>" >> $NightReportF

	echo "</body></html>" >> $NightReportF

	unset TESTS_PASSED
	unset TESTS_FAILED
	unset ColorSuccess
	unset ColorFailing
}


#
# List of functions which run tests themselves
#

verify_night_testing_variables ()
{
	return 0
}

#--NTD_SoftMCUSystemTests
NTD_SoftMCUSystemTests (){
  retCode=0
  SoftMCUruns=Yes

  verify_night_testing_variables || retCode=$?
  if [[ $retCode != 0 ]];then
  	$1 $retCode $2
  else
	RunSmth $1 $2 Scripts/SoftMcuTests.sh NIGHT || retCode=$?
 fi
  SoftMCUruns=No
  return $retCode
}

#--NTD_MCMSAutoRealVoip
NTD_MCMSAutoRealVoip (){
  RunSomeMCMSPython $1 $2 Scripts/RunTest.sh Scripts/AutoRealVoip.py
  return $?
}

#--NTD_MCMSAutoRealVideo
NTD_MCMSAutoRealVideo (){
  RunSomeMCMSPython $1 $2 Scripts/RunTest.sh Scripts/AutoRealVideo.py
  return $?
}

#--NTD_MCMSAdd20ConferenceNew
NTD_MCMSAdd20ConferenceNew (){
  RunSomeMCMSPython $1 $2 Scripts/RunTest.sh Scripts/Add20ConferenceNew.py
  return $?
}

#--NTD_MCMSAddDeleteIpServ
NTD_MCMSAddDeleteIpServ (){
  RunSomeMCMSPython $1 $2 Scripts/RunTest.sh Scripts/AddDeleteIpServ.py 
  return $?
}

#--NTD_MCMSAddDeleteNewIvr
NTD_MCMSAddDeleteNewIvr (){
  RunSomeMCMSPython $1 $2 Scripts/RunTest.sh Scripts/AddDeleteNewIvr.py 
  return $?
}

#--NTD_MCMSAddIpServiceWithGkNew
NTD_MCMSAddIpServiceWithGkNew (){
  RunSomeMCMSPython $1 $2 Scripts/RunTest.sh Scripts/AddIpServiceWithGkNew.py 
  return $?
}

#--NTD_MCMSAddIpServiceWithGk
NTD_MCMSAddIpServiceWithGk (){
  RunSomeMCMSPython $1 $2 Scripts/RunTest.sh Scripts/AddIpServiceWithGk.py 
  return $?
}

#--NTD_MCMSAddRemoveMrNew
NTD_MCMSAddRemoveMrNew (){
  RunSomeMCMSPython $1 $2 Scripts/RunTest.sh Scripts/AddRemoveMrNew.py 
  return $?
}

#--NTD_MCMSAddRemoveOperator
NTD_MCMSAddRemoveOperator (){
  RunSomeMCMSPython $1 $2 Scripts/RunTest.sh Scripts/AddRemoveOperator.py 
  return $?
}

#--NTD_MCMSAddRemoveProfile
NTD_MCMSAddRemoveProfile (){
  RunSomeMCMSPython $1 $2 Scripts/RunTest.sh Scripts/AddRemoveProfile.py 
  return $?
}


#--NTD_EngineMRMNightTests
NTD_EngineMRMNightTests (){
  cd $ENGINE_DIR
  RunSmth $1 $2 ./mrm.sh night
  if [ $2 == "true" ]
  then 
	echo $COLORCYAN"MRMP html report is ready:" $ENGINE_DIR/night.html $TXTRESET
  fi
  return $?
}


#--NTD_RMXPartNightTests
NTD_RMXPartNightTests (){
  Scripts/Header.sh  "run_night_test.sh"
  RunSmth $1 $2 Scripts/run_night_test.sh "short" "NoMail"
  return $?
}

