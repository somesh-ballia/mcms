#!/bin/bash 

printf_new()
{
 	str=$1
 	num=$2
	color=$3
 	v=$(printf "%-${num}s" "$str")
 	echo -e "\e[0;${color}m"  "${v// /$str}" '\e[0m'
}

function getElement()
{
	#el_value=$GetTestStatus
    	# Extracting the value from the <$2> element
    	el_value=`grep "<$2.*<.$2>" $1 | sed -e "s/^.*<$2/<$2/" | cut -f2 -d">"| cut -f1 -d"<"`
    	echo $el_value
}

function printMsg()
{
  	#default color is white
  	colorNumber=37
        sign="-"
  	if [ $# -eq 3 ] ; then
        	sign=$3
		colorNumber=$2
        fi

  	if [ $# -eq 2 ] ;  then
    		colorNumber=$2
   	fi

   	white='\e[0;'${colorNumber}'m'
   	endColor='\e[0m'
      	if [[ !  -z "$sign" ]] ; then 
        	printf_new "$sign" 40 $colorNumber
      	fi 

   	echo -e ${white}   $1  ${endColor} 

      	if [ ! -z "$sign" ]; then 
        	printf_new "$sign" 40 $colorNumber
      	fi 
}

function getLinuxColorNumber()
{
	case "$1" in
	"lightblue") echo "36" ;; 
	"yellow") echo "33" ;;
	"white") echo "30" ;;
	"green") echo "32" ;;
	"red") echo "31" ;;
	"gray") echo "34" ;;
	"black") echo "37" ;;
        *) echo "30" ;;
	esac	
}

function CancelTest()
{
	touch $postDataFile
	chmod 666 $postDataFile
	response_file=~/responseXml.tmp
	touch $response_file
	chmod 666 $response_file

	(
		echo "<TestContract xmlns=\"RealLifeWebExpress.Commons\">"
		echo "<UserGuid>$user_guid</UserGuid>"
		echo "<TestGuid>$test_guid</TestGuid>"
		echo "</TestContract>"
	) > $postDataFile

	resp=$(curl -s --header "Content-Type: application/xml" --request POST $testsgenerator_server_url"/CancelTest" --data @$postDataFile)
	#get the response string and extract result
	echo $resp > $response_file
	result=$(getElement $response_file string) 
	echo -e ${MAGENTA}"Cancel test..."${NO_COLOR}
	rm $postDataFile
	rm $response_file
}

function Encrypt()
{
	touch $postDataFile
	chmod 666 $postDataFile
	response_file=~/responseXml.tmp
	touch $response_file
	chmod 666 $response_file

	(
		echo "<StringContract xmlns=\"RealLifeWebExpress.Commons\">"
		echo "<Value>$1</Value>"
		echo "</StringContract>"
	) > $postDataFile

	resp=$(curl -s --header "Content-Type: application/xml" --request POST $testsgenerator_server_url"/Encrypt" --data @$postDataFile)
	echo $resp > $response_file
	encryptResults=$(getElement $response_file string) 

	rm $postDataFile
	rm $response_file	
}


function ServerCheckUserAuthentication()
{
	touch $postDataFile
	chmod 666 $postDataFile
	response_file=~/responseXml.tmp
	touch $response_file
	chmod 666 $response_file

	(
		echo "<AuthenticationContract xmlns=\"RealLifeWebExpress.Commons\">"
		echo "<UserName>Polycom\\$(whoami)</UserName>"
		echo "<Password>$1</Password>"
		echo "<ADAuthentication>true</ADAuthentication>"
		echo "</AuthenticationContract>"
	) > $postDataFile
	
	resp=$(curl -s --header "Content-Type: application/xml" --request POST $testsgenerator_server_url"/CheckAuthentication" --data @$postDataFile)
	echo $resp > $response_file
	CheckPasswordResults=$(getElement $response_file boolean) 

	rm $postDataFile
	rm $response_file
}

function CheckUserAuthentication()
{
	CheckPasswordResults=false
	if [ -f $passwordFile ] ; then
		. $passwordFile
		userPassword=$EncryptedString
		ServerCheckUserAuthentication $userPassword
	fi

	if [[ $CheckPasswordResults == false ]] ;then			
		if [ -f $passwordFile ] ; then	
			rm $passwordFile
		fi
		read -s -p "Enter password for `whoami`: " userPassword
		echo ""
		Encrypt $userPassword
		userPassword=$encryptResults
		ServerCheckUserAuthentication $userPassword
	fi

	if [[ $CheckPasswordResults == true ]] ;then
		userAuthenticationResults=true
		if [ ! -f $passwordFile ] ; then	
			touch $passwordFile
			echo "EncryptedString='$encryptResults'" >> $passwordFile
		fi		
	else
		userAuthenticationResults=false
	fi
}

function ServerLogIn()
{ 
	touch $postDataFile
	chmod 666 $postDataFile
	response_file=~/responseXml.tmp
	touch $response_file
	chmod 666 $response_file

	#Send login command
	(
		echo "<AuthenticationContract xmlns=\"RealLifeWebExpress.Commons\">"
		echo "<UserName>${testsgenerator_user_name}</UserName>"
		echo "<Password>${testsgenerator_password:="Test"}</Password>"
		echo "<ADAuthentication>false</ADAuthentication>"
		echo "</AuthenticationContract>"
	) > $postDataFile
	
	resp=$(curl -s --header "Content-Type: application/xml" --request POST $testsgenerator_server_url"/Login" --data @$postDataFile)
	echo $resp > $response_file
	user_guid=$(getElement $response_file string) 

	rm $postDataFile
	rm $response_file
}

function ExecuteTest()
{
	touch $postDataFile
	chmod 666 $postDataFile
	response_file=~/responseXml.tmp
	touch $response_file
	chmod 666 $response_file
	touch $postDataFile.tmp
	chmod 666 $postDataFile.tmp

	(
		echo "<ExecuteTestContract xmlns=\"RealLifeWebExpress.Commons\">"
  		echo "<UserGuid>$user_guid</UserGuid>"		
		echo "<TestID>35</TestID>"					#35=PreDeliverySmoke
		if [[ "$smokeGhostMode" != true ]] ; then		
			echo "<TestParameters>606:${endpoint_ip:="0.0.0.0"}</TestParameters>"	
		else
			echo "<TestParameters>606:0.0.0.0</TestParameters>"			
		fi
		echo -n "<CommonParameters>"

		echo -n "522:200,"  						#522=Execution Path	200=MosheS
		
		echo -n "744:219," 						#744=Initiator Type 	219=LINUX
		if [[ "$smokeGhostMode" != true ]] ; then
			echo -n "1476:2,"					#1476=InitiatorWaitForClient 2=FALSE
			echo -n "741:$localIP,"					#741=Initiator IP
			echo -n "742:`whoami`,"    		 		#742=Initiator user
			echo -n "743:${userPassword:="NULL"},"   		#743=Initiator password
		fi
		
		if [[ "$Platform_Type" == "RMX" ]] ; then
			echo -n "4:85," 					#4=PlatformType  	85=RMX
			echo -n "10:128," 					#10=Private MCU  	128=YES			
			echo -n "8:70," 					#8=Upgrade MCU 		70=PRIVATE
			echo -n "18:${OFFICIAL_DIR:=""},"  			#18=Upgrade path

			echo -n "13:${mcu_ip},"					#13=MCU IP
			echo -n "17:${mcu_xml_api_port:="80"},"			#17=MCU port
			echo -n "15:${mcu_user_name:="SUPPORT"},"		#15=MCU user name
			echo -n "16:${mcu_user_password:="SUPPORT"},"		#16=MCU password
			echo -n "425:${mcu_ssh_user:="rt3p1aa"},"    		#425=MCU SSH UserName
			echo -n "426:${mcu_ssh_password:="1z56tcp"},"   	#426=MCU SSH Password
			echo -n "19:${ACTIVATION_KEY:=""},"			#19=MCU Activation key

			if [ ${DSP_DIR:="NULL"} != "NULL" ] ; then
				echo -n "820:94,"				#820=DSP Upgrade 	94=Private
				echo -n "821:${DSP_DIR:="NULL"},"   		#821=DSP Source path
				echo -n "822:$localIP,"				#822=DSP Source Machine IP
				echo -n "823:`whoami`,"    			#823=DSP Source Machine UserName
				echo -n "824:${userPassword:="NULL"},"  	#824=DSP Source Machine Password
			fi

			if [ ${MFA_DIR:="NULL"} != "NULL" ] ; then
				echo -n "1479:94,"				#1479=MFA Upgrade 	94=Private
				echo -n "1483:${MFA_DIR:="NULL"},"   		#1483=MFA Source path
				echo -n "1480:$localIP,"			#1480=MFA Source Machine IP
				echo -n "1481:`whoami`,"    		 	#1481=MFA Source Machine UserName
				echo -n "1482:${userPassword:="NULL"},"   	#1482=MFA Source Machine Password
			fi

			if [ ${SWITCH_DIR:="NULL"} != "NULL" ] ; then
				echo -n "1484:94,"				#1484=Switch Upgrade 	94=Private
				echo -n "1488:${SWITCH_DIR:="NULL"},"   	#1488=Switch Source path
				echo -n "1485:$localIP,"			#1485 Switch Source Machine IP
				echo -n "1486:`whoami`,"    			#1486=Switch Source Machine UserName
				echo -n "1487:${userPassword:="NULL"}," 	#1487=Switch Source Machine Password
			fi
		else
			echo -n "4:291," 					#4=PlatformType  	291=DeveloperVM
			echo -n "10:127," 					#10=Private MCU		127=NO
			echo -n "8:70,"	 					#8=Upgrade MCU  	70=PRIVATE
			echo -n "18:${OFFICIAL_DIR:=""},"  			#18=Upgrade path
			echo -n "1490:${OFFICIAL_DIR:="NULL"},"
			echo -n "986:${MCMS_DIR:="NULL"},"
			echo -n "987:${CS_DIR:="NULL"},"
			echo -n "988:${ENGINE_DIR:="NULL"},"
			echo -n "990:${MRMX_DIR:="NULL"},"
			echo -n "989:${MPMX_DIR:="NULL"},"			
			echo -n "425:`whoami`,"    		 		#425=MCU SSH UserName
			echo -n "426:${userPassword:="NULL"},"   		#426=MCU SSH Password
		fi

		if [[ "$smokeGhostMode" == true ]] ; then
			echo -n "7:PRIVATE," 						#7=Private mail address
			echo -n "9:moshe.stone@polycom.co.il,"
		else
			echo -n "7: ,"							
		fi
		if [[ "$quiet_mode" == "true"  || "$smokeGhostMode" == true ]] ; then
			echo -n "1061:3" 					#1061=Quiet Mode  3=True
		else
		        echo -n "1061:2" 					#1061=Quiet Mode  2=False
		fi
		echo "</CommonParameters>"
		echo -n "<TestScenarios>$run_scenarios</TestScenarios>"
		echo "</ExecuteTestContract>"

	) > $postDataFile.tmp
	
	xmllint --format --nowarning $postDataFile.tmp > $postDataFile
	resp=$(curl -s --header "Content-Type: application/xml" --request POST $testsgenerator_server_url"/ExecuteTest" --data @$postDataFile)
	echo $resp > $response_file
	test_guid=$(getElement $response_file string)
	
	rm $postDataFile
	rm $postDataFile.tmp
	rm $response_file
}

last_test_status=""
function GetTestStatus()
{	
	touch $postDataFile
	chmod 666 $postDataFile
	response_file=~/responseXml.tmp
	touch $response_file
	chmod 666 $response_file

	(
		echo "<TestContract xmlns=\"RealLifeWebExpress.Commons\">"
		echo "<UserGuid>$user_guid</UserGuid>"
		echo "<TestGuid>$test_guid</TestGuid>"
		echo "</TestContract>"
	) > $postDataFile
	resp=$(curl -s --header "Content-Type: application/xml" --request POST $testsgenerator_server_url"/GetTestStatus"  --data @$postDataFile)
	echo $resp > $response_file
	test_status=$(getElement $response_file string) 
	if [[ "$last_test_status" != "$test_status" ]] ; then
		echo ""
		printMsg  "Test Status: ${test_status}" 36 ""
	fi
	last_test_status=$test_status
	rm $postDataFile
	rm $response_file
}

function GetTestParameters()
{
	touch $postDataFile
	chmod 666 $postDataFile
	response_file=~/responseXml.tmp
	touch $response_file
	chmod 666 $response_file

	(
		echo "<GetExecutedTestParametersContract xmlns=\"RealLifeWebExpress.Commons\">"
		echo "<UserGuid>$user_guid</UserGuid>"
		echo "<TestGuid>$test_guid</TestGuid>"
		echo "<XmlFormat>false</XmlFormat>"
		echo "</GetExecutedTestParametersContract>"
	) > $postDataFile
	resp=$(curl -s --header "Content-Type: application/xml" --request POST $testsgenerator_server_url"/GetExecutedTestParameters"  --data @$postDataFile)
	echo $resp > $response_file
	resp=$(getElement $response_file string) 

        paramsArr=(${resp//;/ })
	paramArr=(${paramsArr[0]//:/ })
	mcu_ip=${paramArr[1]}

	paramArr=(${paramsArr[1]//:/ })
	runnerIP=${paramArr[1]}	

	rm $postDataFile
	rm $response_file
}

function DeleteLogFile()
{
	# Creating the file from which other session will communicate from (It will write to this file)
	rm -rf $progressLogFilePath
	touch  $progressLogFilePath
	#File is beign written by other linux user
	chmod 777 $progressLogFilePath 

	rm -rf $devSmokeRunLogPath
	touch  $devSmokeRunLogPath
	chmod 777 $devSmokeRunLogPath
}

NO_COLOR=$(tput sgr0)
BLACK=$(tput setaf 0)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
LIME_YELLOW=$(tput setaf 190)
YELLOW=$(tput setaf 3)
POWDER_BLUE=$(tput setaf 153)
BLUE=$(tput setaf 4)
MAGENTA=$(tput setaf 5)
CYAN=$(tput setaf 6)
WHITE=$(tput setaf 7)
BRIGHT=$(tput bold)
NORMAL=$(tput sgr0)
BLINK=$(tput blink)7
REVERSE=$(tput smso)
UNDERLINE=$(tput smul)

PreDeliverySmokeStatus=""
testsgenerator_server_url="http://10.226.106.253:45758"
#testsgenerator_server_url="http://10.253.72.16:45758"  #My local
#testsgenerator_server_url="http://10.227.1.38:45758"  #Test Server 
localIP=$(ifconfig  | grep 'inet addr:'| grep -v '127.0.0.1' | cut -d: -f2 | awk '{ print $1}')
progressLogFilePath=/tmp/progressLog
configurationFile=~/PreDeliverySmoke_Configuration.ini
passwordFile=~/PreDeliverySmoke.ini
postDataFile=~/postDataFile.xml
devSmokeRunLogPath=~/devSmokeRun.log


if  [[ $# -ge 1 ]] && [[ "$1" == "Initialize" ]]  ; then
	if [[ "$smokeGhostMode" != true ]] ; then
	echo "#######################################################################"
	echo "###          PreDelivery Smoke Test   -   START INITILIZING         ###"
	echo "#######################################################################"
	fi

	#If configuration file not found then create it and prompt the user to enter values
	if [ ! -f $configurationFile ] ; then
		if [[ "$smokeGhostMode" != true ]] ; then	
			echo -e ${MAGENTA} "CONFIGURATION FILE DOESNT EXIST !!!"${NO_COLOR}
			echo -e ${MAGENTA} "Test configuration file not found so ill create one for you..."${NO_COLOR}
			echo -e ${MAGENTA} "Creating file in $configurationFile with template data"${NO_COLOR}
			echo -e ${MAGENTA} "Please open configuration file and edit required parameters "${NO_COLOR}
		fi

		touch $configurationFile
		echo "#endpoint_ip=\"\"						#default value - 0.0.0.0 (Must be HDX)"  >> $configurationFile
		echo "#Platform_Type=\"SMCU\"					#default value - SMCU.  (RMX / SMCU)" >> $configurationFile
		echo "#mcu_ip=\"\"						#default value - for RMX:Empty, for SMCU: $localIP " >> $configurationFile
		echo "#mcu_xml_api_port=\"80\"					#default value - 80" >> $configurationFile
		echo "#mcu_user_name=\"SUPPORT\"				#default value - SUPPORT" >> $configurationFile
		echo "#mcu_user_password=\"SUPPORT\"				#default value - SUPPORT" >> $configurationFile
		echo "#mcu_ssh_user=\"rt3p1aa\"					#default value - for RMX: rt3p1aa, for SMCU: `whoami`" >> $configurationFile
		echo "#mcu_ssh_password=\"1z56tcp\"				#default value - for RMX: 1z56tcp, for SMCU: Empty" >> $configurationFile
		echo "#testsgenerator_user_name=\"`whoami`\"			#default value - `whoami`" >> $configurationFile
		echo "#testsgenerator_password=\"Test\"				#default value - Test" >> $configurationFile
		echo "#run_mcms_tests=true 					#default value - true" >> $configurationFile
		echo "#run_algorithm_tests=true 				#default value - false" >> $configurationFile
		echo "#run_embedded_tests=true  				#default value - false" >> $configurationFile
		echo "#quiet_mode=false  					#default value - true" >> $configurationFile

		export PreDeliverySmokeStatus="FAILED"
	 	return "1"
	fi

	# Read configuration file
	. $configurationFile

	if [[ "$smokeGhostMode" == true  ]] ; then
		if [[ "${Platform_Type:="SMCU"}" == "RMX" ]] ; then
			export PreDeliverySmokeStatus="FAILED"
		 	return "1"
		else
			endpoint_ip="0.0.0.0"
		fi
	fi

	if   [[ "${Platform_Type:="SMCU"}" == "RMX" && "${mcu_ip:=""}" == "" ]] ; then
		echo -e ${RED} "CONFIGURATION PARAMETERES NOT VALID !!!"${NO_COLOR}
		echo -e ${RED} "test configuration file is invalid - please fix file located at: $configurationFile "${NO_COLOR}
		echo -e ${RED} "mcu_ip parameter value is missing  "${NO_COLOR}
		echo -e ${RED} "Please open configuration file and edit invalid parameters "${NO_COLOR}
		export PreDeliverySmokeStatus="FAILED"
		return 1
	fi

	if [[ "$smokeGhostMode" != true ]] ; then	
		printMsg "Your current configuration file:" 36 "" 
		cat $configurationFile
		printMsg  "Above displayed your configuration file. Are you sure you want to continue ? press n to edit" 36 ""
		read -t 15 doEdit
		if [[ "$doEdit" == "n" || "$doEdit" == "N" ]] ; then
			printMsg "Configuration file is located at: $configurationFile" 36 ""
			export PreDeliverySmokeStatus="FAILED"
		 	return "1"
		fi
	fi

	CheckUserAuthentication
	if [[ $userAuthenticationResults != true ]] ; then
		if [[ "$smokeGhostMode" != true ]] ; then	
		 	echo -e ${RED} "Cant run PreDelivery Smoke test. - Authentication failed  !!!"${NO_COLOR}
		fi
		export PreDeliverySmokeStatus="FAILED"
		return 1
	fi

	echo "#######################################################################"
	echo "###          PreDelivery Smoke Test   -   END INITILIZING           ###"
	echo "#######################################################################"

	export PreDeliverySmokeStatus="OK"
	return 0
fi

if  [[ $# -ge 1 ]] && [[ "$1" == "RunTest" ]]  ; then
	echo "#######################################################################"
	echo "###          PreDelivery Smoke Test   -   START RUNNING             ###"
	echo "#######################################################################"

	# set default values
	${quiet_mode:="true"}
	testsgenerator_user_name=${testsgenerator_user_name:=$(whoami)}

	# Build test scenarios string
	${run_mcms_tests:="true"}
	${run_algorithm_tests:="false"}
	${run_embedded_tests:="false"}	
	
	if [[ "$run_mcms_tests" == *"true"* ]]; then 
		run_scenarios="225:true,538:false,1582:false"
	fi
	
	if [[ "$run_embedded_tests" == *"true"* ]]; then 
		run_scenarios="225:false,538:true,1582:false"
	fi
	
	if [[ "$run_algorithm_tests" == *"true"* ]]; then 
		run_scenarios="225:false,538:false,1582:true"
	fi
	
	printMsg "Starting test preparation...Please wait (less then a minute)" 36 ""

	#Log in to Automation.Server
	ServerLogIn

	if [[ "$user_guid" == "" ]] ; then
		if [[ "$smokeGhostMode" != true ]] ; then	
			echo -e ${RED} "Fail: Automation.Server is down"${NO_COLOR}
			echo -e ${RED} "Please contact Moshe Stone or Yonatan Resnick"${NO_COLOR}
		fi
		export PreDeliverySmokeStatus="FAILED"
		return 1
	elif [[ "$user_guid" == "Wrong password" ]] ; then
		if [[ "$smokeGhostMode" != true ]] ; then	
			echo -e ${RED} "Fail: Wrong Automation.Server password"${NO_COLOR}
			echo -e ${RED} "Please contact Moshe Stone or Yonatan Resnick"${NO_COLOR}
		fi
		export PreDeliverySmokeStatus="FAILED"
		return 1
	fi

	if [[ "$smokeGhostMode" != true ]] ; then	
		echo "User ${testsgenerator_user_name} LogedIn to server."
	fi

	ExecuteTest

	if [ "$test_guid" == "" ] ; then
		echo -e ${RED} "Fail: Test didnt start to run on server"${NO_COLOR}
		echo -e ${RED} "Please contact Moshe Stone or Yonatan Resnick"${NO_COLOR}
		export PreDeliverySmokeStatus="FAILED"
		return 1
	fi

	if [[ "$smokeGhostMode" != true ]] ; then
		echo "Test Start to run on server"
	fi
	
	if [[ "$quiet_mode" == "false" ]] ; then
		printMsg "Note: In this test after a few minutes you'll be requested to connect with your RPD client and check video and audio quality." 36 ""
		printMsg "If you will ignore this 5 minutes phase test will fail !" 36 ""
	fi

	# wait untill test process is started (status is bigger than InProgress) - timeout 10 minutes
	testIsReady=false
	timeout=120
	i=0
	while (( $i < $timeout ))
	do
		GetTestStatus
		GetTestParameters	

		if [[ "$runnerIP" != "" && "$mcu_ip" != "" ]] ; then
			break;
		fi

		if [[ ("$test_status" == "Error")  || ("$test_status" == "ResourceDosntExists") || ("$test_status" == "Canceld") || ("$test_status" == "Canceling") ]] ; then
			break;
		fi

		if [[ ("$test_status" == "Scheduled") ]] ; then
			echo -e ${MAGENTA} "Test gets into Scheduled mode."${NO_COLOR}	
			echo -e ${MAGENTA} "Probably there is no available resources. Please try again later."${NO_COLOR}	
	                CancelTest	
			export PreDeliverySmokeStatus="FAILED"
			return 1
		fi

		sleep 5
		echo -n "."
		(( i++ ))
	done

	if [[  "$runnerIP" == "" || "$mcu_ip" == "" ]] ; then	
		echo -e ${RE} "Test didnt gets into running mode."${NO_COLOR}	
		echo -e ${RED} "Probably there is no available resources or something is wrong with the TestGenerator server. Please check test status manually."${NO_COLOR}	
		export PreDeliverySmokeStatus="FAILED"
		return 1
	else
		echo ">>> MCU IP is:" $mcu_ip	
	fi

	if [[ "$smokeGhostMode" == true ]] ; then	
		export PreDeliverySmokeStatus="OK"
		return 0
	fi

	showUpgradeMsg=false
	timeout=120
	
	if [[ "${Platform_Type:="SMCU"}" == "RMX" ]] ; then
		timeout=480
		showUpgradeMsg=true
	fi

	i=0	

	while (( $i < $timeout ))
	do
	{
		GetTestStatus
		if [[ ("$test_status" == "Error")  || ("$test_status" == "ResourceDosntExists") || ("$test_status" == "Canceld") || ("$test_status" == "Canceling") || ("$test_status" == "Crashed") ]] ; then
			export PreDeliverySmokeStatus="FAILED"
			return 1
		fi

		if [[  "$test_status" == "Running" ]] ; then
			break;
		fi
		
		if [[ "$test_status" == "Upgrade" && $showUpgradeMsg == true ]] ; then
			showUpgradeMsg=false
			let "timeoutMinutes = $timeout/12"
			echo "Waiting for mcu version upgrage to complete. Timeout for upgrade: "$timeoutMinutes" minutes"
		fi

        	(( i++ ))
		sleep 5
		echo -n "."

	}
	done

	if (( $i ==  $timeout )) ; then
		echo -e ${RED} "Test didnt gets into running mode."${NO_COLOR}	
		echo -e ${RED} "Probably can't login to mcu or can't create conference on it. Please check mcu mannualy"${NO_COLOR}	
                CancelTest
		export PreDeliverySmokeStatus="FAILED"
		return 1
	fi       

	DeleteLogFile

	last_display_line_index=0;
	test_result=0
	while [ true ]
	do
		sleep 2
		GetTestStatus
		if [[ "$test_status" != "Running" && "$test_status" != "Completed" ]] ; then
			export PreDeliverySmokeStatus="FAILED"
			return 1
	 	fi
	
		cur_line_index=0
	
		while read newLine
		do
			(( cur_line_index++ ))
	   		if [[ $cur_line_index -gt $last_display_line_index ]];  then    
		     		# GET THE COLOR AND THE MESSAGE 
				IFS=^; read msg msg_color <<< "$newLine"
				IFS=' ' 
				newLine=$msg
				color=$(getLinuxColorNumber "$msg_color")
				msg_color=$color
	
	     			if [[ "$newLine" == *"## Test Failed! ##"* ]] ;then
					test_result=1
	     			fi	
	
	     			# write it all to a log file for later watch
				echo $newLine >> $devSmokeRunLogPath
	
				if [[ "$newLine" == *"input:"*   ]] ; then
					if [[ "$newLine" == *"Is Client Ready"*   ]] ; then					
						(echo "start_block "  "ClientIsReady" " end_block"  |  nc -w 5 $localIP 5559) #5559 is the port we are using to communicate back to test client
					else
						if [[ "$quiet_mode" == *"false"* ]] ; then		
	        					echo -en '\007' #beep sound
	        					inputVal=""
		        				printMsg "${newLine:6}" $msg_color "*" #36
							while [[ $inputVal != "no_answer" && $inputVal != "y" && $inputVal != "Y" && $inputVal != "n" && $inputVal != "N" ]]
							do
		   						read -t 150 inputVal </dev/tty
								if [[ -z "$inputVal" ]]; then
									inputVal="no_answer"
								elif [[ $inputVal != "y" && $inputVal != "Y" && $inputVal != "n" && $inputVal != "N" ]] ; then
									printMsg " Syntax error '$inputVal'" 35 ""
								fi	
							done				
							(echo "start_block "  $inputVal " end_block"  |  nc -w 5 $localIP 5559) #5559 is the port we are using to communicate back to test client
						else			
							(echo "start_block "  "y" " end_block"  |  nc -w 5 $localIP 5559) #5559 is the port we are using to communicate back to test client
		                		fi
					fi
				elif  [[ "$newLine" == *"Action:"* ]] ; then	 
	        			echo -en '\007' #beep sound
		 			printMsg "$newLine" $msg_color "*"  #32
		        	else
		        		printMsg "$newLine" $msg_color ""  #33        
	        		fi
				last_display_line_index=$cur_line_index 
	      			if [[ "$newLine" == *"Finish SMOKEForDeliveryProcess Test"* ]];	then
					export PreDeliverySmokeStatus="OK"
		      			return $test_result
	      			fi
		fi
	 	done < $progressLogFilePath
	done
	echo "#######################################################################"
	echo "###          PreDelivery Smoke Test   -   END RUNNING               ###"
	echo "#######################################################################"
fi
