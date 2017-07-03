#!/bin/sh

# Header
# TestName: 98UsbCheck.sh
# TestPath: /etc/init.d/
# Parameters: NULL
# ScriptOwnerMail: edwin.dong@polycom.com
# ScriptLog: output/usb/usb_check.log
# ScriptType: CLI
# End Header

EXIT_OK=0
EXIT_FAIL=1

IS_JITC_MODE=NO

USB_LOG_PATH=$MCU_HOME_DIR/output/usb
USB_LOG_FILE=usb_check.log
USB_MNT="/mnt/usb"
ENTER_DIAGNOSTIC_FILE=$MCU_HOME_DIR/mcms/States/enter_diagnostic.flg
export PATH=$PATH:/bin:/sbin:/usr/bin:/usr/sbin

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
    	echo "Remove Check Log File."
    fi
    
    touch "$USB_LOG_PATH/$USB_LOG_FILE"
    
    DATE=`date`
    echo "Check USB Key Time: $DATE" | tee -a "$USB_LOG_PATH/$USB_LOG_FILE" 
}

print_log()
{
    cat - | tee -a "$USB_LOG_PATH/$USB_LOG_FILE" 
}

#The funciton will be canceled if the system is in JITC mode
jitc_test()
{
    echo "jitc_test entering..." | print_log

    if [ -e "$MCU_HOME_DIR/mcms/JITC_MODE.txt" ]
    then
        IS_JITC_MODE=`cat $MCU_HOME_DIR/mcms/JITC_MODE.txt`
        if [ $IS_JITC_MODE = "YES" ]
        then
            echo "This system is in JITC mode and some USB functions is forbidded" | print_log
            #exit $EXIT_FAIL
        fi
        
        echo "jitc_test exit..." | print_log
    else
        echo "No JITC Mode flag, Continue..." | print_log
    fi
    
}

get_usb_led_device()
{
    echo "get_usb_led_device..." | print_log
    USB_LED_MNT=/proc/bus/usb

#wait 3 seconds for led usb mount ready.
    LED_USB_COUNT=3
    while [ $LED_USB_COUNT -gt 0 ] 
    do
        if [ ! -d $USB_LED_MNT ];
        then
            echo "USB LED Mount is not ready, countdown $LED_USB_COUNT " | print_log
            sleep 1
            COUNT=`expr $LED_USB_COUNT - 1`
        else
            LED_USB_COUNT=0
            echo "USB LED Mount:$USB_LED_MNT is ready." | print_log
            return $EXIT_OK
        fi
    done

    return $EXIT_FAIL
}

do_usb_led_action()
{
    echo "do_usb_led_action..." | print_log

    LIGHT_SRV_COUNT=3
    $MCU_HOME_DIR/mcms/Scripts/GesherLightSrv.sh start

    #wait 3 seconds for App LightSrv ready.	
    while [ $LIGHT_SRV_COUNT -gt 0 ] 
    do
        # to make sure processes:LightSrv are running
        Process_LightSrv=$(ps -ef | grep LightSrv | grep -v grep)                                                                    
        if [ "" == "${Process_LightSrv}" ]
        then                                 
            echo "USB LED Light Server:LightSrv is still not ready, countdown $LIGHT_SRV_COUNT" | print_log
            sleep 1
            COUNT=`expr $LIGHT_SRV_COUNT - 1`
        else
            LIGHT_SRV_COUNT=0
        fi
    done

    echo "USB LED Light Server is ready." | print_log
}

get_usb_device()
{
    echo "get_usb_device..." | print_log
    USB_CONFIG_FILE=$USB_MNT/usb_action.cfg
    
    #wait 5 seconds for usb ready.
    COUNT=5
    while [ $COUNT -gt 0 ] 
    do
      if [ ! -d $USB_MNT ] || [ ! -f $USB_CONFIG_FILE ];
	    then
	    echo "USB Key is not ready, countdown $COUNT " | print_log
	    sleep 1
	    COUNT=`expr $COUNT - 1`
      else
	    COUNT=0
      fi
    done

    if [ ! -d $USB_MNT ] || [ ! -f $USB_CONFIG_FILE ];
    then
        echo "USB Key is not inserted." | print_log
        if [ -f $ENTER_DIAGNOSTIC_FILE -a $IS_JITC_MODE != "YES" ]
        then
            echo "Find $ENTER_DIAGNOSTIC_FILE, Enter Diagnostic Mode!" | print_log
            rm -f $ENTER_DIAGNOSTIC_FILE
            $MCU_HOME_DIR/mcms/Scripts/NinjaDiagStart.sh &> /dev/null
        fi
        echo "Exit!" | print_log
        exit $EXIT_OK
    fi
   
    if [ "`ls -A $USB_MNT`" = "" ]
    then
        echo "USB Directory is Null, Exit!" | print_log
        exit $EXIT_OK
    fi

    echo "USB Mount:$USB_MNT  is ready." | print_log
}

read_usb_config()
{
    echo "read_usb_config..." | print_log
     
    if [ ! -f $USB_CONFIG_FILE ]
    then
        echo "Config file: $USB_CONFIG_FILE is not ready in USB Key, Exit!" | print_log
        exit $EXIT_FAIL
    fi
    
    #USB_UPGRADE_DOWNGRADE =YES
    TMP_ACTION_UPGRADE=`cat $USB_CONFIG_FILE | grep USB_UPGRADE_DOWNGRADE= | awk -F '=' '{ print $2 }' |  tr -d "\r"`
    TMP_ACTION_DIAGNOSTIC=`cat $USB_CONFIG_FILE | grep USB_DIAGNOSTIC= | awk -F '=' '{ print $2 }' |  tr -d "\r"`
    
    ACTION_UPGRADE=$(echo $TMP_ACTION_UPGRADE | tr [a-z] [A-Z]) 
    ACTION_DIAGNOSTIC=$(echo $TMP_ACTION_DIAGNOSTIC | tr [a-z] [A-Z]) 
    
    echo "usb action: USB_UPGRADE_DOWNGRADE=$ACTION_UPGRADE" | print_log
    echo "usb action: USB_DIAGNOSTIC=$ACTION_DIAGNOSTIC" | print_log
}

do_usb_action()
{
    echo "do_usb_action..." | print_log
    if [ $ACTION_UPGRADE == "YES" -a $IS_JITC_MODE != "YES" ]
    then
        echo "Do upgrade in USB Key!" | print_log
        $MCU_HOME_DIR/mcms/Scripts/98UsbUpgrade.sh &> /dev/null
    elif [ $ACTION_DIAGNOSTIC == "YES" -a $IS_JITC_MODE != "YES" ]
    then
        if [ "$PRODUCT_TYPE" == "NINJA" ]
        then
            echo "Enter diagnostic in USB Key!" | print_log
            $MCU_HOME_DIR/mcms/Scripts/NinjaDiagStart.sh usb &> /dev/null
        else
            echo "PRODUCT_TYPE is $PRODUCT_TYPE (not NINJA). Skip diagnostic mode!" | print_log
        fi
    else
        echo "No action is set to YES, do nothing!" | print_log
    fi
}

PRODUCT_TYPE=$(cat $MCU_HOME_DIR/mcms/ProductType)
log_prepare
jitc_test

get_usb_led_device
if [ $? != 0 ]
then
    echo "USB LED Mount failed, Do not running LightSrv!" | print_log
else
    echo "USB LED Mount succeed, Running LightSrv" | print_log
    do_usb_led_action
fi

get_usb_device
read_usb_config
do_usb_action

exit $EXIT_OK
