#!/bin/bash
# Startup.sh

#this is an example that should be placed in appropriated script
#MPL_SIM_FILE="VersionCfg/MPL_SIM_1.XML"  # for 1 MFA card
#MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"  # for 2 MFA cards
#LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_20_YES_v100.0.cfs" # for 20 ports in license
#LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_40_YES_v100.0.cfs" # for 40 ports in license
#USE_ALT_IP_SERVICE="VersionCfg/DefaultIPServiceList.xml"
#SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpm.txt"	  # for MPM mode
#SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"  # for MPM+ mode
#RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_5AUDIO_5CIF_5SD30_5HD720.xml" 

export LD_LIBRARY_PATH=$MCU_HOME_DIR/usr/local/apache2/lib:$MCU_HOME_DIR/mcms/Bin:$MCU_LIBS
export SASL_PATH=$MCU_HOME_DIR/mcms/Bin

export Kernel=`uname -r | cut -d '-' -f 1`


if [[ "$SOFT_MCU_FAMILY" == "YES" ]]; then
	# CentOs 6.x
	if [[ "$Kernel" == "2.6.32" ]]; then
		if [[ ! -d Bin.i32cent6x ]]; then
			echo "Bin.i32cent6x could not be found !"
		fi
	else
		echo "Bin.i32cent56 could not be found !"
	fi
else
	if [[ -d Bin.i32ptx ]]; then
		echo "Copy RMX binaries into Bin folder..."
		rm -Rf Bin/*
		cd Bin
		find ../Bin.i32ptx -type f -exec ln -sf {} . \;
		find ../Bin.i32ptx -type l -exec ln -sf {} . \;
		cd ..
		#(yes | cp Bin.i32ptx/* Bin/) 2> /dev/null
        #ls -altr Bin/
	else
		echo "Bin.i32ptx could not be found !"
	fi
	# In simulation mode we must add the local user. On target the use is mcms from httpd.conf
	#if [[ `grep CFG $MCU_HOME_DIR/mcms/Bin/ApacheModule | grep sim` ]]; then
		# For Single Apache + REST API
	#	cp -f $MCU_HOME_DIR/mcms/StaticCfg/httpd.rest.conf $MCU_HOME_DIR/tmp
	#	if [[ ! -w $MCU_HOME_DIR/tmp/httpd.rest.conf ]]; then
	#		chmod u+w $MCU_HOME_DIR/tmp/httpd.rest.conf
	#	fi 
	#	if [[ ! `grep "^User \`whoami\`" $MCU_HOME_DIR/tmp/httpd.rest.conf` ]]; then
	#		echo "User `whoami`" >> $MCU_HOME_DIR/tmp/httpd.rest.conf
	#	fi
	        # Fix Scripts/WaitForStartup.py errors
	#         if [[ ! `grep 'Listen 127.0.0.1:8080' $MCU_HOME_DIR/tmp/httpd.rest.conf` ]]; then
        #	        echo "Listen 127.0.0.1:8080" >> $MCU_HOME_DIR/tmp/httpd.rest.conf
        #	fi
	#fi
	#if [[ ! `grep "^Include $MCU_HOME_DIR/tmp/httpd.rest.conf" $MCU_HOME_DIR/mcms/StaticCfg/httpd.conf.sim` ]]; then
	#	echo "Include $MCU_HOME_DIR/tmp/httpd.rest.conf" >> $MCU_HOME_DIR/mcms/StaticCfg/httpd.conf.sim
	#fi
	if [[ ! -d $MCU_HOME_DIR/mcms/logs ]]; then
		mkdir $MCU_HOME_DIR/mcms/logs
	fi
	if [[ ! -L $MCU_HOME_DIR/mcms/EMA/htdocs ]]; then
		ln -s $MCU_HOME_DIR/opt/apache/htdocs $MCU_HOME_DIR/mcms/EMA/htdocs
	fi
	# End of Single Apache + REST API 
fi

if [[ "$SOFT_MCU_FAMILY" == "YES" ]]; then 
	export ENDPOINTSSIM=NO
	export GIDEONSIM=NO

	# Force Single Apache binary to be used
	rm -f $MCU_HOME_DIR/mcms/Bin/httpd
	if [ `whoami` == "mcms" ]; then               #kobig: run from network location for simulation
		ln -sf $MCU_HOME_DIR/usr/local/apache2/bin/httpd $MCU_HOME_DIR/mcms/Bin/
	else
		# CentOs 6.3
		if [[ "$Kernel" == "2.6.32" ]]; then
			ln -sf ../Bin.i32cent6x/httpd Bin/
		else
			ln -sf ../Bin.i32cent56/httpd Bin/
		fi
		
	fi
fi 

if [ "$ENDPOINTSSIM" != "NO" ]; then
	export ENDPOINTSSIM=YES
fi

Scripts/Cleanup.sh
Scripts/BinLinks.sh

ls -altr Links/

rm -f $MCU_HOME_DIR/tmp/queue/LoggerPipe
mkfifo $MCU_HOME_DIR/tmp/queue/LoggerPipe

rm -f $MCU_HOME_DIR/tmp/queue/SystemMonitoringPipe
mkfifo $MCU_HOME_DIR/tmp/queue/SystemMonitoringPipe

rm -f $MCU_HOME_DIR/tmp/httpd.conf*
rm -f $MCU_HOME_DIR/tmp/ssl.conf*

if test -e States/restore_config.flg; then
	Scripts/RestoreConfig.sh States 2>&1 1>/dev/null;
fi

echo "Cleaning Diagnostics Indication file States/EnterDiagnosticsind"
rm -f $MCU_HOME_DIR/mcms/States/EnterDiagnosticsind

mkdir -p $MCU_HOME_DIR/mcms/Backup -m 0777
mkdir -p $MCU_HOME_DIR/mcms/Restore -m 0777
mkdir -p $MCU_HOME_DIR/mcms/Install -m 0777
mkdir -p $MCU_HOME_DIR/mcms/LocalTracer -m 0777

ulimit -s 1024

if [ "$CLEAN_AUDIT_FILES" != "NO" ]; then
	echo Cleaning Audit
	rm -Rf Audit/*
fi

if [ "$CLEAN_LOG_FILES" != "NO" ]; then
	echo Cleaning LogFiles
	rm -Rf LogFiles/*
fi

if [ "$CLEAN_FAULTS" != "NO" ]; then
	echo Cleaning Faults
	rm -Rf Faults/*
fi

if [ "$CLEAN_CDR" != "NO" ]; then
	echo Cleaning CdrFiles
	rm -Rf CdrFiles/*
fi

if [ "$CLEAN_STATES" != "NO" ]; then
	echo Cleaning States
	rm -Rf States/*
fi

if [ "$CLEAN_MEDIA_RECORDING" != "NO" ]; then
	echo Cleaning MediaRecordings
	rm -Rf MediaRecording/share/*
fi

if [ "$LICENSE_FILE" == "" ]; then
	LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_60HD_v100.0.cfs"
fi

if [ "$RMX_4000" == "YES" ]; then
	echo -n RMX4000 > $MCU_HOME_DIR/mcms/ProductType

	if [ "$RMX_IN_SECURE" == "YES" ]; then
		MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK_4_SECURED.XML"
	fi

	if [ "$MPL_SIM_FILE" == "" ]; then
		MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK_4.XML"
	fi

	if [ "$RESOURCE_SETTING_FILE" == "" ]; then
		RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_Default_Breeze.xml"
	fi

elif [ "$RMX_1500" = "YES" ]; then
	echo -n RMX1500 > $MCU_HOME_DIR/mcms/ProductType

	if [ "$RMX_IN_SECURE" == "YES" ]; then
		MPL_SIM_FILE="VersionCfg/MPL_SIM_SECURED_BREEZE_ONE_CARD.XML"
	fi

	if [ "$MPL_SIM_FILE" == "" ]; then
		MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE_ONE_CARD.XML"
	fi

	if [ "$RESOURCE_SETTING_FILE" == "" ]; then
		RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_Default_Breeze.xml"
	fi 

elif [ "$RMX_1500Q" = "YES" ]; then
	echo -n RMX1500 > $MCU_HOME_DIR/mcms/ProductType

	if [ "$RMX_IN_SECURE" == "YES" ]; then
		MPL_SIM_FILE="VersionCfg/MPL_SIM_SECURED_BREEZE_ONE_CARD.XML"
	fi

	if [ "$LICENSE_FILE" == "" ]; then
		LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_1500Q_HDEnabled_25CIF.cfs"
	fi

	if [ "$MPL_SIM_FILE" == "" ]; then
		MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE_1500Q.XML"
	fi

	if [ "$RESOURCE_SETTING_FILE" == "" ]; then
		RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_1500Q_20_video.xml"
	fi

elif [ "$SOFT_MCU" == "YES" ]; then
	echo -n SOFT_MCU > $MCU_HOME_DIR/mcms/ProductType

elif [ "$SOFT_MCU_MFW" == "YES" ]; then
	echo -n SOFT_MCU_MFW > $MCU_HOME_DIR/mcms/ProductType

elif [ "$SOFT_MCU_EDGE" == "YES" ]; then
		echo -n SOFT_MCU_EDGE > $MCU_HOME_DIR/mcms/ProductType

elif [ "$GESHER" == "YES" ]; then
		echo -n GESHER > $MCU_HOME_DIR/mcms/ProductType
		echo -n GESHER > $MCU_HOME_DIR/tmp/EMAProductType.txt

elif [ "$NINJA" == "YES" ]; then
	echo -n NINJA > $MCU_HOME_DIR/mcms/ProductType
	echo -n NINJA > $MCU_HOME_DIR/tmp/EMAProductType.txt

elif  [ "$SOFT_MCU_CG" == "YES" ]; then
	echo -n SOFT_MCU_CG > $MCU_HOME_DIR/mcms/ProductType
	echo -n SOFT_MCU_CG > $MCU_HOME_DIR/tmp/EMAProductType.txt

else
	echo -n RMX2000 > $MCU_HOME_DIR/mcms/ProductType
	if [ "$RMX_IN_SECURE" == "YES" ]; then
		MPL_SIM_FILE="VersionCfg/MPL_SIM_SECURED_BREEZE.XML"
	fi

	if [ "$MPL_SIM_FILE" == "" ]; then
			MPL_SIM_FILE="VersionCfg/MPL_SIM.XML"
	fi
	if [ "$LICENSE_FILE" == "" ]; then
		LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_60HD_v100.0.cfs"
	fi

	if [ "$RESOURCE_SETTING_FILE" == "" ]; then
		RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_Default_Breeze.xml"
	fi 
fi

if [ "$CG" = "YES" ]; then
	echo -n CALL_GENERATOR > $MCU_HOME_DIR/mcms/ProductType
	ProductType=`cat ProductType`
fi

if [ "$PERSISTENT_CACHE" != "YES" ]; then
	echo Cleaning External
	rm -Rf External/*
fi

mkdir -p External/IVR

# We need latest UI_AIM RPM installed for REST API
if [[ -d ${OFFICIAL_DIR}/SoftMcuRPMs/RPMs/ ]]; then
	echo "Checking UI_AIM RPM"
	UI_AIM_RPM="`rpm -qa | grep UI_AIM`.rpm"
	UI_AIM_RPM_LATEST=`basename \`ls ${OFFICIAL_DIR}/SoftMcuRPMs/RPMs/Plcm-UI_AIM-*.el6*\``
	if [[ "X$UI_AIM_RPM" == "X" ]]; then
		echo "Installing the UI_AIM RPM"
		cp -f ${OFFICIAL_DIR}/SoftMcuRPMs/RPMs/Plcm-UI_AIM-*.el6* $MCU_HOME_DIR/tmp/UI_AIM.rpm
		sudo rpm -ivh --prefix / $MCU_HOME_DIR/tmp/UI_AIM.rpm --nodeps --noscripts
		##      Old RPMs don't support a prefix tag, therefore the previous installation fails, so we run it again without prefix.
                sudo rpm -ivh $MCU_HOME_DIR/tmp/UI_AIM.rpm --nodeps --noscripts
	else
		if [[ $UI_AIM_RPM != $UI_AIM_RPM_LATEST ]]; then
			echo "Updating the UI_AIM RPM"
			cp -f ${OFFICIAL_DIR}/SoftMcuRPMs/RPMs/Plcm-UI_AIM-*.el6* $MCU_HOME_DIR/tmp/UI_AIM.rpm
			sudo rpm -e Plcm-UI_AIM --nodeps --noscripts
			sudo rpm -ivh --prefix / $MCU_HOME_DIR/tmp/UI_AIM.rpm --nodeps --noscripts
			##      Old RPMs don't support a prefix tag, therefore the previous installation fails, so we run it again without prefix.
                        sudo rpm -ivh $MCU_HOME_DIR/tmp/UI_AIM.rpm --nodeps --noscripts
		else
			echo "You have the latest UI_AIM RPM already installed"
		fi
	fi
	rm -f $MCU_HOME_DIR/tmp/UI_AIM.rpm
	# Fix REST API php file
	if [[ ! `grep 'parent::__construct(&$request);' $MCU_HOME_DIR/opt/apache/htdocs/PHPUtils/RMXApiHandlers/RMXPartyApiHandler.php | grep '^#'` ]]; then
        	sed -i 's/^\(.*parent::__construct(.*request);.*\)/#\1/' $MCU_HOME_DIR/opt/apache/htdocs/PHPUtils/RMXApiHandlers/RMXPartyApiHandler.php
		sudo /opt/polycom/ks-correction.sh FIX_REST_API
	fi
fi
# End of UI_AIM section

if [ "$CLEAN_CFG" != "NO" ]; then
	echo clean keys
	rm -f Keys/*
	sudo rm -f $MCU_HOME_DIR/tmp/passphrase.sh
	sudo rm -f $MCU_HOME_DIR/tmp/passphraseplcm.sh
	echo Removing Cfg directory
	rm -Rf Cfg	
	mkdir -p Cfg/IVR
	echo "Cleaning LocalTracer Logs"
	rm -Rf LocalTracer/*
	rm -f /tmp/startup_logs/*.Ind
	ln -sf '../../External/IVR' Cfg/IVR/External
	mkdir Cfg/AudibleAlarms

	echo cleaning SoftMcuRun Remains
	if [[ "$SOFT_MCU_FAMILY" != "YES" ]]; then
            rm -Rf TS
	    sudo rm -f $MCU_HOME_DIR/tmp/httpd.pid
	fi

	cp -R VersionCfg/AudibleAlarms/* Cfg/AudibleAlarms/
	chmod -R +w Cfg/AudibleAlarms/*

	if [ "$USE_DEFAULT_IVR_SERVICE" != "NO" ]; then
		echo Copy Default IVR service
		cp -R VersionCfg/IVR/* Cfg/IVR/
		chmod -R +w Cfg/IVR/*
	fi

	if [ "$USE_DEFAULT_PSTN_SERVICE" != "NO" ]; then
		echo Copy Default PSTN service
		cp VersionCfg/RtmIsdnServiceList.xml Cfg/
		cp VersionCfg/RtmIsdnSpanMapList.xml Cfg/
		#cp VersionCfg/EthernetSettings.xml Cfg/
		chmod +w Cfg/RtmIsdnServiceList.xml
		chmod +w Cfg/RtmIsdnSpanMapList.xml
		#chmod +w Cfg/EthernetSettings.xml
	fi

	if [ "$USE_DEFAULT_IP_SERVICE" != "NO" ]; then
		if [ "$USE_ALT_IP_SERVICE" != "" ]; then
			echo Copy $USE_ALT_IP_SERVICE
			cp $USE_ALT_IP_SERVICE Cfg/IPServiceList.xml
			chmod +w Cfg/IPServiceList.xml
			if [[ "$CS_SIMULATION_TEST" == "YES" ]]; then
				sed -i -e "s/<IP>replace_ip_for_signalling/<IP>$CS_MCMS_IP_ADDRESS/g" Cfg/IPServiceList.xml
			fi
		else
			if [[ "$SOFT_MCU_FAMILY" == "YES" ]]; then
				echo SOFTMCU - Nothing to do. File creation is in the code. 
			else
				echo Copy Default IP: VersionCfg/DefaultIPServiceList.xml
				cp VersionCfg/DefaultIPServiceList.xml Cfg/IPServiceList.xml
				chmod +w Cfg/IPServiceList.xml
			fi
		fi
	fi

	if [ "$USE_DEFAULT_OPERATOR_DB" != "NO" ]; then
		echo Copy Default operator DB
		cp VersionCfg/OperatorDB.xml Cfg/OperatorDB.xml
		chmod +w Cfg/OperatorDB.xml
	fi

        # System cards' mode
        if [ -d $MCU_HOME_DIR/mcms/StaticStates ]; then
                rm -Rf $MCU_HOME_DIR/mcms/StaticStates/*
        else
                mkdir -p $MCU_HOME_DIR/mcms/StaticStates
                chmod a+w $MCU_HOME_DIR/mcms/StaticStates
        fi


	if [ "$USE_DEFUALT_NETWORK_MANAGMENT" != "NO" ]; then
		if [ "$USE_ALT_MNGMNT_SERVICE" != "" ]; then
			echo Copy $USE_ALT_MNGMNT_SERVICE
			cp $USE_ALT_MNGMNT_SERVICE Cfg/NetworkCfg_Management.xml
			chmod +w Cfg/NetworkCfg_Management.xml
		else		
			echo Copy NetworkCfg_Management.xml
			NetworkCfgWasExit=$true
			if [ ! -f Cfg/NetworkCfg_Management.xml ];then
        			NetworkCfgWasExit=$false
			fi
			cp VersionCfg/NetworkCfg_Management.xml Cfg/NetworkCfg_Management.xml
			chmod +w Cfg/NetworkCfg_Management.xml			
            if [ "$SOFT_MCU_EDGE" == "YES" ] || [ "$SOFT_MCU_MFW" == "YES" ]; then            	
				if [ ! $NetworkCfgWasExit ]; then
					echo "Changing into SECURED MODE in  NetworkCfg_Management.xml Reason: First Startup clean config" 
            			        sed -i -e "s/<IS_SECURED>false/<IS_SECURED>true/g" Cfg/NetworkCfg_Management.xml                    
					touch $MCU_HOME_DIR/mcms/StaticStates/UpgradeIntoSecureDone.flg
				fi				 
             fi
            
			export Host_name=$(hostname -s)
			sed -i -e "s/<HOST_NAME>Who-are-you/<HOST_NAME>$Host_name/g" Cfg/NetworkCfg_Management.xml
		fi
	fi
	
	if [ "$SYSTEM_CFG_USER_FILE" != "" ]; then
		echo Copy system cfg user file
		cp $SYSTEM_CFG_USER_FILE Cfg/SystemCfgUser.xml
		chmod +w Cfg/SystemCfgUser.xml
	fi

	if [ "$SYSTEM_CFG_DEBUG_FILE" != "" ]; then
		echo Copy system cfg debug file
		cp $SYSTEM_CFG_FILE Cfg/SystemCfgDebug.xml
		chmod +w Cfg/SystemCfgDebug.xml
	fi

	if [ "$USE_DEFAULT_TIME_CONFIGURATION" != "NO" ]; then
		echo Copy Default Time configuration
		cp VersionCfg/SystemTime.xml Cfg/SystemTime.xml
		chmod +w Cfg/SystemTime.xml
	fi 
	
	if [ "$ACTIVE_DIRECTORY_FILE" != "" ]; then
		echo Copy Active directory $ACTIVE_DIRECTORY_FILE
		cp $ACTIVE_DIRECTORY_FILE Cfg/ActiveDirectoryConfiguration.xml
		chmod +w Cfg/ActiveDirectoryConfiguration.xml
	fi

	cp VersionCfg/MPL_SIM.XML Cfg/
	cp VersionCfg/EP_SIM.XML Cfg/
	cp VersionCfg/MEDIA_MNGR_CFG.XML Cfg/
	chmod +w Cfg/MPL_SIM.XML
	chmod +w Cfg/MEDIA_MNGR_CFG.XML    

#	if [ "$MPL_SIM_FILE" != "" ]; then
#		echo Copy $MPL_SIM_FILE
#		cp $MPL_SIM_FILE Cfg/MPL_SIM.XML
#	fi

	if [ "$SOFT_MCU_CG" == "YES" ]; then
		SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmRx.txt"
	fi


	# Pizza's default: breeze
	cp VersionCfg/SystemCardsMode_breeze.txt $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt
	chmod a+w $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt

	if [ "$SYSTEM_CARDS_MODE_FILE" != "" ]; then
		echo Copy $SYSTEM_CARDS_MODE_FILE
		cp $SYSTEM_CARDS_MODE_FILE $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt
		chmod a+w $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt
	fi

	cp VersionCfg/Versions.xml Cfg/

	# Pizza's default: breeze
	cp VersionCfg/Resource_Setting_Default_Breeze.xml Cfg/Resource_Settings.xml
	chmod a+w Cfg/Resource_Settings.xml

	cp VersionCfg/License.cfs Cfg/
	if [ "$LICENSE_FILE" != "" ]; then
		echo Copy $LICENSE_FILE
		chmod +w Cfg/License.cfs
		cp $LICENSE_FILE Cfg/License.cfs
	fi

	cp VersionCfg/License.cfs Simulation/
	chmod +w Simulation/License.cfs

	if [ "$LICENSE_FILE" != "" ]; then
		echo Copy $LICENSE_FILE
		chmod +w Simulation/License.cfs
		cp $LICENSE_FILE Simulation/License.cfs
	fi

	if [ "$MPL_SIM_FILE" != "" ]; then
		echo Copy $MPL_SIM_FILE
		cp $MPL_SIM_FILE Cfg/MPL_SIM.XML
	fi

elif [[ "$SOFT_MCU_FAMILY" == "YES" ]]; then
	if [ -e Cfg/IVR ]; then 
		echo "IVR folder already exist - nothing to do"
	else
		mkdir -p Cfg/IVR
		echo Copy Default IVR service
		cp -R VersionCfg/IVR/* Cfg/IVR/
		chmod -R +w Cfg/IVR/*
	fi
fi

if [ "$CLEAN_CFG" == "NO" ]; then
	if [ "$GIDEONSIM_MNGMNT_CONFIG_FILE" == "" ]; then
		echo "set GIDEONSIM_MNGMNT_CONFIG_FILE to Cfg/NetworkCfg_Management.xml"
		export GIDEONSIM_MNGMNT_CONFIG_FILE=Cfg/NetworkCfg_Management.xml
	fi
	
	if [ "$SOFT_MCU_MFW" == "YES" ]; then
		if ! test -e $MCU_HOME_DIR/mcms/StaticStates/UpgradeIntoSecureDone.flg; then
				echo "Changing into SECURED MODE in NetworkCfg_Management.xml Reason: First Upgrade to Version Cfg is not a clean One" 
           		sed -i -e "s/<IS_SECURED>false/<IS_SECURED>true/g" Cfg/NetworkCfg_Management.xml
           		touch $MCU_HOME_DIR/mcms/StaticStates/UpgradeIntoSecureDone.flg          	 
		fi
	fi	 	
fi


# Update ApacheModule in MFW to use sudo only if required (port > 1023)
if [ "$SOFT_MCU_MFW" == "YES" ]; then  
       IS_SECURE=`echo -n \`cat $MCU_HOME_DIR/mcms/Cfg/NetworkCfg_Management.xml | grep '<IS_SECURED>true</IS_SECURED>'\``
       if [[ "$IS_SECURE" == "" ]]; then
                LISTENING_PORT=`grep -A 1 XML_API_PORT /mcu_custom_config/custom.cfg | grep DATA | cut -d'>' -f2 | cut -d'<' -f1`
       else
                LISTENING_PORT=`grep -A 1 XML_API_HTTPS_PORT /mcu_custom_config/custom.cfg | grep DATA | cut -d'>' -f2 | cut -d'<' -f1`
       fi
       if (( $LISTENING_PORT > 1023 )); then
                 sed -i -e 's/sudo //' $MCU_HOME_DIR/mcms/Bin/ApacheModule
                 sync
                 sleep 5
       else
 	      if [[ ! `cat $MCU_HOME_DIR/mcms/Bin/ApacheModule | grep CMD=\'sudo` ]]; then
	                 sed -i -e 's!/bin/env!sudo /bin/env!' $MCU_HOME_DIR/mcms/Bin/ApacheModule
	                 sync
	                 sleep 5
	       fi
       fi
fi



if test -e $MCU_HOME_DIR/mcms/Cfg/SystemCfgUserTmp.xml; then
	export SYSCFG_FILE="$MCU_HOME_DIR/mcms/Cfg/SystemCfgUserTmp.xml"
else
	export SYSCFG_FILE="$MCU_HOME_DIR/mcms/Cfg/SystemCfgUser.xml"
fi

Scripts/SetupCustomSlides.sh

if [[ "$SOFT_MCU_FAMILY" == "YES" && "$SOFT_MCU_MFW" != "YES" && "$SOFT_MCU_EDGE" != "YES" ]]; then
	echo "Getting the cloud IP"
	Scripts/GetCloudIp.sh
else
	echo "No need to get cloud IP - not SOFT_MCU"
fi

if [ "RMX_IN_SECURE" != "" ]; then
	echo Copy $MPL_SIM_FILE
	cp $MPL_SIM_FILE Cfg/MPL_SIM.XML	
fi

if [ "$RESOURCE_SETTING_FILE" != "" ]; then
	echo Copy $RESOURCE_SETTING_FILE
	cp $RESOURCE_SETTING_FILE Cfg/Resource_Settings.xml
fi

# Defines currect state of JITC flag: code copied from
# ./patches/post-compile-rootfs/etc/rc.d/07check_syscfg
if test -e $MCU_HOME_DIR/mcms/JITC_MODE.txt; then
	export JITC_MODE_CUR=$(cat $MCU_HOME_DIR/mcms/JITC_MODE.txt)
else
	export JITC_MODE_CUR="NO"
fi

export JITC_MODE_CFG=$(cat $SYSCFG_FILE | grep -A 1 ULTRA_SECURE_MODE | grep DATA)
if [ "$JITC_MODE_CFG" ] && [$JITC_MODE_CFG == "<DATA>YES</DATA>" ]; then
	echo "YES" > $MCU_HOME_DIR/mcms/JITC_MODE.txt
		export $JITC_MODE_CUR="YES"
	if [ $JITC_MODE_CUR == "NO" ]; then
		echo "YES" > $MCU_HOME_DIR/mcms/JITC_MODE_FIRST_RUN.txt
	fi
else
	echo "NO" > $MCU_HOME_DIR/mcms/JITC_MODE.txt
fi
# End of JITC flag

rm -Rf IVRX
mkdir IVRX
mkdir IVRX/RollCall
cp IVRX_Save/IVRX/RollCall/SimRollCall.aca IVRX/RollCall/SimRollCall.aca

mkdir -p Keys
mkdir -p KeysForCS
mkdir -p KeysForCS/cs1
mkdir -p KeysForCS/cs2
mkdir -p KeysForCS/cs3
mkdir -p KeysForCS/cs4
mkdir -p KeysForCS/cs5
mkdir -p KeysForCS/cs6
mkdir -p KeysForCS/cs7
mkdir -p KeysForCS/cs8
mkdir -p CACert
mkdir -p CRL

cp -f $MCU_HOME_DIR/mcms/Versions.xml $MCU_HOME_DIR/tmp

cp $MCU_HOME_DIR/mcms/Scripts/TLS/passphrase.sh     $MCU_HOME_DIR/tmp
cp $MCU_HOME_DIR/mcms/Scripts/TLS/passphraseplcm.sh $MCU_HOME_DIR/tmp
#cp -R KeysForCS/* $MCU_HOME_DIR/cs/ocs/

# THIS causes one of the process run under valgrind
if [ "$1" ]; then
	EXEC="Bin/"$1

	rm -f Links/$1
	echo "#!/bin/sh" > Links/$1

	if [ "$1" == "ApacheModule" ];
	then
		if [[ "$SOFT_MCU_FAMILY" == "YES" ]]
		then
			EXEC="sudo env MCU_HOME_DIR=$MCU_HOME_DIR LD_LIBRARY_PATH=$MCU_HOME_DIR/usr/local/apache2/lib:$MCU_HOME_DIR/mcms/Bin $MCU_HOME_DIR/mcms/Bin/httpd -f $MCU_HOME_DIR/mcms/StaticCfg/httpd.conf.sim -X"
		else
			export TARGET_PORT=8080			
			echo "sed 's/Listen 0.0.0.0:80/Listen 0.0.0.0:8080/g' $MCU_HOME_DIR/tmp/httpd.listen.conf  > $MCU_HOME_DIR/tmp/httpd.listen.tmp" >> Links/$1
			echo "mv $MCU_HOME_DIR/tmp/httpd.listen.tmp $MCU_HOME_DIR/tmp/httpd.listen.conf" >> Links/$1
			echo "sudo rm -f $MCU_HOME_DIR/tmp/httpd.pid" >> Links/$1
			EXEC="$MCU_HOME_DIR/mcms/Bin/httpd -f $MCU_HOME_DIR/mcms/StaticCfg/httpd.conf.sim -X"

		fi
	fi


	if [ "$2" == "gdb" ];
	then
		echo "xterm -e valgrind --tool=memcheck --db-attach=yes --demangle=no --num-callers=12 --suppressions=Scripts/ValgrindSup.txt "$EXEC >> Links/$1
		echo $1 "will run in valgrind in new xterm window, gdb will open on memory error"
	fi

	#set breakpoint before startup
	#example:
	#Scripts/Startup.sh ddd Resource ResourceManager.cpp:847

	if [ "$1" == "ddd" ];
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

	if [ "$2" == "gensup" ];  
	then
		echo "xterm -hold -e valgrind --tool=memcheck --num-callers=12 --demangle=no --suppressions=Scripts/ValgrindSup.txt --gen-suppressions=yes "$EXEC >> Links/$1
		echo $1 "will run in valgrind in new xterm window, suppession for errors will be printed"
	fi

	if [ "$2" == "prof" ];  
	then
		echo -n "valgrind --tool=callgrind " >> Links/$1
		echo -n "--trace-children=no --num-callers=8 " >> Links/$1
		echo  "--log-file=TestResults/$1.prof --callgrind-out-file=TestResults/$1.callgrind.out.%p "$EXEC >> Links/$1 
		echo $1 "will run in callgrind profiler"
	fi

	if [ "$2" == "massif" ];
	then
	echo -n "valgrind --tool=massif --demangle=yes --num-callers=14 --stacks=yes --log-file=TestResults/$1 " $EXEC >> Links/$1
		echo $1 "will run with valgrind using massif tool to analyze heap usage, run ms_print on the resulting output file"
	fi

	if [ "$2" == "memcheck" ];
	then
		echo -n "valgrind  --tool=memcheck --verbose --leak-check=yes --trace-children=no --gen-suppressions=all  --suppressions=Scripts/ValgrindSup.txt --demangle=yes --num-callers=14 --log-file=TestResults/$1 " $EXEC >> Links/$1
		echo $1 "will run in valgrind"
	fi

	if [ "$2" == "" ];
	then
		echo -n "valgrind -v --tool=memcheck --leak-check=yes " >> Links/$1
		echo -n "--trace-children=no " >> Links/$1
		echo "--suppressions=Scripts/ValgrindSup.txt  --gen-suppressions=all --demangle=no --num-callers=12 --log-file=TestResults/$1 "$EXEC >> Links/$1
		echo $1 "will run in valgrind"
	fi

	chmod u+x Links/$1
fi

SNMP_FIPS_MODE="NO"

SNMP_FIPS_MODE_CFG=$(cat $SYSCFG_FILE | grep -A 1 SNMP_FIPS_MODE | grep DATA)

if [ "$SOFT_MCU_MFW" != "YES" ];then
        SNMP_FIPS_MODE="NO"
else
	SNMP_FIPS_MODE="YES"
fi

if [ "$SNMP_FIPS_MODE_CFG" ];then
	if [ $SNMP_FIPS_MODE_CFG == "<DATA>YES</DATA>" ]; then
       		SNMP_FIPS_MODE="YES"
	else
		if [ $SNMP_FIPS_MODE_CFG == "<DATA>NO</DATA>" ]; then
			SNMP_FIPS_MODE="NO"
		fi 
	fi	
fi

if [ $SNMP_FIPS_MODE == "YES" ]; then
	touch /tmp/netsnmpfipsflag
fi

if [ "$SOFT_MCU" != "YES" ] && [ "$SOFT_MCU_MFW" != "YES" ] && [ "$GESHER" != "YES" ] && [ "$NINJA" != "YES" ] && [ "$SOFT_MCU_EDGE" != "YES" ]; then

        cp StaticCfg/snmpd.conf.sim $MCU_HOME_DIR/tmp/snmpd.conf
        chmod a+w $MCU_HOME_DIR/tmp/snmpd.conf

        if [ $SNMP_FIPS_MODE == "YES" ]; then
                echo "Running snmpdj"
                Bin/snmpdj &
        else
                echo "Running snmpd"
                Bin/snmpd &
        fi
fi

#PCM
#PCMSIM=YES
if [ "$PCMSIM" == "YES" ]; then
	echo -n "Running PCM"
	cd Bin/pcm && ./tv_menu_start_cop.sh &
fi

SELF_SIGNED_LOG_DIR=$MCU_HOME_DIR/tmp/startup_logs/self_signed_log.log
echo -n "Running Self-signed certificate"
Scripts/Self_Signed_Cert.sh boot > $SELF_SIGNED_LOG_DIR 2>&1

echo -n "Running McmsDaemon..."
MCMSD=Bin/McmsDaemon
MCMSD_LINK=$MCU_HOME_DIR/mcms/Links/McmsDaemon
if [ -e $MCMSD_LINK ]; then
	echo "(Using patched McmsDaemon)"
	MCMSD=$MCMSD_LINK
fi
$MCMSD&

sleep 7

if [ "$1" ]; then
	if [ "$1" == "ConfParty" ]; then
		echo Waiting extra 5 seconds since ConfParty process runs under valgrind
		sleep 5
	else
		echo Waiting extra 3 seconds since one of the processes runs under valgrind
		sleep 3
	fi
fi

echo "Wait 10 sec for netconfig to finish"
sleep 10

if [ "$ProductType" != "CALL_GENERATOR" ]; then
	if [ "$GIDEONSIM" != "NO" ]; then
		echo -n "Running GideonSim..."
		Links/GideonSim&
		usleep 200000
	else
		echo "GIDEONSIM env variable not set. Not running GideonSim..."
	fi

	if [ "$ENDPOINTSSIM" == "YES" ]; then
		echo -n "Running EndpointsSim..."
		Links/EndpointsSim&
		usleep 200000
		rm -f $MCU_HOME_DIR/tmp/SysIpTypeForSim
	else
		echo "ENDPOINTSSIM env variable not set. Not running EndpointsSim..."
		echo IpV4 >> $MCU_HOME_DIR/tmp/SysIpTypeForSim
	fi
fi

sleep 2

if [ "$1" ]; then
	echo "Startup.sh: Waiting extra 45 seconds since $1 runs under valgrind"
	sleep 45 
fi

#if [ "$DISABLE_WATCH_DOG" == "YES" ]
#then
#    echo "Disabling watch dog (debug mode)"
#    Bin/McuCmd set mcms WATCH_DOG NO < /dev/null
#fi

echo "Waiting 30 second (Otherwise trying to login and failing will have side effects for some scripts tests)..."
sleep 30 

if [ "$3" ] && [ $3 -ge 0 ]; then
	Scripts/WaitForStartup.py  $3 || exit 101
elif [[ `ps -ef | grep soft.sh | grep -v grep` ]]; then
	( sleep 50; Scripts/WaitForStartup.py 40 || exit 101 ) &
else
	Scripts/WaitForStartup.py 40 || exit 101
fi

# Set trace level to DEBUG_TRACE and disables dynamic change of the level 
if [[ "$SOFT_MCU" != "YES" ]] && [[ "$SOFT_MCU_MFW" != "YES" ]] && [[ "$SOFT_MCU_EDGE" != "YES" ]]; then
	Bin/McuCmd @set_level Logger 100 > /dev/null
fi

exit 0
