#!/bin/sh

trap "echo Aborting the compilation...; rm -rf \$DISTCC_DIR; exit 1;" SIGHUP SIGINT SIGTERM

# DISTCC_DIR will contain distcc run time date
#
export DISTCC_VERBOSE=0
export DISTCC_DIR=/dev/shm/distcc.$$/
mkdir $DISTCC_DIR

Kernel=`uname -r`
# CentOS 5.8
if [[ "$Kernel" == "2.6.18-308.el5" ]]; then
	DISTCC_CONF_FILE=/nethome/mrm/conf/distcc/hosts.58
	export DISTCC_EXEC_DIR=/nethome/sagia/distcc
# CentOs 6.3
elif [[ "$Kernel" == "2.6.32-279.el6.x86_64" ]]; then
	DISTCC_CONF_FILE=/nethome/mrm/conf/distcc/hosts.63
	export DISTCC_EXEC_DIR=/opt/polycom/distcc_6.3
# CentOs 6.5
elif [[ "$Kernel" == "2.6.32-431.el6.x86_64" ]]; then
	DISTCC_CONF_FILE=/nethome/mrm/conf/distcc/hosts.65
	export DISTCC_EXEC_DIR=/opt/polycom/distcc_6.3
else
	echo "Error:Unsupported compilation environment. Supported Centos versions are 5.8, 6.3 and 6.5" 
	kill -TERM $$
fi


if [[ "$USE_LOCAL_HOSTS" == "YES" && -f ~/bin/hosts ]];then
        echo "Use local hosts file"
        DISTCC_CONF_FILE=~/bin/hosts
fi

if [ ! -f $DISTCC_CONF_FILE ]; then
	echo "Error: DISTCC hosts file has not been found." 
	kill -TERM $$
else
	cp $DISTCC_CONF_FILE $DISTCC_DIR/hosts
fi


export MCMS_DIR=$PWD

if [ "$1" == "ver" ]
then
   if [ "$2" != "" ]
   then
	echo "*************************************"
	echo "Running Single Stream developer tests"
	echo "*************************************"
	
	
 	echo -n "Enter OFFICIAL BUILD (i.e. 158) and press [ENTER]:"	
        read OFFICIAL_BUILD
	export OFFICIAL_BUILD
   else
	BASELINE=$(cleartool lsbl -s| tail -n1)
	STRIP_VER=$BASELINE
	MAIN=${STRIP_VER%%.*}
	MAINORIG=${STRIP_VER%%.*}
	MAIN=$(expr "$MAINORIG" : '.*V\([0-9][0-9][0-9]\)')
	if [ "$?" != "0" ]
	then
	  MAIN=$(expr "$MAINORIG" : '.*V\([0-9][0-9]\)')
	  if [ "$?" != "0" ]
	  then
		MAIN=$(expr "$MAINORIG" : '.*V\([0-9]\)')
	  fi
	fi
	STRIP_VER=${STRIP_VER#*.}
	MAJOR=${STRIP_VER%%.*}
        VER_LOCATION=$(ls -l "/misc/carmelversions/NonStableBuild/RMX_$MAIN.$MAJOR/last" |  awk '{print $11}' )
	echo $VER_LOCATION	
	OFFICIAL_BUILD=$(expr "$VER_LOCATION" : 'RMX_*.*.*.*\([0-9][0-9][0-9]\)')
	if [ "$OFFICIAL_BUILD" == "" ]
	then
	   OFFICIAL_BUILD=$(expr "$VER_LOCATION" : 'RMX_*.*.*.*\([0-9][0-9]\)')
	fi
	echo "last offical build used: $OFFICIAL_BUILD"
	export OFFICIAL_BUILD
	echo "make mrproper"
	make mrproper
	echo "make active"
	make active
   fi
else
	if [ "$1" == "clean" ]; then
		find .  -name *.o -exec rm -f {} \;
		find .  -name *.a | grep -v /Utils/Image2IVRSlides/libvslide/lib/ | xargs rm -f \;
		rm -f Bin/*
		rm -f Stripped/*
		rm -f LogFiles/*
		rm -f CdrFiles/*
		rm -f *core*
		rm -f Cfg/Hlog/*
		rm -Rf TestResults/*
		rm -f httpd.access.log
		rm -Rf $MCU_HOME_DIR/tmp/EMACfg/*
		rm -Rf $MCU_HOME_DIR/tmp/OCS/*
		rm -Rf $MCU_HOME_DIR/tmp/Install/*
		rm -Rf MediaRecording/share/*
		rm -f Bin.*/*
		rm -f Processes/*/Makefile.auto
		rm -f bin rpm
		exit 0
	fi
fi

echo "Scripts Validation:"
for f in Scripts/*.sh;
	do
		## exception for this file because it's not a script but tcpdump binary file
		if [ $f != "Scripts/VNGR_20148_Sample_Tcpdump_Output.sh" ];
		then
			bash -n $f
			result=$?
			if [ $result != "0" ];
			then
				echo "#################################"
				echo "#   SCRIPTS VALIDATION FAILED   #"
				echo "#################################"
				exit 1
			fi
		fi
	done
echo "Scripts Validation done"


if [ -e Scripts/GenVersion.xml ];then
        Scripts/GenVersion.xml > Versions.xml
fi

Scripts/CleanAutoXmlGeneratedFiles.sh test || exit 1

export MAKEPARM=-j30
export DISTCC_BIN=$DISTCC_EXEC_DIR/bin/distcc
export PREMAKE=$DISTCC_EXEC_DIR/bin/pump
unset DISTCC_HOSTS


# export DISTCCMON=YES
if [ "$DISTCCMON" == "YES" ]; then
	 pgrep distccmon-gnome || distccmon-gnome &
fi

if [ "$SKIP_CODE_GEN" != "YES" ]; then
	$PREMAKE make automatic_sources makefiles $MAKEPARM || exit 1
fi

if [ -z "$BUILD_NAME" ]; then
	$PREMAKE make $MAKEPARM $* || exit 1
else
        #avoid running active target for mutiple compilations on host
	$PREMAKE make links targets  $MAKEPARM $* || exit 1
fi


./Scripts/CheckOpcodes.py

if [ "$?" == "1" ] 
then
	echo "CheckOpdodes failed"
	exit 1
fi

if [ "$1" == "ver" ]
then
	make test_scripts && ./Scripts/soft.sh test || exit 1
fi

rm -rf $DISTCC_DIR

# Setting Single Apache environment
sudo /opt/polycom/ks-correction.sh FIX_SALIB /opt/polycom/sim_pack/mcms_100.0_v016/SoftMCU > /dev/null
# End of Single Apache settings
