#!/bin/bash

export DEFAULT=""
export RED="*"
export GREEN=""
export BLUE=""
export BLACK=""
export BOLD=""
export UNDERLINE=""


case "$1" in
pre)
        export VALIDATOR_PRE=1
        export VALIDATOR_POST=0
        ;;
post)
        export VALIDATOR_POST=1
        export VALIDATOR_PRE=0
        ;;
*)
        echo "$1: Action not supported."
	exit 1
        ;;
esac



MCU_HOME_DIR=
MCU_HOME_DIR=$(grep MCU_HOME_DIR /home/mcms/.bashrc | cut -d'=' -f2 | awk 'NR==1')
export MCU_LIBS=$MCU_HOME_DIR/lib:$MCU_HOME_DIR/lib64:$MCU_HOME_DIR/usr/lib:$MCU_HOME_DIR/usr/lib64

# The RPMs list was built using the following command, to get the name in specific format 'name(version)architecture'. There is no need for the release tag.
#Charachters '(' and ')' are used as delimiters for seperating between the name, version and architecture. 
# rpm -qa --queryformat "%{NAME}(%{VERSION})%{ARCH} \\n" | grep rpm_name | sort

RHEL_5_8_REQUIRES_RPMS=( 'glibc(2.5)i686' 'glibc(2.5)x86_64' 'glibc-common(2.5)x86_64' 'zlib(1.2.3)i386' 'libgcc(4.1.2)i386' 'libgcc(4.1.2)x86_64' 'libstdc++(4.1.2)i386' 'libstdc++(4.1.2)x86_64' 'libxml2(2.6.26)i386' 'libxml2(2.6.26)x86_64' 'expat(1.95.8)x86_64' 'freetype(2.2.1)x86_64' )

RHEL_6_2_REQUIRES_RPMS=( 'glibc(2.12)i686' 'glibc(2.12)x86_64' 'glibc-common(2.12)x86_64' 'zlib(1.2.3)i686' 'libgcc(4.4.6)i686' 'libgcc(4.4.6)x86_64' 'libstdc++(4.4.6)i686' 'libstdc++(4.4.6)x86_64' 'libxml2(2.7.6)i686' 'libxml2(2.7.6)x86_64' 'expat(1.95.8)x86_64' 'freetype(2.3.11)x86_64' )

SUSE_11_REQUIRES_RPMS=( 'glibc(2.11.3)x86_64' 'glibc-32bit(2.11.3)x86_64' 'zlib-32bit(1.2.3)x86_64' 'zlib(1.2.3)x86_64' 'libgcc46(4.6)x86_64' 'libgcc46-32bit(4.6)x86_64' 'libstdc++46-32bit(4.6)x86_64' 'libstdc++46(4.6)x86_64' 'libxml2(2.7.6)x86_64' 'libxml2-32bit(2.7.6)x86_64' 'aaa_base(11)x86_64' 'expat(2.0.1)x86_64' 'freetype2(2.3.7)x86_64' )

PLCM_RPMS=(ibm-sametime-vmcu-9.0.0.0-AmpSoft*.*el5/* ibm-sametime-vmcu-9.0.0.0-UI_AIM*.*el5/* ibm-sametime-vmcu-9.0.0.0-Mpmx*.*el5/* ibm-sametime-vmcu-9.0.0.0-Rmx1000*.*el5/* ibm-sametime-vmcu-9.0.0.0-VmpSoft*.*el5/* ibm-sametime-vmcu-9.0.0.0-jsoncpp*.*el5/* ibm-sametime-vmcu-9.0.0.0-SoftMcuMainMFW*.*el5/* ibm-sametime-vmcu-9.0.0.0-Ema*.*el5/* ibm-sametime-vmcu-9.0.0.0-Cs*.*el5/* ibm-sametime-vmcu-9.0.0.0-Mcms*.*el5/* ibm-sametime-vmcu-9.0.0.0-MpProxy*.*el5/* ibm-sametime-vmcu-9.0.0.0-EngineMRM*.*el5/* ibm-sametime-vmcu-9.0.0.0-SingleApache*.*el5/* )

INSTALLED_RPMS_LIST=$( rpm -qa )


RHEL="*Red Hat*release*"
SUSE="*SUSE Linux*"


function print_ok()
{
	echo -en "${GREEN}[ OK ]${DEFAULT}"
}

function print_fail()
{
	echo -e "${RED}[ FAIL ]${DEFAULT}"
}

function validate_ver()
{
	NAME=`echo "${RPM}" | cut -d"(" -f1`
  	ARC=`echo "${RPM}" | cut -d")" -f2`
  	installed_rpm=$(rpm -qa --queryformat "%{NAME}(%{VERSION})%{ARCH} \\n" | grep "^$NAME(.*)$ARC" | sort | head -n1)

  	RPM_VER=`echo "${RPM}" | cut -d"(" -f2 | cut -d ")" -f1`
  	INST_VER=`echo "${installed_rpm}" | cut -d"(" -f2 | cut -d ")" -f1`  

  	if [[ "${RPM_VER}" == "${INST_VER}" ]]; then
		return 0 
  	else
		if [[ $(uname -r | grep "2.6.18") != "" ]];then
			#CentOS 5.8 does not support sort -V
		  	if [[ "${RPM_VER}" == "`echo -e "${RPM_VER}\n${INST_VER}" | sort | head -n1`" ]]; then
       				return 0;
			else
				return 1;
			fi
		else
			if [[ "${RPM_VER}" == "`echo -e "${RPM_VER}\n${INST_VER}" | sort -V | head -n1`" ]]; then
                		return 0;
        		else    
                		return 1;
		fi
	fi
  fi
}

function test_rpms()
{
  local RPMS_LOOKUP_LIST="$@"
  RESULT_STRING=""
  counter=0
  rpms=0  
  for RPM in ${RPMS_LOOKUP_LIST[@]}; do
     validate_ver "${RPM}"
     ret=$?
     if [[ $ret == 0 ]] ; then
        let rpms=$rpms+1
     else
        RESULT_STRING=$RESULT_STRING"$RPM - is incorrect\n"
     fi      
     let counter=$counter+1
  done
  echo $RESULT_STRING
}

function test_plcm_rpms()
{
	local RPMS_LOOKUP_LIST="$@"
        RESULT_STRING=""
        counter=0
	rpms=0  
	MCU_HOME_DIR=$(grep MCU_HOME_DIR /home/mcms/.bashrc | cut -d'=' -f2 | awk 'NR==1')
	for RPM in ${RPMS_LOOKUP_LIST[@]}; do
		RPM_NAME=`echo "$INSTALLED_RPMS_LIST" | grep $RPM`
		result=$?
		if [[ $result == 0 ]]; then
			INST_DIR=`rpm -q --queryformat '%{INSTPREFIXES}\n' $RPM_NAME`
			if [[ "$INST_DIR" != "$MCU_HOME_DIR" ]] ; then
				RESULT_STRING=$RESULT_STRING"$RPM - was installed with $INST_DIR [MCU home directory is $MCU_HOME_DIR]\n"
			else
				let rpms=$rpms+1
			fi
		else
			RESULT_STRING=$RESULT_STRING"$RPM - is missing\n"
		fi
		let counter=$counter+1
	done

	#if [[ $rpms == $counter ]]; then
	#	return 0
	#else
	#	return 1
	#fi
	echo $RESULT_STRING
}

function test_ldd()
{
	export LD_LIBRARY_PATH="$1"
	export BIN_PATH="$2"
	if [ -d $3 ] ;then
		cd $3 &> /dev/null
		
		RESULT_STRING=""

		for FILE in $( ls * ); do
			if [[ $( ldd ${BIN_PATH}/${FILE} 2> /dev/null | grep "not found" ) ]]; then
				RESULT_STRING="${RESULT_STRING}${RED}${FILE}:${DEFAULT}\n$(ldd ${BIN_PATH}/${FILE})\n"
			fi
		done
	
		cd - &> /dev/null
	else
		RESULT_STRING="Directory $3 not found"
	fi
	echo $RESULT_STRING
}

###################################
# Collect general machine HW data #
###################################

echo
echo -e "${BOLD}${UNDERLINE}General machine info:${DEFAULT}"

OS_VERSION=""
OS_RELEASE=""
result=""
status=0

# Check OS Version and Kernel
#if [ -f /etc/redhat-release ]; then
#	echo -en "RHEL installed is:\t\t"
#	OS_RELEASE=$(cat /etc/redhat-release)
#	echo ${OS_RELEASE}	
#	OS_VERSION=$(echo "$(cat /etc/issue)" | sed 's/[[:alpha:]|()=|[:space:]]//g' | awk "NR==1{print;exit}")	
#	sestatus
#elif [ -f /etc/SuSE-release ]; then
#	echo -e "SUSE installed is:\t\t"
#	OS_RELEASE=$(cat /etc/SuSE-release)
#	echo ${OS_RELEASE}	
#	OS_VERSION=$(echo "$(cat /etc/*-release | grep "VERSION = ")" | sed 's/[[:alpha:]|()=|[:space:]]//g' | awk "NR==1{print;exit}")	
#	cat /etc/SuSE-brand
#	sp=$(cat /etc/SuSE-release | grep PATCHLEVEL | awk '{print $3}')
#else
#	echo "${RED}OS Not Recognized!${DEFAULT}"
#fi
Release_RPM=`rpm -qa | grep 'release-' | grep -v notes | grep -v DVD | grep -v ^lsb | grep -v epel`
if   [[ `echo -n $Release_RPM | grep ^redhat` ]]; then # RedHat release
        OS_RELEASE='Red Hat release'
        if [[ `echo -n $Release_RPM | grep '6Server'` ]]; then
                OS_VERSION=`echo -n $Release_RPM | cut -d'-' -f5 | cut -d'.' -f1,2`
        else
                OS_VERSION=`echo -n $Release_RPM | cut -d'-' -f4 | cut -d'.' -f1,2`
        fi
        sestatus
elif [[ `echo -n $Release_RPM | grep ^sles` ]]; then # SuSE release
        OS_RELEASE='SUSE Linux'
        OS_VERSION=`echo -n $Release_RPM | cut -d'-' -f3`
        sp=`echo -n $OS_VERSION | cut -d'.' -f2`
	#OS_VERSION=`echo -n $OS_VERSION | cut -d'.' -f1`
else # Unknown OS
        echo "This is an unsupported OS"
fi
if [[ "X$OS_VERSION" == "X" ]]; then
        if   [[ -f /etc/redhat-release ]]; then # RedHat release
                OS_RELEASE='Red Hat release'
                OS_VERSION=`sed -e 's/^.* \([0-9]\.[0-9]*\) .*/\1/g' /etc/redhat-release`
                sestatus
        elif [[ -f /etc/SuSE-release ]]; then # SuSE release
                OS_RELEASE='SUSE Linux'
                OS_VERSION=`grep VERSION /etc/SuSE-release | cut -d'=' -f2 | tr -d ' '`
                sp=`grep PATCHLEVEL /etc/SuSE-release | cut -d'=' -f2 | tr -d ' '`
        else
                echo "This is an unsupported OS"
        fi
fi

KERNEL_VERSION=$(uname -r)
echo -en "Kernel installed is:\t\t"	
echo  ${KERNEL_VERSION}

echo -ne "Processor:\t\t\t${DEFAULT}"
cat /proc/cpuinfo | grep "model name" | head -1 | cut -d':' -f2

# Check Number of Cores
echo -ne "Cores:\t\t\t\t" 
cores=$(cat /proc/cpuinfo | grep processor | wc -l)
echo -ne ${cores}
 

# Check for RAM
echo -ne "\n\n${BOLD}${UNDERLINE}RAM status:${DEFAULT}\n"
free 

echo -e "\n${BOLD}${UNDERLINE}Permissions:${DEFAULT}"
echo -ne "root umask:\t\t\t\t" 
umask
echo -ne "mcms umask:\t\t\t\t" 
if [[ $(cat /etc/passwd | grep ^mcms) ]]; then
        echo -ne "mcms umask:\t\t\t\t"
	su - mcms -c "umask"
fi

echo -e "\n\n${BOLD}${UNDERLINE}Disk data:${DEFAULT}"
eval "fdisk -l"

echo -e "\n${BOLD}${UNDERLINE}Mount data:${DEFAULT}"
mount

echo -e "\n\n${BOLD}${UNDERLINE}Network data:${DEFAULT}"
ifconfig


echo -e "\n\n${BOLD}${UNDERLINE}Route data:${DEFAULT}"
route
echo -e "\n\n\n"

echo -e "${BOLD}${UNDERLINE}Firewall data:${DEFAULT}"
iptables -L
echo -e "\n\n\n"

echo -e "${BOLD}${UNDERLINE}Netstat data:${DEFAULT}"
netstat -ntulp
echo -e "\n\n\n"

echo -e "${BOLD}${UNDERLINE}Installed RPMs data:${DEFAULT}"
rpm -qa
echo -e "\n\n\n"

echo
echo -e "${BOLD}${UNDERLINE}Installation Validity checks:${DEFAULT}"
echo


################################
# 	Validity Check         #
################################

status=0
#################################
# Check for OS and Kernel version
#################################
OS_VERSION_MAJOR=$(echo $OS_VERSION | cut -d'.' -f1)
OS_VERSION_MINOR=$(echo $OS_VERSION | cut -d'.' -f2)
#echo "OS_VERSION=$OS_VERSION, $OS_VERSION_MAJOR, $OS_VERSION_MINOR"

# Supported RHEL version: 5.8 and above
Lowest_OS_REL_Value=`expr $OS_VERSION_MAJOR \* 10 + 7`
OS_REL_Value=`expr $OS_VERSION_MAJOR \* 10 + $OS_VERSION_MINOR`
#if [[ $OS_RELEASE = $RHEL ]] && [[ $OS_VERSION_MAJOR = "5" && $OS_VERSION_MINOR > "7" ]]; then
if [[ $OS_RELEASE = $RHEL ]] && [[ $OS_REL_Value > $Lowest_OS_REL_Value ]] ; then
        result=0
# Supported RHEL version: 6.X
elif [[ $OS_RELEASE = $RHEL ]] && [[ $OS_VERSION_MAJOR = "6" ]]; then
        result=0
# Supported SUSE version: 11 and above
elif [[ $OS_RELEASE = $SUSE ]] && [[ $OS_VERSION_MAJOR = "11" || $OS_VERSION_MAJOR = "12" ]]; then
        result=0
else
        result=1
fi

if [[ $result == 0 ]]; then
        echo -e "OS and Kernel Version\t\t\t" $(print_ok)
else    
        echo -e "OS and Kernel Version\t\t\t" $(print_fail)     
        echo -e "${BOLD}${RED}Error: OS or Kernel not supported!${DEFAULT}\n"   
    status=1
fi

######################################
# Check for service pack (SuSE Only) #
######################################
if [[ $sp != "" ]]; then
	if [[ $sp -ge 1 && $sp -le 3 ]]; then
		echo -e "Service Pack:\t\t\t\t" $(print_ok)
	else	
		echo -e "Service Pack:\t\t\t\t" $(print_fail)	
		echo -e "${BOLD}${RED}Error: Service Pack does not supported!${DEFAULT}\n"	
		status=1
	fi
fi

################################
# Check for number of cores    #
################################
if [[ $cores -ge 4 ]]; then
	echo -e "Number of Cores:\t\t\t" $(print_ok)
else
	echo -e "Number of Cores:\t\t\t" $(print_fail)
	echo -e "${BOLD}${RED}Error: Number of cores should be at least 4${DEFAULT}\n"
	status=1	
fi  

################################
# Check for RAM                #
################################
totalMem=$(grep MemTotal /proc/meminfo | awk '{print $2}')
if [[ $totalMem -lt 8000000 ]]; then
	echo -e "RAM:\t\t\t\t\t" $(print_fail)
	echo -e "${BOLD}${RED}Error: System RAM should be at least 8 GB${DEFAULT}\n"	
	status=1
else
	echo -e "RAM:\t\t\t\t\t" $(print_ok)
fi

################################
# Check for eth0 interface     #
################################
#interface=$(ifconfig | grep eth0 | awk '{print $1}')
interface=$(cat /proc/net/dev | grep -v ^Inter- | grep -v "^ face" | grep -v lo)
if [[ $interface == "" ]]; then
	echo -e "Interface (eth0):\t\t\t" $(print_fail)
	echo -e "\n${BOLD}${RED}Error: An interface named eth0 should be in the system${DEFAULT}\n"	
	status=1	
else
	echo -e "Network Interface:\t\t\t" $(print_ok)
fi

################################
# Check for pre required rpms  #
################################
if [[ $(uname -r | grep "2.6.18") != "" ]];then
	result=$(test_rpms "${RHEL_5_8_REQUIRES_RPMS[@]}")
elif [[ $(uname -r | grep "2.6.32") != "" ]];then
result=$(test_rpms "${RHEL_6_2_REQUIRES_RPMS[@]}")
elif [[ $(uname -r | grep "3.0") != "" ]];then
	result=$(test_rpms "${SUSE_11_REQUIRES_RPMS[@]}")
else
	echo "\n${BOLD}${RED}Error: OS Not recognized${DEFAULT}"
	status=1
fi

if [[ $result == "" ]]; then
	echo -e "Pre-Required RPMs\t\t\t" $(print_ok)
else
	echo "---------------------------------------"
	echo -e "Pre-Required RPMs\t\t\t" $(print_fail)
	echo -e ${result}
	echo "---------------------------------------"
	status=1
fi

# Check for vmcu rpms

# Check for required rpms only in case service is up
if [[ ${VALIDATOR_POST} == 1 ]]; then 
        result=$(test_plcm_rpms "${PLCM_RPMS[@]}")
        if [[ $result == "" ]]; then
                echo -e "vmcu RPMs\t\t\t\t" $(print_ok)
        else
                echo "---------------------------------------"
                echo -e  "vmcu RPMs\t\t\t\t" $(print_fail)
                echo -e ${result}
                echo "---------------------------------------"
                status=1
        fi
fi
result=""

##########################
# Check for linked libs  #
##########################
if [[ ${VALIDATOR_POST} == 1 ]]; then
	if [[ $(rpm -qa | grep -e "-Mcms-.*el5" ) ]];then
		result=$( test_ldd "$MCU_HOME_DIR/mcms/Bin:$MCU_LIBS" "$MCU_HOME_DIR/mcms/Bin" "/" )
		if [[ $result == "" ]]; then
			echo -e "MCMS code linked libs\t\t\t" $(print_ok)
		else
			echo -e "MCMS code linked libs\t\t\t" $(print_fail)
			echo -e ${result}
			status=1
		fi
	fi
	result=""

	if [[ $( rpm -qa | grep -e "-Cs-.*el5" ) ]];then
		result=$( test_ldd "$MCU_HOME_DIR/mcms/Bin:lib:$MCU_LIBS" "$MCU_HOME_DIR/cs/bin" "$MCU_HOME_DIR/cs" )
		if [[ $result == "" ]]; then
			echo -e "CS code linked libs\t\t\t" $(print_ok)
		else
			echo -e "CS code linked libs\t\t\t" $(print_fail)
			echo -e ${result}
			status=1
		fi
	fi
	result=""

	if [[ $( rpm -qa | grep -e "-Mpmx-.*el5" ) ]];then
		result=$( test_ldd "$MCU_HOME_DIR/mcms/Bin:$MCU_LIBS:$MCU_HOME_DIR/usr/share/MFA/mfa_x86_env/bin" "$MCU_HOME_DIR/usr/share/MFA/mfa_x86_env/bin" "$MCU_HOME_DIR/usr/share/MFA/mfa_x86_env/bin" )
		if [[ $result == "" ]]; then
		        echo -e "MPMX code linked libs\t\t\t" $(print_ok)
		else
		        echo -e "MPMX code linked libs\t\t\t" $(print_fail)
		        echo -e ${result}
			status=1
		fi
	fi
	result=""


	if [[ $( rpm -qa | grep -e "-Rmx1000-.*el5" ) ]];then
		result=$( test_ldd "$MCU_HOME_DIR/usr/rmx1000/bin:$MCU_LIBS" "$MCU_HOME_DIR/usr/rmx1000/bin" "$MCU_HOME_DIR/usr/rmx1000/bin" )
		if [[ $result == "" ]]; then
		        echo -e "Rmx1000 code linked libs\t\t" $(print_ok)
		else
		        echo -e "Rmx1000 code linked libs\t\t" $(print_fail)
		        echo -e ${result}
			status=1
		fi
	fi
	result=""

	if [[ $( rpm -qa | grep -e "-EngineMRM-.*el5" ) ]];then
	  result=$( test_ldd "$MCU_HOME_DIR/usr/share/EngineMRM/Bin-regular:$MCU_LIBS" "$MCU_HOME_DIR/usr/share/EngineMRM/Bin-regular" "$MCU_HOME_DIR/usr/share/EngineMRM/Bin-regular" )      
	  if [[ $result == "" ]]; then
	    result=$( test_ldd "$MCU_HOME_DIR/usr/share/EngineMRM/Bin-high:$MCU_LIBS" "$MCU_HOME_DIR/usr/share/EngineMRM/Bin-high" "$MCU_HOME_DIR/usr/share/EngineMRM/Bin-high" )	
	    if [[ $result == "" ]]; then
	      echo -e "EngineMRM code linked libs\t\t" $(print_ok)
	    else
	      echo -e "EngineMRMHigh code linked libs\t" $(print_fail)
	      echo -e ${result}
	      status=1
	    fi
	  else
	    echo -e "EngineMRMRegular code linked libs\t\t" $(print_fail)
	    echo -e ${result}
	    status=1
	  fi
	fi
	result=""

	if [[ $( rpm -qa | grep -e "-httpd-.*el5" ) ]];then
		result=$( test_ldd "$MCU_HOME_DIR/usr/local/apache2/lib:$MCU_LIBS" "$MCU_HOME_DIR/usr/local/apache2/bin" "$MCU_HOME_DIR/usr/local/apache2/bin" )
		if [[ $result == "" ]]; then
		        echo -e "httpd code linked libs\t\t\t" $(print_ok)
		else
		        echo -e "httpd code linked libs\t\t\t" $(print_fail)
		        echo -e ${result}
			status=1
		fi
	fi
	result=""
fi

###################
# Check accounts  #
###################
if [[ ${VALIDATOR_POST} == 1 ]]; then
	if [[ $(rpm -qa | grep -e "-Mcms-.*el[5-6]" ) ]];then
		if [[ $(cat /etc/passwd | grep ^mcms) ]];then
			echo -e "mcms account:\t\t\t\t" $(print_ok)
		else
			echo -e "mcms account:\t\t\t\t" $(print_fail)
			status=1
		fi

		if [[ $(cat /etc/group | grep ^polycom_mcu) ]];then
			echo -e "mcu group account:\t\t\t" $(print_ok)
		else
			echo -e "mcu group account:\t\t\t" $(print_fail)
			status=1
		fi
	fi

	#if [[ $(cat /etc/passwd | grep apache) ]];then
	#	echo -e "apache account:\t\t\t\t" $(print_ok)
	#else
	#	echo -e "apache account:\t\t\t\t" $(print_fail)
	#	status=1
	#fi
fi

echo -e "\n${BOLD}Status = ${status}${DEFAULT}" 
exit $status


