#!/bin/bash
# scriptname - CommonTestsDefs.sh


#
# List of private functions of the file
#
CleanCoreDumpsFrom (){
  ccdf_param=${1-NONE}
  if [ $ccdf_param != "NONE" ]
  then
	echo $COLORBLUE"CoreDump files from "$COLORBROWN"$ccdf_param "$COLORBLUE"have been cleaned up"$TXTRESET
	cd $1
	rm -f *core* #> /dev/null
  fi
}

CheckCoreDumpsExistingFor (){
  retCode=0
  #echo "CheckCoreDumpsExistingFor() : disabled for now!!!!!!"
  #return 0
  ccdef_param=${1-NONE}
  if [ $ccdef_param != "NONE" ]
  then
	cd $ccdef_param

	rm -f *Gideon*
	if [ $? != 0 ]
	then
		echo $COLORRED"Ignoring GideonSim Core"$COLORCYAN"$ccdef_param"$TXTRESET
	fi

	retCode=`ls *core* 2>/dev/null | wc -l`
	if [ $retCode != 0 ]
	then
		echo $COLORRED"coreDump file(s) has(ve) been found in "$COLORCYAN"$ccdef_param"$TXTRESET
	fi
  fi
  return $retCode
}




#
# List of public functions of the file
#

# It is using to start MCMS python tests
StartMCMSPythonTests (){
  cd $MCMS_DIR
  export GIDEONSIM=YES
  export ENDPOINTSSIM=YES
}

# It is called after MCMS python tests
EndMCMSPythonTests (){
  unset GIDEONSIM
  unset ENDPOINTSSIM
}

# CleanCoreDumpsFiles  ==========================
CleanCoreDumpsFiles (){
  CurrentPath=`pwd`
  echo $COLORBROWN"starting CleanCoreDumpsFiles()"$TXTRESET
  
  # Clean coreDumps of MCMS
  CleanCoreDumpsFrom $MCU_HOME_DIR/mcms/Cores

  echo $COLORBROWN"ending CleanCoreDumpsFiles()"$TXTRESET

  cd $CurrentPath
}

# CheckCoreDumpsExisting  =======================
CheckCoreDumpsExisting (){
  ColorIs=$COLORCYAN
  retMCMS=0
  exitCode=0

  CurrentPath=`pwd`
  #echo $COLORCYAN"starting CheckCoreDumpsExisting()"$TXTRESET

  CheckCoreDumpsExistingFor $MCU_HOME_DIR/mcms/Cores || retMCMS=$? 

  let "exitCode=$retMCMS"

  if [ $exitCode -ne 0 ]
  then
    ColorIs=$COLORRED
  fi

  echo $ColorIs"CheckCoreDumpsExisting() returns $exitCode"$TXTRESET
  cd $CurrentPath

  return $exitCode
}

# RunSmth  ======================================
#Notes: This function can run command with one parameter only
RunSmth (){
  RetCode=0
  RsltCoreDump=0

  param=${2-false}

  #echo RunSmth $@
 
  CurrentPath=`pwd`
  cd $MCMS_DIR

  ##$MCMS_DIR/Scripts/Destroy.sh
  $MCMS_DIR/Scripts/Cleanup.sh
  $MCMS_DIR/Scripts/CleanSharedResources.sh
  #echo "Sleeping 4 seconds...."
  #sleep 4

  cd $CurrentPath

  $3 $4 || RetCode=$?
  CheckCoreDumpsExisting || RsltCoreDump=$?

  if [ $RetCode -ne 0 -o $RsltCoreDump -ne 0 ]
  then
    if [ $RsltCoreDump -ne 0 ]
    then
	#echo $COLORRED"The test was failed, because CoreDump file has been created"$TXTRESET
	RetCode=$RsltCoreDump
    fi 
    $1 $RetCode $param
  else
    if [ $param == "true" ]
    then
	$1 0 $param
    fi
  fi

  return $RetCode
}



# RunSomeMCMSPython  ============================
RunSomeMCMSPython (){
  #echo "RunSomeMCMSPython runs with '$@'"
  param=${2-false}
  if [ $param == "true" ]
  then
    StartMCMSPythonTests
  fi

  RunSmth $@
  return $?

  #It is not relevant, because RunSmth() will do the same
  
  #if [ $param == "true" ]
  #then
  #  EndMCMSPythonTests
  #  $1 0
  #fi
}


# GetVersionFrom_VERSIONLOG  ====================
GetVersionFrom_VERSIONLOG (){
  if test -e VERSION.LOG; then
    cat VERSION.LOG | grep "Version\:" | cut -f2 -d":" | tail -1 | sed 's/[ \t]*//g'
  else
    CurrentPath=`pwd`
    #echo $COLORRED"VERSION.LOG can't be found in dir $CurrentPath"
    if test -e $OFFICIAL_DIR/BL_names.txt; then
      if [ "$1" == "MPMX" ]; then
        cat $OFFICIAL_DIR/BL_names.txt | grep ^MediaCard | cut -f2 -d"="
      else
        cat $OFFICIAL_DIR/BL_names.txt | grep ^$1 | cut -f2 -d"="
      fi
    else
      #echo $COLORRED"$OFFICIAL_DIR/BL_names.txt doesn't exist too"
      echo "$1_V1000000"
    fi
  fi
}

# GetVersionFrom_VersionH  ======================
GetVersionFrom_VersionH (){
  if test -e version.h; then
    cat version.h | cut -f3 -d" " | sed 's/[ \t"]*//g'
  else
    CurrentPath=`pwd`
    #echo $COLORRED"version.h can't be found in dir $CurrentPath for $1"
    if test -e $OFFICIAL_DIR/BL_names.txt; then
      if [ "$1" == "MPMX" ]; then
        cat $OFFICIAL_DIR/BL_names.txt | grep ^MediaCard | cut -f2 -d"="
      else
        cat $OFFICIAL_DIR/BL_names.txt | grep ^$1 | cut -f2 -d"="
      fi
    else
      #echo $COLORRED"$OFFICIAL_DIR/BL_names.txt doesn't exist too"
      echo "$1_V100000"
    fi
  fi
}








# List of functions from Yakov

verify_testing_variables () {
	if [ "$CALLGEN_IP" != "" ]
	then
	 	echo  "CALLGEN_IP is set to " $CALLGEN_IP
		#ping -q -c 1 $CALLGEN_IP
		#if [$? != "0"]
		#then
		#	echo $CALLGEN_IP "is unreachable"
		#	return 1
		#fi
	else
		echo $COLORRED"CALLGEN_IP is set to default 10.227.3.119. Please verify that Call Generator is running on this host!"$TXTRESET
		#return 1
	fi

	if [ "$EP1" != "" ]
	then
	 	echo  "EP1 is set to " $EP1
	else
		echo $COLORRED"EP1 is not defined. Tests using EP1 will fail!"$TXTRESET
		return 2
	fi

	if [ "$EP2" != "" ]
	then
	 	echo  "EP2 is set to " $EP2
	else
		echo $COLORRED"EP2 is not defined. The second end point will not be connected!"$TXTRESET
		#return 3
	fi
	export my_ip=`/sbin/ifconfig eth0 | grep inet | cut -f 2 -d ":" | cut -f 1 -d " "`
	#export my_ip=`/sbin/ifconfig $1 | awk '/inet addr/ { print $2; }' | awk -F: '{ print $2; }' | grep -v 127.0.0.1`
}

