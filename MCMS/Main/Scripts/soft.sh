#!/bin/sh

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
BLINK=$(tput blink)
REVERSE=$(tput smso)
UNDERLINE=$(tput smul)

export MCU_HOME_DIR=
export MCU_HOME_DIR=$(grep MCU_HOME_DIR /home/mcms/.bashrc | cut -d'=' -f2 | awk 'NR==1')
export MCU_LIBS=$MCU_HOME_DIR/lib:$MCU_HOME_DIR/lib64:$MCU_HOME_DIR/usr/lib:$MCU_HOME_DIR/usr/lib64

Kernel=`uname -r | cut -d '-' -f 1`
# CentOS 5.8
if [[ "$Kernel" == "2.6.18" ]]; then
        RMX1000_RPM_FILE=${OFFICIAL_DIR}"/SoftMcuRPMs/RPMs/Plcm-Rmx1000-*.el5.x86_64.rpm"
# CentOs 6x
elif [[ "$Kernel" == "2.6.32" ]]; then
        RMX1000_RPM_FILE=${OFFICIAL_DIR}"/SoftMcuRPMs/RPMs/Plcm-Rmx1000-*.el6.x86_64.rpm"
fi

LAST_BUILD="/Carmel-Versions/CustomerBuild/RMX_100.0/200_last"

BUILD_LIST=(MCMS CS MPMX MC AUDIO VIDEO ENGINE)
DIR_LIST=(MCMS_DIR CS_DIR MPMX_DIR MRMX_DIR ENGINE_DIR)
OUT_LIST=(MCMS_OUT CS_OUT MPMX_OUT MRMX_OUT ENGINE_OUT)
WHO_AM_I=`whoami`
START_UP_LOG=$MCU_HOME_DIR/tmp/startup_logs/soft_mcu_start_up.log
SERVICE_OUTPUT_LOG=$MCU_HOME_DIR/tmp/startup_logs/soft_mcu_service_output
OUTPUT_FOLDERS='apache core firmware cdr rec cslogs cslogs/cs1 cslogs/cs2
                cslogs/cs3 cslogs/cs4 cslogs/cs5 cslogs/cs6 cslogs/cs7 cslogs/cs8 faults tmp IVR mylogs media_rec audit local_tracer 
                media nids links tcp_dump tcp_dump/mcms tcp_dump/emb 802_1x 802_1x/emb 802_1x/emb/media1 802_1x/emb/media2 802_1x/emb/media3 802_1x/emb/media4 802_1x/emb/switch log backup external_ivr'

log()
{
  if [ $# -ne 1 ]; then
    echo "Usage: log \"message to log\""
    return 0
  fi

  STAMP=`date +"%Y%m%d%H%M%S"`
  echo -e "$STAMP $1" | tee -a $START_UP_LOG
  return 0
}

export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin
export NATIVE_TOOLCHAIN=YES

export BUILD_MCMS=TRUE
export BUILD_CS=TRUE
export BUILD_MPMX=TRUE
export BUILD_MC=TRUE
export BUILD_AUDIO=TRUE
export BUILD_VIDEO=TRUE
export BUILD_ENGINE=TRUE

export Kernel

# Localization Support
export LANG=en_US.UTF-8

################################ HELP ##################################
if [ "$1" == "" ]
then
	echo "usage: soft.sh COMMAND"
	echo "commands:"
	echo "make [MCMS|CS|MPMX|MC|AUDIO|VIDEO|ENGINE]- build all soft mcu components (native toolchain mode)"
	echo "make_clean - clean all soft mcu components"
	echo "clean - clean all soft mcu components"
	echo "start - start all soft mcu processes"
	echo "start [process] - start all soft mcu processes and test specific process with Valgrind, process={mcms process, mfa, mrmx process} - example: soft.sh start mfa" 	
	echo "target - start all soft mcu processes, on an rpm installed target"
	echo "stop - stop all soft mcu processes"
	echo
	echo "mandatory environment variables:"
	echo "MCMS_DIR - MCMS main folder"
	echo "CS_DIR - CS main folder"
	echo "MPMX_DIR - MPMX main folder"
	echo "MRMX_DIR - MRMX main folder (MC, AUDIO, VIDEO sources)"
	echo "ENGINE_DIR - Engine MRM main folder"
	echo "MEMORY_MODE (High\Low) - Memory mode simulation (default is low)"
	echo
	echo "mandatory Soft MCU testing environment variables:"
	echo "CALLGEN_IP - Call Generator IP"
	echo "EP1 - First End Point IP"
	echo "EP2 - Second End Point IP"

	echo "optional environment variables:"
	echo "FARM=YES  - for fast compilation over farm" 
	exit 
fi



#################################################################################
#										#
#				SERVICE FUNCTIONS				#
#										#
#################################################################################


###########################################################################
setup_env (){

	mkdir -p $MCU_HOME_DIR/tmp/startup_logs/
	mkdir -p $MCU_HOME_DIR/tmp/mfa_x86_env/bin $MCU_HOME_DIR/tmp/mfa_x86_env/cfg $MCU_HOME_DIR/tmp/mfa_x86_env/logs
	mkdir -p $MCU_HOME_DIR/tmp/mfa_x86_env/mcms_share/cfg $MCU_HOME_DIR/tmp/mfa_x86_env/mcms_share/bin
	
	if [[ $MEMORY_MODE == "" ]]; then
		export MEMORY_MODE=Low
	fi
	 
	# CentOs 6x
	if [[ "$Kernel" == "2.6.32" ]]; then
		if [ -e Bin.i32cent6x ]; then
			echo "Copy SOFT MCU binaries CentOS 6x into Bin folder..."
			rm -Rf Bin/*
			(
				cd Bin
				find ../Bin.i32cent6x -type f -exec ln -sf {} . \;
				find ../Bin.i32cent6x -type l -exec ln -sf {} . \;
				#ln -sf ../Bin.i32ptx/lib* ../Bin.i32ptx/httpd ../Bin.i32ptx/mod_polycom.so  ../Bin.i32ptx/libMcmsCs.so \
				#../Bin.i32ptx/snmpd ../Bin.i32ptx/openssl ../Bin.i32ptx/ApacheModule ../Bin.i32ptx/McuCmd .
				#ln -sf ../Bin.i32cent6x/* . 
				#ln -sf $MCU_HOME_DIR/usr/local/apache2/bin/httpd .
			) 2> /dev/null
		fi 
	else
		if [ -e Bin.i32cent56 ]; then
			echo "Copy SOFT MCU binaries CentOS 5.6 into Bin folder..."
			rm -Rf Bin/*
			(
				cd Bin
				find ../Bin.i32cent56 -type f -exec ln -sf {} . \;
				find ../Bin.i32cent56 -type l -exec ln -sf {} . \;
				#ln -sf ../Bin.i32ptx/lib* ../Bin.i32ptx/httpd ../Bin.i32ptx/mod_polycom.so  ../Bin.i32ptx/libMcmsCs.so \
				#../Bin.i32ptx/snmpd ../Bin.i32ptx/openssl ../Bin.i32ptx/ApacheModule ../Bin.i32ptx/McuCmd .
				#ln -sf ../Bin.i32cent56/* . 
			) 2> /dev/null
		fi
	fi

	#Fix INFRA-38
	#rm -f Bin/ApacheModule

	if [[ $MCMS_DIR == "$OFFICIAL_DIR/mcms/" ]];then
		if [[ $CLEAN_CFG != "NO" ]];then
			rm -rf $HOME/dev_env/*
		fi
                setup_env_official_mcms
        fi
	
	rm $MCU_HOME_DIR/tmp/mcms
	ln -sf $MCMS_DIR $MCU_HOME_DIR/tmp/mcms
	mkdir $MCMS_DIR/EMACfg

	rm $MCU_HOME_DIR/tmp/mcu_custom_config
	mkdir $HOME/dev_env/mcu_custom_config
	ln -sf $HOME/dev_env/mcu_custom_config $MCU_HOME_DIR/tmp/mcu_custom_config
	
	if [[ $ENGINE_DIR == "$OFFICIAL_DIR/EngineMRM/" ]]; then
                setup_env_official_engine
        fi

	if [[ $CS_DIR == "$OFFICIAL_DIR/cs_smcu/CS/" ]];then
                setup_env_official_cs
        fi
	
	if [[ $MPMX_DIR != "$OFFICIAL_DIR/MediaCard/MFA/MFA" ]]; then
                cd $MPMX_DIR
                ./scripts_x86/build-x86_env.sh
                cd -
        fi
}

###########################################################################
setup_env_official_mcms (){
	
	echo -n "Setup your mcms cfg env..."
	USER_MCMS_DIR="$HOME/dev_env/mcms/"
	mkdir -p $USER_MCMS_DIR
	cd $USER_MCMS_DIR &> /dev/null
	mkdir -p Audit Backup CdrFiles Cores Faults IVRX Keys KeysForCS LogFiles MediaRecording Restore States TestResults Bin EMACfg TS
	cd - &> /dev/null
	
	#ln -sf $MCMS_DIR/EMA $USER_MCMS_DIR/EMA
	ln -sf $MCMS_DIR/Libs $USER_MCMS_DIR/Libs
	ln -sf $MCMS_DIR/MIBS $USER_MCMS_DIR/MIBS
	ln -sf $MCMS_DIR/Scripts $USER_MCMS_DIR/Scripts
	#ln -sf $MCMS_DIR/StaticCfg $USER_MCMS_DIR/StaticCfg
	ln -sf $MCMS_DIR/VersionCfg $USER_MCMS_DIR/VersionCfg
	ln -sf $MCMS_DIR/Makefile $USER_MCMS_DIR/Makefile
	ln -sf $MCMS_DIR/Main.mk $USER_MCMS_DIR/Main.mk
	ln -sf $MCMS_DIR/Processes $USER_MCMS_DIR/Processes

	if [ ! -d $USER_MCMS_DIR/Cfg ]; then
		cp -rf $MCMS_DIR/Cfg $USER_MCMS_DIR
	fi
	if [ ! -d $USER_MCMS_DIR/IVRX ]; then
		cp -rf $MCMS_DIR/IVRX $USER_MCMS_DIR
	fi
	if [ ! -d $USER_MCMS_DIR/Utils ]; then
		cp -rf $MCMS_DIR/Utils $USER_MCMS_DIR
	fi

	# Fix SA on OFFICIAL BUILD
	cp -rf ${MCMS_DIR}/StaticCfg $USER_MCMS_DIR
	cp -rf ${MCMS_DIR}/EMA $USER_MCMS_DIR
	chmod u+w $USER_MCMS_DIR/StaticCfg/httpd.conf.sim
	if [[ ! `tail -1 $USER_MCMS_DIR/StaticCfg/httpd.conf.sim | grep '^User'` ]]; then
	        echo "User `whoami`" >> $USER_MCMS_DIR/StaticCfg/httpd.conf.sim
	fi
	#rm -f $USER_MCMS_DIR/StaticCfg/httpd.conf
	#cd $USER_MCMS_DIR/StaticCfg
	#ln -s httpd.conf.sim httpd.conf
	#cd -

	echo "Copy SOFT MCU binaries into Bin folder..."
	# CentOs 6x
	if [[ "$Kernel" == "2.6.32" ]]; then
		rm -Rf $USER_MCMS_DIR/Bin/*
		(
			cd $USER_MCMS_DIR/Bin
			#ln -sf $MCMS_DIR/Bin.i32ptx/lib* $MCMS_DIR/Bin.i32ptx/httpd $MCMS_DIR/Bin.i32ptx/mod_polycom.so  $MCMS_DIR/Bin.i32ptx/libMcmsCs.so \
			ln -sf $MCMS_DIR/Bin.i32ptx/lib* $MCMS_DIR/Bin.i32ptx/mod_polycom.so  $MCMS_DIR/Bin.i32ptx/libMcmsCs.so \
			$MCMS_DIR/Bin.i32ptx/snmpd $MCMS_DIR/Bin.i32ptx/openssl $MCMS_DIR/Bin.i32ptx/ApacheModule $MCMS_DIR/Bin.i32ptx/McuCmd .
			ln -sf $MCMS_DIR/Bin.i32cent6x/* . 
			ln -sf $MCU_HOME_DIR/usr/local/apache2/bin/httpd .
		) 2> /dev/null
	else
		rm -Rf $USER_MCMS_DIR/Bin/*
		(
			cd $USER_MCMS_DIR/Bin
			ln -sf $MCMS_DIR/Bin.i32ptx/lib* $MCMS_DIR/Bin.i32ptx/httpd $MCMS_DIR/Bin.i32ptx/mod_polycom.so  $MCMS_DIR/Bin.i32ptx/libMcmsCs.so \
			$MCMS_DIR/Bin.i32ptx/openssl $MCMS_DIR/Bin.i32ptx/ApacheModule $MCMS_DIR/Bin.i32ptx/McuCmd .
			ln -sf $MCMS_DIR/Bin.i32cent56/* . 
		) 2> /dev/null
	fi

	

	export MCMS_DIR=$USER_MCMS_DIR
	rm $MCU_HOME_DIR/tmp/mcms
	
	echo "Done."
}

###########################################################################
setup_env_official_cs (){
	
	echo -n "Setup your cs env..."

	USER_CS_DIR="$HOME/dev_env/cs/"
	mkdir -p $USER_CS_DIR
	cd $USER_CS_DIR &> /dev/null
	mkdir -p logs/cs1 ocs/cs1
	cd - &> /dev/null
	
	rm -f $USER_CS_DIR/bin  $USER_CS_DIR/scripts  $USER_CS_DIR/lib	
	ln -sf $CS_DIR/bin $USER_CS_DIR/bin
	ln -sf $CS_DIR/scripts $USER_CS_DIR/scripts
	ln -sf $CS_DIR/lib $USER_CS_DIR/lib

	if [ ! -d $USER_CS_DIR/cfg ]; then
		##cp -rf $CS_DIR/cfg $USER_CS_DIR
		ln -sf $CS_DIR/cfg $USER_CS_DIR/cfg
	fi

	export CS_DIR=$USER_CS_DIR
	rm $MCU_HOME_DIR/tmp/cs
	ln -sf $CS_DIR $MCU_HOME_DIR/tmp/cs
	
	echo "Done."
}

###########################################################################
setup_env_official_engine (){
	
	echo -n "Setup your EngineMRM env..."

	USER_ENGINE_MRM_DIR="$HOME/dev_env/EngineMRM/"
	mkdir -p $USER_ENGINE_MRM_DIR
	cd $USER_ENGINE_MRM_DIR &> /dev/null
	rm -rf *
	ln -sf $ENGINE_DIR/Scripts ./
	ln -sf $ENGINE_DIR/mrm.sh ./
	ln -sf $ENGINE_DIR/night.sh ./
	ln -sf /tmp ./
	cd - &> /dev/null

	if [[ $MEMORY_MODE == "High" ]]; then
		echo "$OFFICIAL_DIR/EngineMRM/Bin-high/"
		echo "$USER_ENGINE_MRM_DIR/Bin"
		ln -sf "$OFFICIAL_DIR/EngineMRM/Bin-high/" "$USER_ENGINE_MRM_DIR/Bin"
	        echo "Running high memory mode simulation" 
	else
		echo "$OFFICIAL_DIR/EngineMRM/Bin-regular/"
		echo "$USER_ENGINE_MRM_DIR/Bin"
		ln -sf "$OFFICIAL_DIR/EngineMRM/Bin-regular" "$USER_ENGINE_MRM_DIR/Bin"
	        echo "Running low memory mode simulation" 
	fi

	export ENGINE_DIR=$USER_ENGINE_MRM_DIR
	echo "Done."
}

###########################################################################
export_official () {
	
	case $1 in
	MCMS_DIR)
		export $1=$OFFICIAL_DIR/mcms/
		;;
	CS_DIR)
		export $1=$OFFICIAL_DIR/cs_smcu/CS/
		;;
	MPMX_DIR)
		export $1=$OFFICIAL_DIR/MediaCard/MFA/MFA
		;;
	MRMX_DIR)
		export $1=$OFFICIAL_DIR/MRMX/
		;;
	ENGINE_DIR)
		export $1=$OFFICIAL_DIR/EngineMRM/
		;;
	esac
}

##############################################################################
test_using_official (){

	OFFICIAL=`echo $1 | grep $OFFICIAL_DIR`
	if [[ $OFFICIAL != "" || $1 == "$USER_MCMS_DIR" ]]; then
		echo " - Using Official Build $OFFICIAL_DIR"
	else
		echo " - On a private path: $1"
	fi
}


##############################################################################
mark_not_to_build () {

	case $1 in
	MCMS_DIR)
		export BUILD_MCMS=FALSE
		;;
	CS_DIR)
		export BUILD_CS=FALSE
		;;
	MPMX_DIR)
		export BUILD_MPMX=FALSE
		;;
	MRMX_DIR)
		export BUILD_MC=FALSE
		export BUILD_AUDIO=FALSE
		export BUILD_VIDEO=FALSE
		;;
	ENGINE_DIR)
		export BUILD_ENGINE=FALSE
		;;
	esac
}

#############################################################################
echo_build_projects (){
	echo
	echo "building projects:"
	echo "=================="
	for build in ${BUILD_LIST[*]}
	do
		echo `env | grep BUILD_$build=`
	done

	echo
}

#############################################################################
test_var (){
	VAR=`env | grep $1 | cut -d'=' -f2`
	if [  "$VAR" == "" ]; then
		LOOP="true"
		while [[ "$LOOP" == "true" ]]; do
			clear
			echo -e  "Please specify $1 directory:\n1. Use $1 from last build\n2. export $1=PATH (.../vob/MCMS/Main)"
			read -p "Your choice [1/2]:" 	
			case $REPLY in
			1)
				export_last $1
				LOOP="false";
				;;
			2)
				read -p "export $1="
				export "$1"="$REPLY"
				LOOP="false"
				;;
			*)
				echo "Invalid choice..."
				sleep 1
				;;				
			esac
		done
	fi
}	

#############################################################################
#Point Projects directories that were not export to 'last', and mark them not to be built
test_dir (){
	
	OFFICIAL=`env | grep "OFFICIAL_DIR" | cut -d'=' -f2`
	VAR=`env | grep ^$1 | cut -d'=' -f2`
	#Test official build dir
	if [ "$OFFICIAL" != "" ]; then
		if [ -d "$OFFICIAL_DIR" ]; then
			
			if [  "$VAR" == "" ]; then
				export_official $1 
				mark_not_to_build $1
			else		
				TMP=`env | grep $1 | cut -d'=' -f2 | grep NonStableBuild`
				if [ "$TMP" != "" ]; then
					mark_not_to_build $1
				fi
			fi

		else
			echo "OFFICIAL_DIR is invalid: $OFFICIAL_DIR"
			exit 1
		fi
	else
		#subproject 
		if [  "$VAR" == "" ]; then
			echo "Please specify $1 directory by export $1=PATH (.../vob/MCMS/Main)"
			exit 1
		else		
			TMP=`env | grep $1 | cut -d'=' -f2 | grep NonStableBuild`
			if [ "$TMP" != "" ]; then
				mark_not_to_build $1
			fi
		fi
	fi
}


################################ VARIABLES VERIFICATION ##################################
verify_variables () {
	for var in ${DIR_LIST[*]}
	do
		test_dir $var
	done

	echo ""
	echo "Verifying variables:"
	echo "===================="

	if [ -e $MCMS_DIR/Scripts/soft.sh ]
	then
	 	echo -n "MCMS_DIR is verified"
		test_using_official $MCMS_DIR
	else
		echo "MCMS_DIR is invalid:" $MCMS_DIR
		return 1
	fi

	if [ -e $CS_DIR/bin/loader ]
	then
	 	echo -n "CS_DIR is verified"
		test_using_official $CS_DIR
	else
		echo "CS_DIR is invalid:" $CS_DIR
		return 1
	fi

	if [ -e $MPMX_DIR/scripts_x86/runmfa.sh ]
	then
	 	echo -n "MPMX_DIR is verified"
		test_using_official $MPMX_DIR
	else
		echo "MPMX_DIR is invalid:" $MPMX_DIR
		return 1
	fi

	if [ -e $MRMX_DIR/mp*proxy ]
	then
		echo -n "MRMX_DIR is verified"
		test_using_official $MRMX_DIR
	else
		echo "MRMX_DIR is invalid:" $MRMX_DIR
		return 1
	fi

	if [ -e $ENGINE_DIR/Scripts/ ]
	then
	 	echo -n "ENGINE_DIR is verified"
		test_using_official $ENGINE_DIR
	else
		echo "ENGINE_DIR is invalid:" $ENGINE_DIR
		return 1
	fi
}

################################## Make MCMS #############################
make_MCMS() {
	cd $MCMS_DIR
	if [[ "$KLOCWORK" == "YES" ]]; then
		echo "This compilation will run KLOCWORK analyze!!!"
		make active
		/opt/polycom/soft_mcu/KW_local_analyze.sh MCMS_7_6_1S " " ./make.sh  || echo "Klocwork - Nothing to analyze. "
		return 0
	fi
	
	if [[ $? == 0 ]]; then
		make active
		./make.sh || return 1
	else	
			return 1
	fi
}

################################## Make CS #############################
make_CS() {

	cd $CS_DIR
	if [[ $? == 0 ]]; then

		if [ -e $MCMS_DIR/CSFirstRun ]
		then
			echo "Compile the CS without -C"
			./csmake || return 1
		else
		 	echo "Compile the CS with -C"
	                touch $MCMS_DIR/CSFirstRun
			./csmake -C || return 1
		fi

	else
		return 1
	fi
}

################################## Make MediaCard #############################
make_MPMX() {
	cd $MPMX_DIR
	if [[ $? == 0 ]]; then
		./make_common/mfamake || return 1
		./scripts_x86/build-x86_env.sh || return 1
	else	
		return 1
	fi
}

################################## Make MC #############################
make_MC() {
	ln -sf $RMX1000_RPM_FILE $MCU_HOME_DIR/tmp/rmx1000.rpm
	. $MCMS_DIR/Scripts/InstallRmx1000rpm.sh	
	#cd $MRMX_DIR/mermaid/
	cd $MRMX_DIR/mp_proxy
	if [[ $? == 0 ]]; then
		make -j4 || return 1
	else	
		return 1
	fi
}

################################## Make AUDIO #############################
make_AUDIO() {
	cd $MRMX_DIR/ampSoft/
	if [[ $? == 0 ]]; then
		echo "Compile audio"
		make -j4 || return 1
	else	
		return 1
	fi
}

################################## Make VIDEO #############################
make_VIDEO() {
	cd $MRMX_DIR/vmp/
	if [[ $? == 0 ]]; then
		echo "Compile video"
		source $MRMX_DIR/vmp/ia64_pt
		make -j8 || return 1
		make install
	else	
		return 1
	fi
}
################################## Make ENGINE #############################
make_ENGINE() {

	cd $ENGINE_DIR

	if [[ "$KLOCWORK" == "YES" ]]; then
		echo "This compilation will run KLOCWORK analyze!!!"
		/opt/polycom/soft_mcu/KW_local_analyze.sh EngineMRM_7_6_1S " " make  ||  echo "Klocwork - Nothing to analyze."
		return 0
	fi
	
	if [[ "$FARM" == "YES" ]]; then
		export DISTCC_DIR=/nethome/sagia/distcc
		export MAKEPARM=-j36
		export DISTCC_BIN=$DISTCC_DIR/bin/distcc
		export PREMAKE=$DISTCC_DIR/bin/pump
		unset DISTCC_HOSTS
	fi

	if [[ $? == 0 ]]; then
		$PREMAKE make $MAKEPARM || return 1
	else	
		return 1
	fi
}

################################## MAKE All##################################
make_all_projects (){
	
	if [ `whoami` == "root" ]
	then
		echo "You can't run this script as root!!!"
		exit
	fi	
	
	#If 'soft.sh make TARGET' is used, export only this TARGET as TRUE and the rest with FALSE
	if [[ "$1" != "" ]]
	then
		for build in ${BUILD_LIST[*]}
		do
			if [ "$build" == $1 ]; then
				export BUILD_$build=TRUE
			else
				export BUILD_$build=FALSE
			fi
		done
	fi	

	echo_build_projects
	
	#Loop to build each project
	for build in ${BUILD_LIST[*]}
	do
		if [ `env | grep BUILD_$build= | cut -d'=' -f2` == TRUE ]; then
			trace_start_build $build
			make_$build
			compile_result $? "BUILD_$build"
		fi
	done
	
	compile_result 0 "ALL"
}

################################## MAKE CLEAN ##################################
make_clean (){


	if [ `whoami` == "root" ]
	then
		echo "You can't run this script as root!!!"
		exit
	fi

#Cleaning
	clean

# Building
	echo "building all soft mcu process after clean"

	make_all_projects

	exit 0 
}


################################## CLEAN ##################################
clean () {
	if [ `whoami` == "root" ]
	then
		echo "You can't run this script as root!!!"
		exit
	fi

	echo "cleaning soft mcu modules"
	echo_build_projects
	
	if [ "$BUILD_MCMS" == "TRUE" ]; then
		cd $MCMS_DIR
		./fast_clean.sh
	fi
	#cd $CS_DIR
	#Force CS make clean by removing the following file:
	if [ "$BUILD_CS" == "TRUE" ]; then
		rm -f $MCMS_DIR/CSFirstRun
	fi

	if [ "$BUILD_MPMX" == "TRUE" ]; then
		cd $MPMX_DIR
		make clean
	fi

	if [ "$BUILD_MC" == "TRUE" ]; then
		#cd $MRMX_DIR/mermaid/
		cd $MRMX_DIR/mp_proxy
		make clean
	fi

	if [ "$BUILD_AUDIO" == "TRUE" ]; then
		cd $MRMX_DIR/ampSoft/
		make clean
	fi
	if [ "$BUILD_VIDEO" == "TRUE" ]; then
		cd $MRMX_DIR/vmp/
		source ./ia64_pt
		make clean
	fi

	if [ "$BUILD_ENGINE" == "TRUE" ]; then
		cd $ENGINE_DIR
		make clean
	fi
}


################################## MAKE RPM ################################
make_rpm () {

	if [ `whoami` == "root" ]
	then
		echo "You can't run this script as root!!!"
		exit
	fi

	echo "make rpm for soft mcu modules"
	echo_build_projects
	
	if [ "$BUILD_MCMS" == "TRUE" ]; then
		echo "Make MCMS RPM"
		cd $MCMS_DIR
		make active
		./MakeRpm.sh no
	fi

	
	if [ "$BUILD_CS" == "TRUE" ]; then
		echo "Make CS RPM"
		cd $CS_DIR
		./MakeRpm.sh no
	fi

	if [ "$BUILD_MPMX" == "TRUE" ]; then
		echo "Make MPMX RPM"
		cd $MPMX_DIR
		./make_common/MakeRPM.sh no
	fi

	if [ "$BUILD_MC" == "TRUE" ]; then
		echo "Make mp_proxy and amp RPM"
		cd $MRMX_DIR/
		./MakeRmx1000Rpm.sh yes
	fi


	if [ "$BUILD_AUDIO" == "TRUE" ]; then
		echo "Make Audio(Amp) RPM"
		cd $MRMX_DIR/ampSoft/
		./MakeRPM.sh no
	fi
	if [ "$BUILD_VIDEO" == "TRUE" ]; then
		echo "Make Video(Vmp) RPM"
		cd $MRMX_DIR/vmp/
		./MakeRPM.sh no
	fi
	if [ "$BUILD_ENGINE" == "TRUE" ]; then
		echo "Make engine RPM"
		cd $ENGINE_DIR
		./MakeRPM.sh no
	fi
}

################################## PRIVATE_BUILD ##################################
private_build(){
	if [ -d "$OFFICIAL_DIR" ]; then
		echo "Creating Private build:"
		# Create private rpms
		make_rpm
		# Prepare dir
		rm -rf	~/SoftMcu_PrivateBuild
		mkdir -p ~/SoftMcu_PrivateBuild
		
		# Copy original rpms
		cp $OFFICIAL_DIR/SoftMcuRPMs/RPMs/*.rpm ~/SoftMcu_PrivateBuild/
		
		# Copy specific rpms - per project
		if [ "$BUILD_MCMS" == "TRUE" ]; then
			rm ~/SoftMcu_PrivateBuild/Plcm-Mcms-*.i386.rpm
			echo "Copy MCMS RPM"
			cp ~/McmsRpmbuild/rpmbuild/RPMS/i386/Plcm-Mcms-*.i386.rpm ~/SoftMcu_PrivateBuild/
		fi
	
		if [ "$BUILD_CS" == "TRUE" ]; then
			rm ~/SoftMcu_PrivateBuild/Plcm-Cs-*.i386.rpm
			echo "Copy CS RPM"
			cp ~/CsRpmBuild/rpmbuild/RPMS/i386/Plcm-Cs-*.i386.rpm ~/SoftMcu_PrivateBuild/
		fi

		if [ "$BUILD_MPMX" == "TRUE" ]; then
			rm ~/SoftMcu_PrivateBuild/Plcm-Mpmx-*.i386.rpm
			echo "Copy MPMX RPM"
			cp ~/MediaCardRpm/rpmbuild/RPMS/i386/Plcm-Mpmx-*.i386.rpm ~/SoftMcu_PrivateBuild/
		fi

		if [ "$BUILD_MC" == "TRUE" ]; then
			rm ~/SoftMcu_PrivateBuild/Plcm-Rmx1000-*.i386.rpm
			rm ~/SoftMcu_PrivateBuild/Plcm-AmpSoft-*.i386.rpm
			rm ~/SoftMcu_PrivateBuild/Plcm-VmpSoft-*.x86_64.rpm
			rm ~/SoftMcu_PrivateBuild/Plcm-MpProxy-*.i386.rpm
			echo "Copy MpProxy, amp and vmp RPM"
			cp ~/Rmx1000Rpmbuild/rpmbuild/RPMS/i386/Plcm-Rmx1000-*.i386.rpm ~/SoftMcu_PrivateBuild/
			cp ~/Rmx1000Rpmbuild/rpmbuild/RPMS/i386/Plcm-AmpSoft-*.i386.rpm ~/SoftMcu_PrivateBuild/
			cp ~/Rmx1000Rpmbuild/rpmbuild/RPMS/x86_64/Plcm-VmpSoft-*.x86_64.rpm ~/SoftMcu_PrivateBuild/
			cp ~/Rmx1000Rpmbuild/rpmbuild/RPMS/i386/Plcm-MpProxy-*.i386.rpm ~/SoftMcu_PrivateBuild/
		fi

		if [ "$BUILD_ENGINE" == "TRUE" ]; then
			rm ~/SoftMcu_PrivateBuild/Plcm-EngineMRM-*.x86_64.rpm
			echo "Copy engine RPM"
			cp ~/EngineRpm/rpmbuild/RPMS/x86_64/Plcm-EngineMRM-*.x86_64.rpm ~/SoftMcu_PrivateBuild/
		fi
		echo -e ${GREEN}"Private build is ready at: $HOME/SoftMcu_PrivateBuild/"${NO_COLOR}
	else
		echo "OFFICIAL_DIR is invalid: $OFFICIAL_DIR"
		exit 1
	fi	
}

################################## START ##################################
install_build(){
	if [ `whoami` == "root" ]
	then
		echo "You can't run this script as root!!!"
		exit 1
	fi
	if [[ $1 == "" ]]; then
		echo "Usage: soft.sh install IP_ADDRESS"
		exit 1
	fi
	# Validate dest address
	ping -c 1 $1 &> /dev/null
	if [[ $? != 0 ]]; then
		echo -e ${RED}"$1 - Address is not reachable"${NO_COLOR}
		exit 1
	else
		ssh root@$1 'rm -rf $MCU_HOME_DIR/tmp/RPMs; mkdir -p $MCU_HOME_DIR/tmp/RPMs; service soft_mcu stop'
		echo "Copying rpm files to remote machine"
		scp ~/SoftMcu_PrivateBuild/*.rpm root@$1:$MCU_HOME_DIR/tmp/RPMs/
		echo "Installing remote machine"
		#ssh root@$1 'if [[ `rpm -qa | grep "Plcm" ` != "" ]]; then rpm -e Plcm-SoftMcuMain-* Plcm-EngineMRM-* Plcm-UI_AIM-* Plcm-jsoncpp-* Plcm-libphp-* Plcm-httpd-* Plcm-VmpSoft-* Plcm-MpProxy-* Plcm-AmpSoft-* Plcm-Rmx1000-* Plcm-Mpmx-* Plcm-Cs-* Plcm-Ema-* Plcm-Mcms-*; fi; cd $MCU_HOME_DIR/tmp/RPMs; rpm -ivh Plcm-SoftMcuMain-* Plcm-EngineMRM-* Plcm-UI_AIM-* Plcm-jsoncpp-* Plcm-libphp-* Plcm-httpd-* Plcm-VmpSoft-* Plcm-MpProxy-* Plcm-AmpSoft-* Plcm-Rmx1000-* Plcm-Mpmx-* Plcm-Cs-* Plcm-Ema-* Plcm-Mcms-*; if [[ $? == 0 ]]; then echo "===== INSTALLATION FINISHED ===="; else echo "===== INSTALLATION FAILED ====";fi'
		ssh root@$1 'if [[ `rpm -qa | grep "Plcm" ` != "" ]]; then rpm -e Plcm-SoftMcuMain-* Plcm-EngineMRM-* Plcm-UI_AIM-* Plcm-jsoncpp-* Plcm-VmpSoft-* Plcm-MpProxy-* Plcm-AmpSoft-* Plcm-Rmx1000-* Plcm-Mpmx-* Plcm-Cs-* Plcm-Ema-* Plcm-Mcms-* Plcm-SingleApache-*; fi; cd $MCU_HOME_DIR/tmp/RPMs; rpm -ivh Plcm-SoftMcuMain-* Plcm-EngineMRM-* Plcm-UI_AIM-* Plcm-jsoncpp-* Plcm-VmpSoft-* Plcm-MpProxy-* Plcm-AmpSoft-* Plcm-Rmx1000-* Plcm-Mpmx-* Plcm-Cs-* Plcm-Ema-* Plcm-Mcms-* Plcm-SingleApache-*; if [[ $? == 0 ]]; then echo "===== INSTALLATION FINISHED ===="; else echo "===== INSTALLATION FAILED ====";fi'
		 
	fi
}

################################## SETUP_VM ##################################
setup_sim_license_file(){

	# select license file by product type
	if [ "$LICENSE_FILE" == "" ]; then

		PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`

		if [ $PRODUCT_TYPE == "SOFT_MCU_EDGE" ]; then
			LICENSE_FILE="VersionCfg/Keycodes_SoftMcuEdgeAxis_20_HD.cfs"
			mkdir $MCU_HOME_DIR/mcms/TS
		elif [ $PRODUCT_TYPE == "SOFT_MCU_CG" ]; then
			LICENSE_FILE="VersionCfg/Keycodes_SoftMcuCG.cfs"
		else
			LICENSE_FILE="VersionCfg/Keycodes_SoftMcu_20_HD.cfs"
		fi
	fi

	echo ""
	echo "Using License file for simulation:" $LICENSE_FILE
	
	mkdir -p $MCU_HOME_DIR/config/sysinfo
	chmod a+w $MCU_HOME_DIR/config/sysinfo
	
	cp $MCMS_DIR/$LICENSE_FILE $MCU_HOME_DIR/config/sysinfo/keycode.txt
	chmod a+w $MCU_HOME_DIR/config/sysinfo/keycode.txt
	
}

################################## SETUP_VM ##################################
setup_vm_all_products (){

	if [ `whoami` == "root" ]
	then
		echo "You can't run this script as root!!!"
		exit
	fi

	cd $MCMS_DIR
	stop
	
        if [[ -z "$SOFT_MCU" ]]
        then
            make active
        fi

	setup_env

	rm $MCU_HOME_DIR/tmp/mcms
	ln -sf $MCMS_DIR $MCU_HOME_DIR/tmp/mcms

	rm $MCU_HOME_DIR/tmp/mcu_custom_config
	ln -sf $HOME/dev_env/mcu_custom_config $MCU_HOME_DIR/tmp/mcu_custom_config
	
	#Fix BRIDGE-12692
	echo $1 > $MCU_HOME_DIR/mcms/ProductType
		
	ln -sf $RMX1000_RPM_FILE $MCU_HOME_DIR/tmp/rmx1000.rpm
	. $MCMS_DIR/Scripts/InstallRmx1000rpm.sh
	
	export VM=YES

	//support mcu_custom_config on mfw vm also
	if [[ $PRODUCT_TYPE == "SOFT_MCU_MFW" ]]; then
		create_custom_cfg_path
	fi

	if [[ $ENGINE_DIR != "$OFFICIAL_DIR/EngineMRM/" ]]; then
		if [[ $MEMORY_MODE == "High" ]]; then
			ln -sf "$ENGINE_DIR/EngineMRM/Bin-high/" "$USER_ENGINE_MRM_DIR/Bin"
			echo "Running high memory mode simulation" 
		else
			ln -sf "$ENGINE_DIR/EngineMRM/Bin-regular" "$USER_ENGINE_MRM_DIR/Bin"
			echo "Running low memory mode simulation" 
		fi
	fi


	# Start of Block for Single Apache   - kobig , remove this link in VM=YES
#       rm -f $MCU_HOME_DIR/mcms/Bin/httpd 2> /dev/null
#       ln -sf $MCU_HOME_DIR/usr/local/apache2/bin/httpd $MCU_HOME_DIR/mcms/Bin/
	#sudo /bin/rm $MCU_HOME_DIR/tmp/httpd.rest.conf
#       cp -f $MCU_HOME_DIR/usr/local/apache2/conf/httpd.rest.conf $MCU_HOME_DIR/tmp
	#cp -f $MCU_HOME_DIR/mcms/StaticCfg/httpd.rest.conf $MCU_HOME_DIR/tmp
#	chmod u+w $MCU_HOME_DIR/tmp/httpd.rest.conf




	#sed -i 's#^Include.*\/httpd\.rest\.conf.*$##' $MCU_HOME_DIR/tmp/mcms/StaticCfg/httpd.conf.sim
	#chmod u+w $MCU_HOME_DIR/tmp/httpd.rest.conf
	#sed -i 's/^.*User .*$//' $MCU_HOME_DIR/tmp/httpd.rest.conf
	#echo "User `whoami`" >> $MCU_HOME_DIR/tmp/httpd.rest.conf
	mkdir $MCU_HOME_DIR/mcms/logs 2> /dev/null
	chmod a+rwx $MCU_HOME_DIR/mcms/logs
	#echo "Include $MCU_HOME_DIR/tmp/httpd.rest.conf" >> $MCU_HOME_DIR/tmp/mcms/StaticCfg/httpd.conf.sim
	if [[ ! `stat -L $MCU_HOME_DIR/mcms/Bin/httpd` ]]; then
		echo "Replacing httpd with the local one"
		rm -f $MCU_HOME_DIR/mcms/Bin/httpd
		ln -sf $MCU_HOME_DIR/usr/local/apache2/bin/httpd $MCU_HOME_DIR/mcms/Bin/httpd
	fi

	if [[ ! -e $MCU_HOME_DIR/mcms/EMA/htdocs ]]; then
               	#ln -sf ${LAST_BUILD}/ema/ $MCU_HOME_DIR/mcms/EMA/htdocs
		rm -f $MCU_HOME_DIR/mcms/EMA/htdocs
		ln -sf /opt/apache/htdocs $MCU_HOME_DIR/mcms/EMA/htdocs
	fi

        # Fix missing Self Signed file
        if [[ ! -e $MCU_HOME_DIR/mcms/Keys/cert_off.pem ]]; then
                echo "Fix missing Self Signed file"
                $MCU_HOME_DIR/mcms/Scripts/Self_Signed_Cert.sh boot
		if [ $? != 0 ];then
			echo "Failed to create self signed certificate"
			exit 1
		fi
        fi
	if [[ ! -e $MCU_HOME_DIR/tmp/privateKey.pem ]]; then
		ln -sf $MCU_HOME_DIR/mcms/Keys/private3.pem $MCU_HOME_DIR/tmp/privateKey.pem
	fi

        if [[ $MCMS_DIR == "$OFFICIAL_DIR/mcms/" ]]; then
                if [[ `ps -ef | grep ApacheModule` ]]; then
                        pkill ApacheModule
                        $MCU_HOME_DIR/mcms/Bin/ApacheModule &
                fi
        fi
	

	# Fix missing python link
	if [[ ! -e $MCU_HOME_DIR/mcms/python/bin/python ]]; then
		echo "Fix missing python link"
		ln -s /opt/polycom/python2.5_snmp/ $MCU_HOME_DIR/mcms/python
	fi
	if [ "$CLEAN_CFG" != "NO" ]
	then
		export CLEAN_CFG=YES
		echo "CONFIGURATION FILES WILL BE CLEANED"
	fi	

	sync;sync;sync;sleep 10
}


################################ CHECK KS VERSION ##################################
check_ks_ver(){

	#Latest_KS=`grep 'Version: ' /nethome/mrm/install/mrm-ks6.sh  | cut -d':' -f3 | cut -d'"' -f1 | tr -d ' '`
	Current_KS=`grep 'Version: ' /etc/ks_ver.txt | cut -d':' -f3 | tr -d ' ' | cut -d'.' -f4 | cut -d'_' -f1`
	Required_KS=`cat ks_ver.txt | cut -d'.' -f4`
	
	if (( ${Required_KS} == ${Current_KS} || ${Required_KS} < ${Current_KS} )); then
        	echo -n -e ${green}"KS OK "${normal}
	        echo "( $Current_KS )"
	else
	        echo  -e ${magenta}"KS Check Fail !!! "${normal}
	        echo "Your KS version ( $Current_KS ) is not updated to required version ( $Required_KS ) or above  "
	        echo "Are you sure you want to continue ? (y,n)"
	        read continue_ks_not_updated
	        if [[ "$continue_ks_not_updated" == "n" ]] ; then
	                exit
	        fi
	fi
}

################################## START ##################################
start ()
{
  	# Cleans log file.
	> $START_UP_LOG

  	if [ `whoami` == "root" ]
  	then
		log "You can't run this script as root"
    		exit 1
  	fi

  	# Gives 2 minutes to cool down in case of high load average.
  	# Exits if the load still exist.
  	export NUMBER_OF_CORES=`grep -c ^processor /proc/cpuinfo`
  	export LOAD_AVERAGE=`uptime | awk '{printf "%.0f\n",$(NF-2)}'`
  	export LOAD_THRESHOLD=1
  	if [[ "YES" != "${VM}" ]]; then
    		if [[ $((LOAD_AVERAGE/NUMBER_OF_CORES)) -ge $LOAD_THRESHOLD ]]
    		then
      			log "Load average $LOAD_AVERAGE / $NUMBER_OF_CORES >= $LOAD_THRESHOLD, wait for 2 minutes..."
 			sleep 2m

      			LOAD_AVERAGE=`uptime | awk '{printf "%.0f\n",$(NF-2)}'`
      			if [[ $((LOAD_AVERAGE/NUMBER_OF_CORES)) -ge $LOAD_THRESHOLD ]]
      			then
        			log "Load average $LOAD_AVERAGE / $NUMBER_OF_CORES >= $LOAD_THRESHOLD, exit."
        			exit 1
      			fi
    		fi
  	fi
	
	#Launch the periodic MCU status logger
        #echo "Launching Soft MCU Status Logger" | $LOG
	if [[ "YES" == "${VM}" ]]; then
        	#$MCU_HOME_DIR/mcms/Scripts/status_logger.sh soft.sh $MCU_HOME_DIR/tmp/startup_logs/MCU_Process_Status.log 15  60 &
            SIM_PACK_SA=/opt/polycom/sim_pack/mcms_100.0_v011/SoftMCU
            sudo /opt/polycom/ks-correction.sh FIX_SALIB $SIM_PACK_SA
	fi
	MFA_WITH_VALGRIND="NO"
	if [[ "$1" != "" && "$1" == "mfa" ]]
	then
		MFA_WITH_VALGRIND="YES"	
	fi
				
	export LD_LIBRARY_PATH=$MCMS_DIR/Bin:$CS_DIR/lib:$MCU_LIBS
	export ENDPOINTSSIM=NO
	export GIDEONSIM=NO

	if [ "$CLEAN_CFG" != "YES" ]
	then
		log "CONFIGURATION FILES WILL NOT BE CLEANED"
		export CLEAN_CFG=NO
	fi

	export CLEAN_LOG_FILES=NO
	export CLEAN_AUDIT_FILES=NO
 	export CLEAN_FAULTS=NO
 	export CLEAN_CDR=NO
 	export CLEAN_STATES=NO
 	export CLEAN_MEDIA_RECORDING=NO
	export PERSISTENT_CACHE="YES"
	#to keep local tracer logs in startup turn on this export below
	#export CLEAN_LOCAL_TRACER_LOGS=NO         
	rm -f $MCU_HOME_DIR/tmp/httpd.pid

  	cd $MCMS_DIR
	log "Clean before start."
  	Scripts/Cleanup.sh 2>&1

	PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`
  	if [ -z $PRODUCT_TYPE ]; then
    		PRODUCT_TYPE="SOFT_MCU"
  	fi

  	log "Start $PRODUCT_TYPE."
  	export SOFT_MCU_FAMILY=YES

	if [[ $PRODUCT_TYPE == "SOFT_MCU_MFW" ]]; then
    		export SOFT_MCU_MFW=YES
    		mkdir -p $MCU_HOME_DIR/mcms/EMA
  	elif [[ $PRODUCT_TYPE == "SOFT_MCU_EDGE" ]]; then
    		export SOFT_MCU_EDGE=YES
  	elif [[ "$PRODUCT_TYPE" == "GESHER" ]]; then
    		export GESHER=YES

    		log "Gesher Bringing up eth0 first..."
    		Scripts/GesherUpEths.sh
    		log "Run Gesher McmsStart.sh ..."
    		Scripts/GesherMcmsStart.sh &
  	elif [[ "$PRODUCT_TYPE" == "NINJA" ]]; then
    		export NINJA=YES

    		log "Ninja Bringing up eth0 first..."
    		Scripts/GesherUpEths.sh
    		log "Run Ninha McmsStart.sh ..."
    		Scripts/GesherMcmsStart.sh &
	elif [[ "$PRODUCT_TYPE" == "SOFT_MCU_CG" ]]; then
		export SOFT_MCU_CG=YES
  	else 
    		export SOFT_MCU=YES
  	fi
	
	echo -n $PRODUCT_TYPE > $MCU_HOME_DIR/tmp/EMAProductType.txt
	echo -n NO > $MCU_HOME_DIR/tmp/JITC_MODE.txt

	export MPL_SIM_FILE=VersionCfg/MPL_SIM_SWITCH_ONLY.XML

        if [[ -z "$SOFT_MCU" ]]
        then
            make active
        fi
      
	log "Start EngineMRM."
	cd $ENGINE_DIR
	./mrm.sh all 2>&1 >$ENGINE_OUT &
	cd - 

  	log "Start MCMS."

	if [[ "$1" != "" ]]
	then	
		echo "#######################################" | $LOG
		echo "   Running $1 under Valgrind           " | $LOG
		echo "#######################################" | $LOG
		if [ ! -d TestResults ]; then
		   mkdir TestResults
		fi
		chmod 777 TestResults		
		Scripts/Startup.sh $1 $2  > $MCMS_OUT 2>&1 &
	else
		echo "start no valgrind"
		Scripts/Startup.sh  > $MCMS_OUT 2>&1 &
	fi	
		
	sleep 25

	if [[ $PRODUCT_TYPE == "SOFT_MCU_MFW" || $PRODUCT_TYPE == "SOFT_MCU_EDGE" || $PRODUCT_TYPE == "SOFT_MCU_CG" ]]; then
		if [ -e $MCU_HOME_DIR/tmp/stop_monitor ]
		then
			log "Stop monitor for debug."
		else
		# Makes sure processes are running
		log "Watcher is alive."
		Process_Watcher=$(ps -ef | grep SoftMcuWatcher.sh | grep -v grep)
		if [ "" == "${Process_Watcher}" ]
		then
			log "Start SoftMcuWatcher."
       			nohup $MCU_HOME_DIR/mcms/Scripts/SoftMcuWatcher.sh > /dev/null 2>&1 &
		fi
	fi
	fi
	
	if [ "YES" == "${VM}" ]
	then
		 sudo /bin/chmod -R a+w $MCU_HOME_DIR/etc/rmx1000/sysconfig
		 sed -i '/SkipImage.*/d' $MCU_HOME_DIR/etc/rmx1000/sysconfig/imgs_to_launch.conf
	cd $MPMX_DIR
	(echo "# Generated by soft.sh"
	echo export SIMULATION=YES
	echo export RUN_MCMS=NO
	echo export GDB=NO
	echo export VALGRIND=$MFA_WITH_VALGRIND
	echo export DMALLOC_SIM=NO
	echo export EFENCE_SIM=NO
	echo export TRACE_IPMC_PROTOCOL=NO
	echo export PLATFORM=RMX2000) > ./mfa_x86_env/cfg/runmfa.export
	log "Start MPMX."
	(	
		export LD_LIBRARY_PATH=$MPMX_DIR/mfa_x86_env/bin:$MCU_LIBS
		ulimit -n 4096
		./scripts_x86/runmfa.sh 2>&1 >$MPMX_OUT &
	)&
	else
	# to make sure processes are running
	Process_MpWatcher=$(ps -ef | grep MpWatcher.sh | grep -v grep)
	if [ "" == "${Process_MpWatcher}" ]
	then 
		$MCU_HOME_DIR/mcms/Scripts/MpWatcher.sh &
	fi
	fi
	# run rmx1000 parts
  log "Start RMX1000."
	( 
		export LD_LIBRARY_PATH=$MCU_HOME_DIR/usr/rmx1000/bin:$MRMX_DIR/libs/lib:$MRMX_DIR/libs/usr/lib:$MCU_LIBS
		ulimit -n 4096 
		nohup $MCU_HOME_DIR/usr/rmx1000/bin/launcher 2>&1 >$MRMX_OUT
	) &

	cp $MCU_HOME_DIR/mcms/Scripts/TLS/passphrase.sh     $MCU_HOME_DIR/tmp
	cp $MCU_HOME_DIR/mcms/Scripts/TLS/passphraseplcm.sh $MCU_HOME_DIR/tmp
	chown mcms:mcms $MCU_HOME_DIR/tmp/passphrase.sh
	chown mcms:mcms $MCU_HOME_DIR/tmp/passphraseplcm.sh

	# Making sure Single Apache is monitored
	sleep 5
	echo "**************************"
	echo "* STARTING SINGLE APACHE *"
	echo "**************************"
	# Update ApacheModule in MFW to use sudo only if required (port > 1023)
	#if [[ "$PRODUCT_TYPE" == "SOFT_MCU_MFW" ]]; then
	#       IPnPort=`grep '^Listen' $MCU_HOME_DIR/tmp/httpd.listen.conf | grep -v '\[.*\]' | cut -d' ' -f2`
	#	OnlyPort=`echo -n $IPnPort | cut -d':' -f2`
	#	if (( $OnlyPort < 1024 )); then
	#		sed -i -e 's/sudo //' $MCU_HOME_DIR/mcms/Bin/ApacheModule
	#	else
	#		if [[ ! `grep CMD $MCU_HOME_DIR/mcms/Bin/ApacheModule | grep sudo` ]]; then
	#			sed -i -e 's/env/sudo env/' $MCU_HOME_DIR/mcms/Bin/ApacheModule
	#		fi
	#	fi
	#fi
	# Force Single Apache binary to be used"
	if [[ `whoami` == 'root' || `whoami` == 'mcms' ]]; then
		rm -f $MCU_HOME_DIR/mcms/Bin/httpd
		ln -sf $MCU_HOME_DIR/usr/local/apache2/bin/httpd $MCU_HOME_DIR/mcms/Bin/
	fi

	if [[ "YES" == "${VM}" ]]; then
		log "Running ApacheModule fix on VM."
		$MCU_HOME_DIR/mcms/Scripts/ApacheModule.SoftMcuSim 2> /dev/null &
	fi

	sleep 5
	
	if [[ "YES" == "${VM}" ]]; then
		if [[ ! -d $MCU_HOME_DIR/mcms/LogFiles ]]; then
			rm -f $MCU_HOME_DIR/mcms/LogFiles
			mkdir $MCU_HOME_DIR/mcms/LogFiles
		fi
	fi

	cd $CS_DIR
	log "Start CS."
	mkdir -p $MCU_HOME_DIR/config/ocs
	chmod a+w $MCU_HOME_DIR/config/ocs
		
	#incase of upgrade change file to new name
	for i in 1 2 3 4 5 6 7 8
	
	do
	  ## create the directory if does not exist
	  mkdir -p $MCU_HOME_DIR/config/ocs/cs$i/keys

	  filename=$MCU_HOME_DIR/cs/ocs/cs$i/keys/certPassword.txt
	  if [[ "YES" == "${VM}" ]]; then
	  	filename=$MCU_HOME_DIR/cs/ocs/cs$i/certPassword.txt
	  fi
	  if [ -f $filename ]
	  then  
	    newFileName=$MCU_HOME_DIR/cs/ocs/cs$i/keys/certPassword.txt.orig
	    if [[ "YES" == "${VM}" ]]; then
	  		newFileName=$MCU_HOME_DIR/cs/ocs/cs$i/certPassword.txt.orig
	  	fi	
	    mv $filename $newFileName
	  fi
	  cs_destination=$MCU_HOME_DIR/cs/ocs/cs$i/keys
  	  cs_source=$MCU_HOME_DIR/config/keys/cs/cs$i/*
  	  if [[ "YES" == "${VM}" ]]; then
	  	 cs_source=$MCU_HOME_DIR/mcms/KeysForCS/cs$i/*
	  	 cs_destination=$MCU_HOME_DIR/cs/ocs/cs$i
	  fi
  
  	  cp -Rf $cs_source $cs_destination
  	  
	done
	cp -f $MCU_HOME_DIR/mcms/Versions.xml $MCU_HOME_DIR/tmp

	CS_NUM_OF_CALLS=-N400
	CS_PLATFORM_TYPE=-P6
	
	if [ "$PRODUCT_TYPE" == "SOFT_MCU_MFW" ]; then
		CS_NUM_OF_CALLS=-N2000	
		CS_PLATFORM_TYPE=-P7
	fi
	
	CS_XML_MODE=`grep -A 1 "<KEY>CS_API_XML_MODE</KEY>" $MCU_HOME_DIR/mcms/Cfg/SystemCfgUserTmp.xml | grep -c "<DATA>YES</DATA>"`
	if [ "$CS_XML_MODE" == "1" ]
	then
		CS_XML_FLAG=-X1
	else
		CS_XML_FLAG=-X0
	fi
	
	if [ "$VM" == "YES" ]
	then
		./bin/acloader -c -C$CS_DIR/cfg/cfg_soft/cs_private_cfg_dev.xml $CS_PLATFORM_TYPE -S1 $CS_NUM_OF_CALLS $CS_XML_FLAG 2>&1 >$CS_OUT
	else
		./bin/acloader -c -C$CS_DIR/cfg/cfg_soft/cs_private_cfg_rel.xml $CS_PLATFORM_TYPE -S1 $CS_NUM_OF_CALLS $CS_XML_FLAG 2>&1 >$CS_OUT
	fi

	exit
}

create_custom_cfg_file ()
{
	CUSTOM_CFG_FILE=/mcu_custom_config/custom.cfg
	PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`
	echo "<SYSTEM_CFG>" >> $CUSTOM_CFG_FILE
	echo "	<CFG_SECTION>" >> $CUSTOM_CFG_FILE
	echo "	    <NAME>CUSTOM_CONFIG_PARAMETERS</NAME>" >> $CUSTOM_CFG_FILE
	echo "	     <CFG_PAIR>" >> $CUSTOM_CFG_FILE
	echo "	     	<KEY>XML_API_PORT</KEY>" >> $CUSTOM_CFG_FILE
	echo "	        <DATA>80</DATA>" >> $CUSTOM_CFG_FILE
	echo "	     </CFG_PAIR>" >> $CUSTOM_CFG_FILE
	echo "	     <CFG_PAIR>" >> $CUSTOM_CFG_FILE
	echo "	     	<KEY>XML_API_HTTPS_PORT</KEY>" >> $CUSTOM_CFG_FILE
	echo "	        <DATA>443</DATA>" >> $CUSTOM_CFG_FILE
	echo "	     </CFG_PAIR>" >> $CUSTOM_CFG_FILE
	echo "	     <CFG_PAIR>" >> $CUSTOM_CFG_FILE
	echo "	     	<KEY>STUN_SERVER_PORT</KEY>" >> $CUSTOM_CFG_FILE
	echo "	        <DATA>3478</DATA>" >> $CUSTOM_CFG_FILE
	echo "	     </CFG_PAIR>" >> $CUSTOM_CFG_FILE
	echo "	     <CFG_PAIR>" >> $CUSTOM_CFG_FILE
	echo "	     	<KEY>TURN_SERVER_PORT</KEY>" >> $CUSTOM_CFG_FILE
	echo "	        <DATA>3478</DATA>" >> $CUSTOM_CFG_FILE
	echo "	     </CFG_PAIR>" >> $CUSTOM_CFG_FILE
	echo "	     <CFG_PAIR>" >> $CUSTOM_CFG_FILE
	echo "	     	<KEY>CUSTOM_USER_LOGIN</KEY>" >> $CUSTOM_CFG_FILE
	echo "	        <DATA>POLYCOM</DATA>" >> $CUSTOM_CFG_FILE
	echo "	     </CFG_PAIR>" >> $CUSTOM_CFG_FILE
	echo "	     <CFG_PAIR>" >> $CUSTOM_CFG_FILE
	echo "	     	<KEY>CUSTOM_USER_PASSWD</KEY>" >> $CUSTOM_CFG_FILE
	echo "	        <DATA>POLYCOM</DATA>" >> $CUSTOM_CFG_FILE
	echo "	     </CFG_PAIR>" >> $CUSTOM_CFG_FILE			
	echo "	     <CFG_PAIR>" >> $CUSTOM_CFG_FILE
	echo "	     	<KEY>FORCE_LOW_MEMORY_USAGE</KEY>" >> $CUSTOM_CFG_FILE
	echo "	        <DATA>NO</DATA>" >> $CUSTOM_CFG_FILE
	echo "	     </CFG_PAIR>" >> $CUSTOM_CFG_FILE				
	echo "	     <CFG_PAIR>" >> $CUSTOM_CFG_FILE
	echo "	     	<KEY>DEFAULT_NETWORK_INTERFACE</KEY>" >> $CUSTOM_CFG_FILE
	echo "	     	<DATA>eth0</DATA>" >> $CUSTOM_CFG_FILE
	echo "	     </CFG_PAIR>" >> $CUSTOM_CFG_FILE 	
	echo "	</CFG_SECTION>" >> $CUSTOM_CFG_FILE
	echo "</SYSTEM_CFG>" >> $CUSTOM_CFG_FILE

	chmod 777 $CUSTOM_CFG_FILE

}

create_custom_cfg_path ()
{
	if [ ! -d /mcu_custom_config ];then
		mkdir /mcu_custom_config
		chmod 777 /mcu_custom_config
		create_custom_cfg_file
	else
		CUSTOM_CFG_FILE=/mcu_custom_config/custom.cfg
		if [ -f $CUSTOM_CFG_FILE ];then
			chmod 777 $CUSTOM_CFG_FILE
		else			
			create_custom_cfg_file
		fi
	
	fi



}


################################## TARGET ##################################
target () {
	
	log "Target log."
	
	restore_factory_defaults

	rm -fR $MCU_HOME_DIR/tmp/queue
	rm -fR $MCU_HOME_DIR/tmp/shared_memory
	rm -fR $MCU_HOME_DIR/tmp/semaphore
	rm -fR $MCU_HOME_DIR/tmp/802_1xCtrl
	rm -f $MCU_HOME_DIR/tmp/loglog.txt
	rm -f $MCU_HOME_DIR/tmp/httpd.pid
	rm -f $MCU_HOME_DIR/tmp/httpd.listen.conf
	#rm -f $MCU_HOME_DIR/tmp/httpd.rest.conf

	create_custom_cfg_path
	auto_detect_compilation_type
	touch $MCU_HOME_DIR/tmp/httpd.listen.conf
	#touch $MCU_HOME_DIR/tmp/httpd.rest.conf
	chmod 777 $MCU_HOME_DIR/tmp/httpd.listen.conf
	#chmod 777 $MCU_HOME_DIR/tmp/httpd.rest.conf

	su - mcms -c "cd $MCMS_DIR ; . Scripts/SoftMcuExports.sh ; ./Scripts/soft.sh start $1 &"
	PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`
        #if [[ $PRODUCT_TYPE != "SOFT_MCU_EDGE" ]];then      
	#       if [[ `ps -ef | grep $MCU_HOME_DIR/usr/local/apache2/bin/httpd | grep -v root` == "" ]];then
	#		service httpd start
	#	fi    
	#fi
	
	verify_system_is_up

	$MCU_HOME_DIR/usr/rmx1000/bin/SetAudioSoftPriority.sh
}

restore_factory_defaults() {
if test -e $MCU_HOME_DIR/mcms/States/restore_factory_defaults.flg; then
	echo "Restore factory defaults" | tee -a $STARTUP_LOG;

	echo "NO" > $MCU_HOME_DIR/mcms/JITC_MODE.txt
	echo "NO" > $MCU_HOME_DIR/mcms/JITC_MODE_FIRST_RUN.txt
	
  # Removes some of Cfg files
	mkdir -p $MCU_HOME_DIR/config/lost+found
	mv $MCU_HOME_DIR/mcms/Cfg/NetworkCfg_Management.xml $MCU_HOME_DIR/config/lost+found/

	rm -Rf $MCU_HOME_DIR/mcms/Cfg/*
	
	mv $MCU_HOME_DIR/config/lost+found/NetworkCfg_Management.xml $MCU_HOME_DIR/mcms/Cfg/

	rm -Rf $MCU_HOME_DIR/config/states/*
	/bin/touch $MCU_HOME_DIR/config/states/McmsRestoreFactoryFileInd.flg

	rm -Rf $MCU_HOME_DIR/mcms/Links/*
	rm -Rf $MCU_HOME_DIR/config/ema/*
	rm -Rf $MCU_HOME_DIR/config/lost+found/*

	rm -Rf cs
    
	rm -Rf $MCU_HOME_DIR/mcms/StaticStates/MepMode.txt
    
	# removing all folders from HD
	cd $MCU_HOME_DIR/output
	rm -Rf $OUTPUT_FOLDERS
	mkdir -p $OUTPUT_FOLDERS
	chown mcms:mcms $OUTPUT_FOLDERS
	chmod a+w $OUTPUT_FOLDERS
	cd -
fi

cd $MCU_HOME_DIR/mcms
if test -e $MCU_HOME_DIR/config/states/restore_config.flg; then
Scripts/RestoreConfig.sh $MCU_HOME_DIR/config/states  1 > /dev/null 2>&1;
fi
}

##################################Auto detect compilation type#############################

auto_detect_compilation_type () {

	rm $MCU_HOME_DIR/usr/share/EngineMRM/Bin
	FORCE_LOW_MEMORY_USAGE=`echo "cat /SYSTEM_CFG/CFG_SECTION/CFG_PAIR[KEY='FORCE_LOW_MEMORY_USAGE']/DATA/text()" | xmllint --shell /mcu_custom_config/custom.cfg  | egrep '^\w'`
	if [[ "$FORCE_LOW_MEMORY_USAGE" == "YES" ]];then
		ln -sf $MCU_HOME_DIR/usr/share/EngineMRM/Bin-regular/ $MCU_HOME_DIR/usr/share/EngineMRM/Bin
		log "FORCE_LOW_MEMORY_USAGE - low memory compilation"
	else
		MEMORY_AMOUNT=`cat /proc/meminfo | grep MemTotal | awk '{ print $2 }'`
		if [[ $MEMORY_AMOUNT -le 700000 ]];then		
		        log "Auto detect memory low than 8g - stop startup!"
			echo "The system does not meet the minimum hardware requirements." > $SERVICE_OUTPUT_LOG
			stop
			exit 1
		fi

		PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`
		if [[ $MEMORY_AMOUNT -ge 16000000 && $PRODUCT_TYPE == "SOFT_MCU_MFW" ]];then
			ln -sf $MCU_HOME_DIR/usr/share/EngineMRM/Bin-high/ $MCU_HOME_DIR/usr/share/EngineMRM/Bin
		        log "Auto detect high memory compilation."
		else
			ln -sf $MCU_HOME_DIR/usr/share/EngineMRM/Bin-regular/ $MCU_HOME_DIR/usr/share/EngineMRM/Bin
		        log "Auto detect low memory compilation."
		fi	
	
	fi
	
	chown -R mcms:mcms $MCU_HOME_DIR/usr/share/EngineMRM/Bin

}

################################## VERIFY SYSTEM IS UP ##################################
verify_system_is_up () {

	# Allow 'start' to begin
	sleep 30
# wait for 'audio_soft' to run and renice to -10
	AUDIO_SOFT_TIMOUT=60
	HTTPD_SOFT_TIMOUT=180
	time_out_counter=0
	while [[ $(ps -A | grep "audio_soft") == "" && $time_out_counter -le $AUDIO_SOFT_TIMOUT ]]
	do
		sleep 1
		((time_out_counter++))
	done

	if [[ $time_out_counter -ge $AUDIO_SOFT_TIMOUT ]]
	then
		log "AUDIO_SOFT_TIMOUT expired."
		stop
		exit 1			
	fi
	
	time_out_counter=0
	
	while [[ `ps -ef | grep $MCU_HOME_DIR/mcms/Bin/httpd | grep -v root` == "" && $time_out_counter -le $HTTPD_SOFT_TIMOUT ]]
   	do
       		sleep 1
       		((time_out_counter++))
   	done  

	if [[ $time_out_counter -ge $HTTPD_SOFT_TIMOUT ]]
	then
		log "HTTPD_SOFT_TIMOUT expired."
		stop
		exit 1			
	fi     
}

################################## STOP ##################################
stop ()
{
  PRODUCT_TYPE=`cat $MCU_HOME_DIR/mcms/ProductType`
  if [ -z $PRODUCT_TYPE ]; then
    PRODUCT_TYPE="SOFT_MCU" 
  fi

  log "Stop $PRODUCT_TYPE."

  if [[ $PRODUCT_TYPE == "SOFT_MCU_MFW" ]]; then
    export SOFT_MCU_MFW=YES 
  elif [[ $PRODUCT_TYPE == "SOFT_MCU_EDGE" ]]; then
    export SOFT_MCU_EDGE=YES 
  elif [[ "$PRODUCT_TYPE" == "GESHER" ]]; then
    export GESHER=YES
  elif [[ "$PRODUCT_TYPE" == "NINJA" ]]; then
    export NINJA=YES
  elif [[ "$PRODUCT_TYPE" == "SOFT_MCU_CG" ]]; then
    export SOFT_MCU_CG=YES
  else 
    export SOFT_MCU=YES 
  fi

  # Only if in vm simulation, stop Watcher process.
  if [[ $USER != "mcms" ]]; then
		killall -9 SoftMcuWatcher.sh 2> /dev/null
  fi
	
  export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin:$MCU_LIBS
  cd $MCMS_DIR
  log "Flush Logger."
  pgrep -x Logger && Bin/McuCmd flush Logger < /dev/null && sleep 2

  # MCMS
  $MCMS_DIR/Scripts/Destroy.sh 2>&1 >$MCMS_OUT
  killall Startup.sh WaitForStartup.sh 2>/dev/null 
  killall -9 MpWatcher.sh 2>/dev/null

  # Kills launcher first to prevent restart of some process.
  pgrep -x launcher && killall -9 launcher 2>/dev/null && sleep 1

  # Generously asks to die and gives some time to free resources.
  killall -2 MRM-MrmMain audio_soft video 2>/dev/null
  killall acloader mfa mp_launcher LinuxSysCallProcess traced mpproxy 2>/dev/null
  kill -9 $(ps -ea | grep ice_manager | awk '{print $1}') 2>/dev/null
  pkill -f status_logger.sh 2>/dev/null

  # Brutal termination.
  PROCESSES="\
            MRM-MrmMain \
            acloader \
            tarAndZip \
            calltask \
            csman \
            gkiftask \
            h323LoadBalancer \
            mcmsif \
            siptask \
            mcms \
            mfa \
            launcher \
            sys \
            mp \
            audio_soft \
            traced \
            Proxy \
            sys_status_monitor \
            ManageMcuMenu \
            menu \
            mpproxy \
            video \
            Startup.sh \
            WaitForStartup.sh \
            "

  #wait for maximum of 25 secs for all processes to terminate
  set -- `echo "$PROCESSES"`
  i=0
  retry=25
  while [ $# -ne 0 -a $i -lt $retry ]
  do
      proc=$1
      sproc=${proc:0:15}
      pgrep -x $sproc >/dev/null
      if [ $? -eq 0 ]; then
          sleep 1.0
          i=`expr $i + 1`

          if [ "YES" == "${VM}" -a $i -gt 5 ]; then
             echo -e "\n#####\nProcess $sproc takes more than $i secs to exit\n#####\n"
          fi
      else
          shift
      fi
  done

  # Limits process names to the 15 characters.
  for p in $PROCESSES
  do
    pgrep -x ${p:0:15} && log "$p is still alive, finish him." && killall -9 -v $p 2>/dev/null 
    killall -9 $p 2>/dev/null
  done

  wait

  for p in $PROCESSES
  do
    killall -9 $p 2>/dev/null
    pgrep -x ${p:0:15} && log "Failed to kill $p."
  done

    killall -9 launcher sys mp traced Proxy sys_status_moni ManageMcuMenu menu httpd mpproxy video sys_status_monitor \
               ASS-AMPMgr ASS-AMPUdpRx ASS-AH ASS-AMPTx ASS-AMPLog AMP-AmpAHMgr AMP-AmpAHDec0 AMP-AmpAHEnc0 AMP-AmpAHIvr0 mcmsif 2>/dev/null

  log "Stop Apache."
    pkill ApacheModule
    if [[ `whoami` == "root" ]]; then
	    ( pkill httpd && sleep 1; pkill -9 httpd && sleep 1 ) 2> /dev/null
    else
	    ( sudo pkill httpd && sleep 1; sudo pkill httpd && sleep 1 ) 2> /dev/null
    fi
  # Cleans files and shared memory (once) for MCMS.
  export -n SOFT_MCU_MFW
  export -n SOFT_MCU_EDGE
  export -n GESHER
  export -n NINJA
  export -n SOFT_MCU
  export -n SOFT_MCU_CG
  #to keep local tracer logs in startup turn on this export below
  #export CLEAN_LOCAL_TRACER_LOGS=NO   

  cd $MCMS_DIR

  kill -9 `pgrep Cards` 2> /dev/null
  kill -9 `pgrep MCCFMngr` 2> /dev/null

  Scripts/Cleanup.sh
}

################################## TEST ##################################
DTD_list (){
  echo $COLORBROWN
  grep "#--DTD_" ./Scripts/DeliveryTestsDefs.sh | cut -f2 -d"_" | sort
  echo $TXTRESET
}

# make_test  ====================================
make_test () {
	cd $MCMS_DIR  

	# Set global variables for colors
	. ./Scripts/ClrSetting.sh
	# Load set of functions of DeliveryTests
	. ./Scripts/DeliveryTestsDefs.sh 
	
	export LD_LIBRARY_PATH=$MCU_HOME_DIR/usr/local/apache2/lib:$MCU_HOME_DIR/mcms/Bin:$MCU_LIBS
        ulimit -c unlimited

	make active
	setup_env

	ln -sf $RMX1000_RPM_FILE $MCU_HOME_DIR/tmp/rmx1000.rpm
	. $MCMS_DIR/Scripts/InstallRmx1000rpm.sh

	param=${1-NONE}

        if [ $param != "NONE" ]
	then
	  if [ $param != "list" ]
          then
	  	echo "$COLORMGNTA********** Running $param tests ***************$TXTRESET"
		CleanCoreDumpsFiles
		#CheckCoreDumpsExisting
	  fi

	  DTD_$param Exit_From_MakeTest true
          
        else 
	  CleanCoreDumpsFiles

	  echo $COLORBROWN
	  echo "*****************************************"
	  echo "********** START DELIVERY TESTS *********"
	  echo "*****************************************"
	  echo $TXTRESET

	  echo "$COLORMGNTA********** Running MCMS Unit tests ***************$TXTRESET"
	  DTD_MCMSUnitTests Exit_From_MakeTest false

	  echo "$COLORMGNTA********** Running EngineMRM Unit tests **********$TXTRESET"
	  DTD_EngineMRMUnitTests Exit_From_MakeTest false
          Exit_From_MakeTest 0
	fi
}

# NTD_list  =====================================
NTD_list (){
  echo $COLORBROWN
  grep "#--NTD_" ./Scripts/NightTestsDefs.sh | cut -f2 -d"_" | sort
  echo $TXTRESET
}

# night() ========================================
night () {
	if [ `whoami` == "root" ]
	then
		echo "You can't run this script as root!!!"
		exit
	fi

	# Set global variables for colors
  	. ./Scripts/ClrSetting.sh
  	# Load set of functions of NightTests
  	. ./Scripts/NightTestsDefs.sh 

	Init_NightTest

	param=${1-NONE}
	if [ $param != "NONE" ]
	then
	  if [ $param != "list" ]
          then
	  	echo $COLORMGNTA
		echo "NightTest($param) Start: `date` ; `hostname` ; `whoami` "
		echo $TXTRESET
		make active
		CleanAllRelevantDirs
		#CreateRelevantIssuesForNightTests
	  fi

	  NTD_$param Return_From_NightTest true  
          
        else
	  make active
          CleanAllRelevantDirs
	  CreateRelevantIssuesForNightTests

	  # To delete all directories older then 30 days
	  # This call must be located under 'CreateRelevantIssuesForNightTests()'
	  find $RootDirectory/* -type d -mtime +30 -exec rm -fR '{}' \; 2>/dev/null
	  #find $RootDirectory/* -type d -mtime +1 -exec rm -fR '{}' \; 2>/dev/null

	  echo $COLORBROWN
	  echo "************************************************************"
	  echo "NightTestStart: `date` ; `hostname` ; `whoami` "
	  echo "************************************************************"
	  echo $TXTRESET

	  echo "$COLORMGNTA********** Running SoftMCU system night tests **********$TXTRESET"
	  RunOneSetOfTests NTD_SoftMCUSystemTests "SystemTests" "SoftMCU tests"

	  echo "$COLORMGNTA********** Running EngineMRM night tests **********$TXTRESET"
	  RunOneSetOfTests NTD_EngineMRMNightTests "EngineMRM" "EngineMRM tests"

	  echo "$COLORMGNTA********** Running MCMS python night tests **********$TXTRESET"
	  StartMCMSPythonTests

	  #old way to run python tests
	  #make all_test_scripts

	  #The function runs 'Scripts/AutoRealVoip.sh'
	  RunOneSetOfTests NTD_MCMSAutoRealVoip "AutoRealVoip" "MCMS tests"

	  #The function runs 'Scripts/AutoRealVideo.sh'
	  RunOneSetOfTests NTD_MCMSAutoRealVideo "AutoRealVideo" "MCMS tests" 

	  #The function runs 'Scripts/Add20ConferenceNew.sh'
	  #RunOneSetOfTests NTD_MCMSAdd20ConferenceNew "Add20ConferenceNew" "MCMS tests"

	  #The function runs 'Scripts/AddDeleteIpServ.sh'
	  #RunOneSetOfTests NTD_MCMSAddDeleteIpServ "AddDeleteIpServ" "MCMS tests"

	  #The function runs 'Scripts/AddDeleteNewIvr.sh'
	  RunOneSetOfTests NTD_MCMSAddDeleteNewIvr "AddDeleteNewIvr" "MCMS tests"

	  #The function runs 'Scripts/AddIpServiceWithGkNew.sh'
	  #RunOneSetOfTests NTD_MCMSAddIpServiceWithGkNew "AddIpServiceWithGkNew" "MCMS tests"

	  #The function runs 'Scripts/AddIpServiceWithGk.sh'
	  #RunOneSetOfTests NTD_MCMSAddIpServiceWithGk "AddIpServiceWithGk" "MCMS tests"

	  #The function runs 'Scripts/AddRemoveMrNew.sh'
	  RunOneSetOfTests NTD_MCMSAddRemoveMrNew "AddRemoveMrNew" "MCMS tests"

	  #The function runs 'Scripts/AddRemoveOperator.sh'
	  RunOneSetOfTests NTD_MCMSAddRemoveOperator "AddRemoveOperator" "MCMS tests"

	  #The function runs 'Scripts/AddRemoveProfile.sh'
	  RunOneSetOfTests NTD_MCMSAddRemoveProfile "AddRemoveProfile" "MCMS tests"

	  EndMCMSPythonTests

	  #The function runs Scripts/run_night_test.sh
	  RunOneSetOfTests NTD_RMXPartNightTests "RMXNightTests" "RMX tests"


	  PrintHTMLEnd 
	  ./Scripts/MoveLastTblUP.sh $NightReportF "<table border=" "</table>"
          
	  SendMail $NightReportF

	  UnsetOfAllRestedExportVars
	fi

	exit 0
}

################################## LIST ######################################
McmsUnitTestList() {
	cd $MCMS_DIR
	for Name in ./Bin/*.Test ; 
	do
		echo "+-+-+- List of tests in '$Name' +-+-+-+"
		$Name list 2>&1 | grep "::"
	done
}

##############################################################################
trace_start_build (){
	echo -e ${BLUE}
	echo "#######################################"
	echo "#  Project: $1  #"
	echo "#  STARTING COMPILATION		    #"
	echo "#######################################"
	echo -e ${NO_COLOR}
}

##############################################################################
compile_result (){
	if [ $1 == 0 ];then
		echo -e ${GREEN}
		echo "#######################################"
		echo "#  Project: $2  #"
		echo "#  COMPILATION FINISHED SUCCESSFULLY  #"
		echo "#######################################"
		echo -e ${NO_COLOR}
	else
		echo -e ${RED}
		echo "##########################"
		echo "#  Project: $2  #"
		echo "#   COMPILATION FAILED   #"
		echo "##########################"
		echo -e ${NO_COLOR}
		exit 1
	fi
}


#################################################################
#								#
#			MAIN					#
#								#
#################################################################


verify_variables
if [ $? != 0 ];then
	exit 1
fi

# Defines appropriate output streams for terminal messages.
if [ ${#DIR_LIST[@]} != ${#OUT_LIST[@]} ]; then
  echo "Lengths of DIR_LIST and OUT_LIST shoud be the equal:\
 ${#DIR_LIST[@]} != ${#OUT_LIST[@]}"
  exit 1
fi

msg="Output streams configuration:"
for i in ${!DIR_LIST[@]}
do
  dir="${DIR_LIST[$i]}"
  out="${OUT_LIST[$i]}"

  # Defines output if it is not defined.
  if [ -z ${!out} ]; then
    # Sets standard output only for private directory.
    if [[ ${!dir} == *"$WHO_AM_I"* ]]; then
      export $out="/dev/stdout"
    else
      export $out="/dev/null"
    fi
  fi

  msg+="\n\t$out=${!out}"
done

log "$msg"

echo -e "\nRunning soft.sh $1 $2:\n=================================="
        
case "$1" in
make)
	make_all_projects $2
	;;
make_clean)	
	make_clean
	;;
clean)
	clean
	;;
make_rpm)
	make_rpm
	;;
private_build)
	private_build
	;;
install_build)
	install_build $2
	;;
start_vm)
	check_ks_ver
	setup_vm_all_products "SOFT_MCU"
	echo -n SOFT_MCU > $MCU_HOME_DIR/mcms/ProductType
	setup_sim_license_file
	start $2 $3
	;;
start_vm_cg)
	check_ks_ver
	setup_vm_all_products "SOFT_MCU_CG"
	echo -n SOFT_MCU_CG > $MCU_HOME_DIR/mcms/ProductType
	setup_sim_license_file
	/opt/polycom/scalesuite_video_mount
	start $2 $3
	;;
start_vm_mfw)
	check_ks_ver
	setup_vm_all_products "SOFT_MCU_MFW"
	echo -n SOFT_MCU_MFW > $MCU_HOME_DIR/mcms/ProductType
	setup_sim_license_file
	start $2 $3
	;;
start_vm_edge)
	check_ks_ver
	setup_vm_all_products "SOFT_MCU_EDGE"
	echo -n SOFT_MCU_EDGE > $MCU_HOME_DIR/mcms/ProductType
	setup_sim_license_file
	start $2 $3
	;;
start_vm_gesher)
	check_ks_ver
	setup_vm_all_products "GESHER"
    	echo -n GESHER > $MCU_HOME_DIR/mcms/ProductType
	start $2 $3
	;;
start)
	start $2 $3
	;;
target)
	target $2
	;;
stop)
	stop	
	;;
test)
	make_test $2
	;;
night)
	night $2
	;;

mcmsUnitTestList)
	McmsUnitTestList
	;;
*)
	echo "$1: Action not supported."
	;;
esac
