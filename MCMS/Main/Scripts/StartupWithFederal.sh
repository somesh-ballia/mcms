#!/bin/sh

#this is an example that should be placed in appropriated script
#MPL_SIM_FILE="VersionCfg/MPL_SIM_1.XML"  # for 1 MFA card
#MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"  # for 2 MFA cards
#LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_20_YES_v100.0.cfs" # for 20 ports in license
#LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_40_YES_v100.0.cfs" # for 40 ports in license
#USE_ALT_IP_SERVICE="VersionCfg/DefaultIPServiceList.xml"
#SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpm.txt"	  # for MPM mode
#SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"  # for MPM+ mode
#RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_5AUDIO_5CIF_5SD30_5HD720.xml" 


Scripts/Cleanup.sh
Scripts/BinLinks.sh

rm -f $MCU_HOME_DIR/tmp/queue/LoggerPipe
mkfifo $MCU_HOME_DIR/tmp/queue/LoggerPipe

Scripts/GenVersion.xml > Versions.xml

#cp -f StaticCfg/snmpd.conf.sim $MCU_HOME_DIR/tmp/snmpd.conf
#./snmpd -d -DALL -l $MCU_HOME_DIR/tmp/snmpd.log -c $MCU_HOME_DIR/tmp/snmpd.conf 0.0.0.0:8090

ulimit -s 1024

if [ "$CLEAN_AUDIT_FILES" != "NO" ]
then
    echo Removing Audit directory
    rm -Rf Audit
fi

if [ "$CLEAN_LOG_FILES" != "NO" ]
then
    echo Removing LogFiles directory
    rm -Rf LogFiles
fi

if [ "$CLEAN_FAULTS" != "NO" ]
then
    echo Cleaning Faults directory
    rm -Rf Faults
    mkdir Faults
fi

if [ "$CLEAN_CDR" != "NO" ]
then
    echo Cleaning CdrFiles directory
    rm -Rf CdrFiles
fi

if [ "$CLEAN_STATES" != "NO" ]
then
    echo Cleaning States directory
    rm -Rf States
fi

if [ "$CLEAN_CFG" != "NO" ]
then
    echo Removing Cfg directory
    rm -Rf Cfg
    mkdir Cfg
    mkdir Cfg/IVR
#    ln -sf $MCU_HOME_DIR/output/IVR Cfg/IVR
#    rm -Rf $MCU_HOME_DIR/output/IVR/*

    if [ "$USE_DEFAULT_IVR_SERVICE" != "NO" ]
		then
		echo Copy Default IVR service
		cp -R VersionCfg/IVR/* Cfg/IVR/
		chmod -R +w Cfg/IVR/*
    fi

    if [ "$USE_DEFAULT_PSTN_SERVICE" != "NO" ]
	then
	echo Copy Default PSTN service
	cp VersionCfg/RtmIsdnServiceList.xml Cfg/
	cp VersionCfg/RtmIsdnSpanMapList.xml Cfg/
	chmod +w Cfg/RtmIsdnServiceList.xml
	chmod +w Cfg/RtmIsdnSpanMapList.xml
    fi
    
	if [ "$USE_DEFAULT_IP_SERVICE" != "NO" ]
		then
		if [ "$USE_ALT_IP_SERVICE" != "" ]
	          then
	          echo Copy $USE_ALT_IP_SERVICE
	          cp $USE_ALT_IP_SERVICE Cfg/IPServiceList.xml
              chmod +w Cfg/IPServiceList.xml
        else
        echo Copy Default IP: VersionCfg/DefaultIPServiceList.xml
		cp VersionCfg/DefaultIPServiceList.xml Cfg/IPServiceList.xml
		chmod +w Cfg/IPServiceList.xml
		fi
    fi
	    
    if [ "$USE_DEFAULT_OPERATOR_DB" != "NO" ]
	then
	echo Copy Default operator DB
	cp VersionCfg/OperatorDB.xml Cfg/OperatorDB.xml
	chmod +w Cfg/OperatorDB.xml
    fi
    if [ "$USE_DEFUALT_NETWORK_MANAGMENT" != "NO" ]
	then
	echo Copy NetworkCfg_Management.xml
	cp VersionCfg/NetworkCfg_Management.xml Cfg/NetworkCfg_Management.xml
	chmod +w Cfg/NetworkCfg_Management.xml
    fi
    if [ "$SYSTEM_CFG_USER_FILE" != "" ]
	then
	echo Copy system cfg user file
	cp $SYSTEM_CFG_USER_FILE Cfg/SystemCfgUser.xml
	chmod +w Cfg/SystemCfgUser.xml
    fi
    if [ "$SYSTEM_CFG_DEBUG_FILE" != "" ]
	then
	echo Copy system cfg debug file
	cp $SYSTEM_CFG_FILE Cfg/SystemCfgDebug.xml
	chmod +w Cfg/SystemCfgDebug.xml
    fi
	if [ "$USE_DEFAULT_TIME_CONFIGURATION" != "NO" ]
		then
		echo Copy Default Time configuration
		cp VersionCfg/SystemTime.xml Cfg/SystemTime.xml
		chmod +w Cfg/SystemTime.xml
    fi 
    cp VersionCfg/MPL_SIM.XML Cfg/
    cp VersionCfg/EP_SIM.XML Cfg/
    chmod +w Cfg/MPL_SIM.XML

#    if [ "$MPL_SIM_FILE" != "" ]
#	then
#	echo Copy $MPL_SIM_FILE
#	cp $MPL_SIM_FILE Cfg/MPL_SIM.XML
#   fi
	
    if [ "$SYSTEM_CARDS_MODE_FILE" != "" ]
	then
	echo Copy $SYSTEM_CARDS_MODE_FILE
	mkdir Cfg/SystemCardsMode
	cp $SYSTEM_CARDS_MODE_FILE Cfg/SystemCardsMode/SystemCardsMode.txt
    fi

   cp VersionCfg/Versions.xml Cfg/
   
   cp VersionCfg/Resource_Settings.xml Cfg/
   chmod a+w Cfg/Resource_Settings.xml
   
   cp VersionCfg/License.cfs Cfg/
    if [ "$LICENSE_FILE" != "" ]
	then
	echo Copy $LICENSE_FILE
    chmod +w Cfg/License.cfs
	cp $LICENSE_FILE Cfg/License.cfs
    fi

   cp VersionCfg/License.cfs Simulation/
    chmod +w Simulation/License.cfs
    if [ "$LICENSE_FILE" != "" ]
	then
	echo Copy $LICENSE_FILE
    chmod +w Simulation/License.cfs
	cp $LICENSE_FILE Simulation/License.cfs
    fi

fi

if [ "$MPL_SIM_FILE" != "" ]
	then
	echo Copy $MPL_SIM_FILE
	cp $MPL_SIM_FILE Cfg/MPL_SIM.XML
fi

if [ "$RESOURCE_SETTING_FILE" != "" ]
	then
	echo Copy $RESOURCE_SETTING_FILE
	cp $RESOURCE_SETTING_FILE Cfg/Resource_Settings.xml
fi

rm -Rf IVRX
mkdir IVRX
mkdir IVRX/RollCall
cp IVRX_Save/IVRX/RollCall/SimRollCall.aca IVRX/RollCall/SimRollCall.aca


# ----- Preperations for JITC Mode -----

cp StaticCfg/SystemCfgUser.xml Cfg/SystemCfgUser.xml
chmod 666 Cfg/SystemCfgUser.xml
echo YES > $MCU_HOME_DIR/mcms/JITC_MODE.txt
echo YES > $MCU_HOME_DIR/tmp/OLD_JITC_MODE.txt


# THIS causes one of the process run under valgrind
if [ "$1" ]
then
  EXEC="Bin/"$1
  if [ "$1" == "ApacheModule" ]
  then
     EXEC="$MCU_HOME_DIR/mcms/httpd -f $MCU_HOME_DIR/tmp/httpd.conf.sim -X"
  fi
  rm -f Links/$1
  echo "#!/bin/sh" > Links/$1
  if [ "$2" == "gdb" ]  
  then
      echo "xterm -e valgrind --tool=memcheck --db-attach=yes --num-callers=12  --suppressions=Scripts/ValgrindSup.txt "$EXEC >> Links/$1
      echo $1 "will run in valgrind in new xterm window, gdb will open on memory error"
  fi
  
#set breakpoint before startup
#example:
#Scripts/Startup.sh ddd Resource ResourceManager.cpp:847

if [ "$1" == "ddd" ]  
then
	  echo the $2 process will run under ddd
	  echo "set breakpoint pending on" > run.txt
	  echo "b "$3 >> run.txt
	  echo "run" >> run.txt

	  (echo "#!/bin/sh";
           echo "rm -f ~/.gdbinit";
	   echo "ddd Bin/$2 --command=$MCU_HOME_DIR/mcms/run.txt") > Links/$2
	  chmod u+x Links/$2
fi



  if [ "$2" == "gensup" ]  
  then
      echo "xterm -e valgrind --tool=memcheck --num-callers=12  --suppressions=Scripts/ValgrindSup.txt --gen-suppressions=yes "$EXEC >> Links/$1
      echo $1 "will run in valgrind in new xterm window, suppession for errors will be printed"
  fi

  
  if [ "$2" == "prof" ]  
  then
      echo -n "callgrind  " >> Links/$1
      echo -n "--trace-children=no " >> Links/$1
      echo " --num-callers=8 --log-file=TestResults/$1.prof "$EXEC >> Links/$1
      echo $1 "will run in callgrind profiler"
  fi
  
  if [ "$2" == "" ]  
  then
      echo -n "valgrind  --tool=memcheck --leak-check=yes " >> Links/$1
      echo -n "--trace-children=no " >> Links/$1
      echo "--suppressions=Scripts/ValgrindSup.txt --num-callers=8 --log-file=TestResults/$1 "$EXEC >> Links/$1
      echo $1 "will run in valgrind"
  fi

  chmod u+x Links/$1
fi


echo -n "Running snmpd"
cp StaticCfg/snmpd.conf.sim $MCU_HOME_DIR/tmp/snmpd.conf
chmod a+w $MCU_HOME_DIR/tmp/snmpd.conf

Bin/snmpd &


echo -n "Running McmsDaemon..."
MCMSD=Bin/McmsDaemon
MCMSD_LINK=$MCU_HOME_DIR/mcms/Links/McmsDaemon
if [ -e $MCMSD_LINK ] 
    then
    echo "(Using patched McmsDaemon)"
    MCMSD=$MCMSD_LINK
fi
$MCMSD&

sleep 7

if [ "$1" ]
then
    if [ "$1" == "ConfParty" ]
    then
        echo Waiting extra 5 seconds since ConfParty process runs under valgrind
	sleep 5
    else
        echo Waiting extra 3 seconds since one of the processes runs under valgrind
    	sleep 3
    fi
fi

if [ "$GIDEONSIM" == "YES" ]
then
    echo -n "Running GideonSim..."
    Links/GideonSim&
    usleep 200000
else
    echo -n "GIDEONSIM env variable not set. Not running GideonSim..."
fi

if [ "$ENDPOINTSSIM" == "YES" ]
then
    echo -n "Running EndpointsSim..."
    Links/EndpointsSim&
    usleep 200000
else
   echo -n "ENDPOINTSSIM env variable not set. Not running EndpointsSim..." 
fi

sleep 2

if [ "$1" ]
then
    echo Waiting extra 2 seconds since one of the processes runs under valgrind
    sleep 2
fi

#if [ "$DISABLE_WATCH_DOG" == "YES" ]
#then
#    echo "Disabling watch dog (debug mode)"
#    Bin/McuCmd set mcms WATCH_DOG NO < /dev/null
#fi

echo "Waiting 25 second (Otherwise login fails)..."
sleep 25

echo "and another 3 seconds (for gk_registraion)..."
sleep 3

#Scripts/WaitForStartupFederal.py || exit 101

exit 0
