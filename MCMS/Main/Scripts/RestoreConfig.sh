#!/bin/sh -x

	echo "Restore Config"

	delete_restore_file()
	{
		mount $MCU_HOME_DIR/data -o remount -o rw -o async -o noexec
		rm -f Restore/${RESTORE_FILE}
		mount $MCU_HOME_DIR/data -o remount -o ro -o async -o noexec
	}

	# delete restore configuration indicator
        rm -f $1/restore_config.flg

        RESTORE_FILE=`ls -tr Restore/ | tail -1`
	# ensuring restore file exists
	if [ a${RESTORE_FILE} = "a" ]; then
		touch $1/restore_config_failed_ls.flg
		exit 0
	fi

	#DECRYPT="openssl bf -d -k 31415926535"
	#This ^ line has refernces in Restore scripts

	# re-validate decrypted-tarred file, 
	# the 1st validation is done before restart, in code
	#cat Restore/${RESTORE_FILE} | ${DECRYPT} | tar tz 2>/dev/null
	cat Restore/${RESTORE_FILE} | tar tz 2>/dev/null
	if [ "$?" != "0" ]; then
		touch $1/restore_config_failed_tar_tz.flg
		delete_restore_file
		exit 0	
	fi 

	# delete configuration and extract restore file, instead.
	rm -Rf Cfg EMACfg /mcu_custom_config &>/dev/null
        #cat Restore/${RESTORE_FILE} | ${DECRYPT} | tar xz 2>/dev/null
	cat Restore/${RESTORE_FILE} | tar xz 2>/dev/null
	SUCCESS=`echo $?`
	
	#copy mcu_custom_config to root (for MFW)
	if [ -d $MCU_HOME_DIR/mcu_custom_config/ ]; then
        	rm -Rf $MCU_HOME_DIR/mcu_custom_config/* &>/dev/null
        	cp -r mcu_custom_config/* $MCU_HOME_DIR/mcu_custom_config/ &>/dev/null
	else
        	mv mcu_custom_config $MCU_HOME_DIR/ &>/dev/null
	fi
	rm -Rf mcu_custom_config &>/dev/null
	
	# delete Restore file
	delete_restore_file

	# raise indicator for further analysis by BackupRestore process
	if [ "$SUCCESS" != "0" ]; then
		touch $1/restore_config_failed_tar_xz.flg
	else
		touch $1/restore_config_succeeded.flg
	fi

