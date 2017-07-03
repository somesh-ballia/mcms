#!/bin/sh

# Header
# TestName: 98UsbUpgrade.sh
# TestPath: /etc/init.d/
# Parameters: NULL
# ScriptOwnerMail: flora.yao@polycom.com
# ScriptLog: output/usb/usb_upgrade.log
# ScriptType: CLI
# End Header

EXIT_OK=0
EXIT_FAIL=1

USB_LOG_PATH=$MCU_HOME_DIR/output/usb
USB_LOG_FILE=usb_upgrade.log
USB_MNT="/mnt/usb"

log_prepare()
{
    if [ ! -d "$USB_LOG_PATH" ]
    then
        echo "mkdir $USB_LOG_PATH"
    	mkdir -p "$USB_LOG_PATH"
    fi
    
    if [ -f "$USB_LOG_PATH/$USB_LOG_FILE" ]
    then
    	rm -f "$USB_LOG_PATH/$USB_LOG_FILE"
    	echo "Remove Update Log File."
    fi
    
    touch "$USB_LOG_PATH/$USB_LOG_FILE"
    
    DATE=`date`
    echo "Upgrading with USB Key Time: $DATE" | tee -a "$USB_LOG_PATH/$USB_LOG_FILE" 
}

print_log()
{
    cat - | tee -a "$USB_LOG_PATH/$USB_LOG_FILE" 
}

led_upgrade_in_progress()
{
    echo "LED: led_upgrade_in_progress..." | print_log
	$MCU_HOME_DIR/mcms/Bin/LightCli UsbUpgrade usb_upgrade_in_progress &> /dev/null	
}

led_upgrade_complete()
{
    echo "LED: led_upgrade_complete..." | print_log
	$MCU_HOME_DIR/mcms/Bin/LightCli UsbUpgrade usb_upgrade_completed &> /dev/null	
}

led_upgrade_failed()
{
    echo "LED: led_upgrade_complete..." | print_log
	$MCU_HOME_DIR/mcms/Bin/LightCli HotStandby single_mode &> /dev/null	
}

usb_upgrade()
{
    echo "usb_upgrade..." | print_log
    USB_IMG_PATH=$USB_MNT/rmx_versions
    USB_CONFIG_FILE=$USB_MNT/usb_action.cfg

    usb_imageCheck
    if [ $? != 0 ]
    then
        echo "usb_imageCheck Failed!" | print_log
        exit $EXIT_FAIL
    else
		led_upgrade_in_progress
        #execute sum&firmware check
        # $MCU_HOME_DIR/mcms/Scripts/SoftUpgrade {factory|fallback|file} <file> <IsNeedSUMCheck> <IsNeedVersionCheck> <LogFilePath>
        $MCU_HOME_DIR/mcms/Scripts/SoftUpgrade.sh file $USB_IMG_PATH/$IMAGE_NAME YES YES $USB_LOG_PATH/$USB_LOG_FILE &> /dev/null
	    INSTALL_RET=$?
        #0: Succeed to Install.
        #not 0: Failed to Install.
        if [ $INSTALL_RET != 0 ]
        then
            echo "Call SoftUpgrade to Check&Install new image Failed, Reason = $INSTALL_RET!" | print_log
			led_upgrade_failed
			exit $EXIT_FAIL
        else
            echo "Call SoftUpgrade to Check&Install new image Succeed!" | print_log
            led_upgrade_complete
            return $EXIT_OK
        fi
    fi
    
    
   
}

usb_imageCheck()
{
    echo "usb_image2CF..." | print_log
    if [ ! -d $USB_IMG_PATH ]
    then
        echo "New version file is not ready in USB Key: $USB_IMG_PATH, Exit!" | print_log
        exit $EXIT_FAIL
    fi
    
    USER_SELECTED_VERSION=`cat $USB_CONFIG_FILE | grep USER_SELECTED_VERSION= | awk -F '=' '{ print $2 }' | tr -d "\r"`
    echo "User Selected Version = $USER_SELECTED_VERSION" | print_log

    if [ $USER_SELECTED_VERSION ]
    then
        IMAGE_NAME=`ls $USB_IMG_PATH | grep $USER_SELECTED_VERSION | awk '{ print }'`
        echo "Selected Image Full Path: = $USB_IMG_PATH/$IMAGE_NAME" | print_log
        
        if [ $IMAGE_NAME ]
        then
            return $EXIT_OK
        else
            echo "Please upload image of Version:$USER_SELECTED_VERSION to USB stick" | print_log
            exit $EXIT_FAIL
        fi   
    else
        echo "User didn't sellect the upgraded version file, Exit!" | print_log
        exit $EXIT_FAIL
    fi
    
    return $EXIT_OK
}

log_prepare
usb_upgrade

if [ $? != 0 ]
then
    echo "Update image failed!" | print_log
    exit $EXIT_FAIL
else
    echo "Update image succeed, Waiting for User Reboot in 30 sec...!" | print_log
    sleep 30
    echo  "Auto Reboot...!" | print_log
    reboot
fi

exit $EXIT_OK
