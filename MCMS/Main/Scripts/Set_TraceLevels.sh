#!/bin/sh







help_me()
{
  echo "Please use one of the trace levels values"
 for i in 0 1 2 3 4 5 6 7 

    do
	echo ${tracelevels[$i]};
  
    done
  exit ;

}

verifiy_level()
{ 
   for i in 0 1 2 3 4 5 6 7 
    do
	
	if [ "$1" == "${tracelevels[$i]}" ]
	then 
	   echo "trace level valid "  ${tracelevels[$i]} ;
	   return $i;
	fi
      
    done
    return 0;
}

set_new_level_all_process()
{
	#for  i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38
	#do
	#	 echo "set trace level for " ${process[$i]} $1;
	 #	$MCU_HOME_DIR/mcms/Bin/McuCmd f_level ${process[$i]} $1
	#done
	echo "set trace level for logger ${itracelevels[$1]}";
	$MCU_HOME_DIR/mcms/Bin/McuCmd set_level Logger  ${itracelevels[$1]};

	return 1;
}

verify_process()
{
	for  i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38
	do
		if [ "$1" == "${process[$i]}" ]
		then 
	   	   echo "process valid "  ${tracelevels[$i]} ;
		   return $i;
		fi	
	done
 return -1;
}

print_process()
{
	for  i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38
	do
  	     echo  ${process[$i]};

	done
}

set_process_level()
{
    echo "set trace level for " $2 $1;
    $MCU_HOME_DIR/mcms/Bin/McuCmd f_level $2 $1
}

set_local_tracer_state_all_process()
{
	for  i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38
	do
		 echo "set trace level for " ${process[$i]} $1;
	 	$MCU_HOME_DIR/mcms/Bin/McuCmd set_local_tracer ${process[$i]} $1
	done	
	return 1;
}


verify_localTracerState()
{
	for  i in 0 1
	do
		if [ "$1" == "${localTracerState[$i]}" ]
		then 
	   	   echo "local tracer state valid "  ${localTracerState[$i]} ;
		   return $i;
		fi	
	done
	return -1;
}

############# GLOBAL VAR ###################
#        eLevelOff          = 0,
#        eLevelFatal        = 1,  //System crash, conference crash etc'. NULL pointer for example. (p0 jira issue)
#        eLevelError        = 10, //Error which is not fatal but still a bug (p1,p2,p3,… jira issues)
#        eLevelWarn         = 20, //Not a bug but has to be taken into consideration. High CPU usage for example.
#        eLevelInfoHigh     = 30, //MPLAPI, Inter process and other high importance messages.
#        eLevelInfoNormal   = 50, //Minimal debugging information
#        eLevelDebug        = 70, //Debug messages (additional information on field)
#        eLevelTrace        = 100 //Trace messages (mostly for lab)

itracelevels[0]="100"
itracelevels[1]="70"
itracelevels[2]="50"
itracelevels[3]="30"
itracelevels[4]="20"
itracelevels[5]="10"
itracelevels[6]="1"
itracelevels[7]="0"

tracelevels[0]="TRACE"
tracelevels[1]="DEBUG"
tracelevels[2]="INFO_NORMAL"
tracelevels[3]="INFO_HIGH"
tracelevels[4]="WARN"
tracelevels[5]="ERROR"
tracelevels[6]="FATAL"
tracelevels[7]="OFF"

process[0]="McmsDaemon"
process[1]="Configurator"
process[2]="Logger"
process[3]="Auditor"
process[4]="Faults"
process[5]="IPMCInterface"
process[6]="SNMPProcess"
process[7]="CSMngr"
process[8]="CertMngr"
process[9]="McuMngr"
process[10]="ConfParty"
process[11]="Cards"
process[12]="CDR"
process[13]="Resource"
process[14]="SipProxy"
process[15]="DNSAgent"
process[16]="Gatekeeper"
process[17]="QAAPI"
process[18]="ExchangeModule"
process[19]="Authentication"
process[20]="Installer"
process[21]="RtmIsdnMngr"
process[22]="BackupRestore"
process[23]="MplApi"
process[24]="CSApi"
process[25]="Collector"
process[26]="SystemMonitoring"
process[27]="MediaMngr"
process[28]="Failover"
process[29]="LdapModule"
process[30]="Utility"
process[31]="MCCFMngr"
process[32]="ApacheModule"
process[33]="NotificationMngr"
process[34]="Diagnostics"
process[35]="GideonSim"
process[36]="EndpointsSim"
process[37]="EncryptionKeyServer"
process[38]="Ice"

localTracerState[0]="on"
localTracerState[1]="off"
###########  Menu Functions ##############
helperReadTraceLevel()
{

    echo "Please enter trace";
    read tracelevel;
    verifiy_level $tracelevel;	
    result=$?;
    if [ $result -eq 0 ] 
    then
      echo "Invaid parameter ="  $tracelevel ;
       help_me;
       exit 1;
    fi
    echo "Valid filter " :$tracelevel ;
    return $result;	
}

helperReadProcessName()
{
    echo "Please enter Process Name:";
    
    read processname;
    verify_process $processname;	
    result=$?;
    if [ $result -eq -1 ] 
    then
        echo "Invaid process name ="  $processname;
     	echo "select process name from list";
     	print_process
     	exit 1;
    fi
    return $result;
}
helperReadLocalTracerState()
{
  echo "Please enter local tracer state (on/off)";
  read  state;
  verify_localTracerState $state;
  result=$?;
  if [ $result -eq -1 ] 
    then
      echo "Invaid state ="  $result ;       
       exit 1;
    fi
  return $result;
}

SetTraceForAllProcess()
{
    helperReadTraceLevel;
    result=$?;
    set_new_level_all_process $result;

    echo "to return to Menu hit enter";
    read enterKey;    
}

SetTraceForSpecificProcess()
{
    helperReadProcessName;
    processIndex=$?;
    helperReadTraceLevel;
    tracelevel=$?;
    set_process_level $tracelevel ${process[$processIndex]};
    echo "to return to Menu hit enter";
    read enterKey;
}

SetLocalTracerStateForAllProcess()
{
  helperReadLocalTracerState;
  stateIndex=$?;
  set_local_tracer_state_all_process ${localTracerState[$stateIndex]};
  echo " ";
  echo " *************** DONE ********************";
  echo "to return to Menu hit enter";
  read enterKey;
}

SetLocalTracerStateForSpecificProcess()
{
  helperReadProcessName;
  processIndex=$?;
  helperReadLocalTracerState;
  stateIndex=$?;
  
  $MCU_HOME_DIR/mcms/Bin/McuCmd set_local_tracer ${process[$i]} ${localTracerState[$stateIndex]};

  echo "to return to Menu hit enter";
  read enterKey;
}

############# MAIN ###################

#check number arguments which was pass to script
#if [ $# -eq 0 ] 

#then

#  echo "no arguments were passed ";
#  help_me;
#  exit 1;

#fi

#check trace level is valid
#verifiy_level  $1;
#result=$?;
#if [ $result -eq 0 ] 
#then
#     echo "Invaid parameter ="  $1 ;
#     help_me;
#     exit 1;
#fi

#echo "Valid filter " :$1 ;
export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin
export SASL_PATH=$MCU_HOME_DIR/mcms/Bin
export Kernel=`uname -r`



while :
do
clear
echo " M A I N - M E N U"
echo "1. Set trace level for all process"
echo "2. Set trace level for specific process"
echo "3. Enable/Disable local tracer for all process"
echo "4. Enable/Disable local tracer for specific process"
echo "5. exit"
echo -n "Please enter option [1 - 5]"
read opt
case $opt in
	1) SetTraceForAllProcess;;
	2) SetTraceForSpecificProcess;;
	   
	3) SetLocalTracerStateForAllProcess;;
	4) SetLocalTracerStateForSpecificProcess;;

        5) echo "Bye $USER";
	       exit 1;;
        *) echo "$opt is an invaild option. Please select option between 1-4 only";
	   echo "Press [enter] key to continue. . .";
           read enterKey;;
esac
done
