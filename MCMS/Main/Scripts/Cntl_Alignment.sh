#!/bin/sh

STARTUP_LOG=$MCU_HOME_DIR/tmp/startup_logs/start_sh.log

USB_MNT=$MCU_HOME_DIR/tmp/usb_stick1
USB_DEV=/dev/usb_stick1

USB_CONFIG_FILE=$USB_MNT/ver_alignment.cfg
USB_IMG_PATH=$USB_MNT/rmx_versions
USB_LOG_FILE=$USB_MNT/alignment.log

ALIGNMENT_FILE=$MCU_HOME_DIR/config/CntlSoftwareAlignment.cfg
RUN_SW_RECOVERY=`cat $ALIGNMENT_FILE | grep RUN_SW_RECOVERY= |awk -F '=' '{print $2}' `

print_log()
{
  #cat - | tee -a $STARTUP_LOG | tee -a $USB_LOG_FILE
  cat - | tee -a $STARTUP_LOG
}

#The funciton will be canceled if the system is in JITC mode
IS_JITC_MODE=`cat $MCU_HOME_DIR/mcms/JITC_MODE.txt`
if [ $IS_JITC_MODE = "YES" ]
then
        echo This system is in JITC mode and the CNTL Alignment is canceled | print_log
        exit 0
fi

#Judge if resotre factory
#if [ -f $MCU_HOME_DIR/config/states/Cntl_Alignment_RestoreFactoryFileInd.flg ]
#then
#        echo restore factory in CNTL Alignment and set RUN_SW_RECOVERY=ON | print_log
#        touch $ALIGNMENT_FILE
#        echo RUN_SW_RECOVERY=ON > $ALIGNMENT_FILE
#        rm -rf $MCU_HOME_DIR/config/states/Cntl_Alignment_RestoreFactoryFileInd.flg
#        exit 0
#fi
 
rm -rf $USB_LOG_FILE

echo RUN_SW_RECOVERY= $RUN_SW_RECOVERY | print_log
if [ "$RUN_SW_RECOVERY" != ON ]
then
        echo Exit the CNTL Alignment! | print_log
        exit 0
else
        #LED ACT and RDY is blinking
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr amber flickering
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr green flickering
fi


turn_off_alignment()
{
        echo Call turn_off_alignment | print_log
        touch $ALIGNMENT_FILE
        echo RUN_SW_RECOVERY=OFF > $ALIGNMENT_FILE
}

update_image2CF()
{
        echo Begin to update the image | print_log

        #LED ACT and RDY is blink
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr amber flickering
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr green flickering

        mount $MCU_HOME_DIR/data -o remount -o rw -o async -o noexec
        rm -f $MCU_HOME_DIR/data/`readlink $MCU_HOME_DIR/data/fallback`
        rm -f $MCU_HOME_DIR/data/fallback
        ln -s `readlink $MCU_HOME_DIR/data/current` $MCU_HOME_DIR/data/fallback
        
        $MCU_HOME_DIR/mcms/Scripts/Set_Led.sh $1 &
 
        cp $USB_IMG_PATH/$1 $MCU_HOME_DIR/data/$2
        rm -f $MCU_HOME_DIR/data/current
        ln -s $2 $MCU_HOME_DIR/data/current
        turn_off_alignment
        
        sync        

        echo Update image successfully! | print_log

        sleep 4
        #LED ACT and RDY is off
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr amber off
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr green off 

        umount $USB_DEV
        #reboot
        #added by huiyu on 2011.7.15
        rm -rf $MCU_HOME_DIR/mcms/StaticStates/SystemCardsMode.txt
        exit 0
}

get_usb_device()
{
        echo get_usb_device! | print_log
        HARD_DISK_DEV=`df | grep '$MCU_HOME_DIR/output' | awk -F ' ' '{ print $1;}'  | tr -d "\r" | cut -c 6-`
        echo Hard Disk Name = $HARD_DISK_DEV | print_log

        CF_CARD_DEV=`df | grep '$MCU_HOME_DIR/data' | awk -F ' ' '{ print $1;}'  | tr -d "\r" | cut -c 6-8`
        echo CF Card Name = $CF_CARD_DEV | print_log

        USB_DEV=`ls /dev/ | grep sd.1 | grep -v usb_stick | grep -v $HARD_DISK_DEV | grep -v $CF_CARD_DEV`

        if [ -z $USB_DEV ] ; then
            USB_DEV=`ls /dev/ | grep sd. | grep -v usb_stick | grep -v $HARD_DISK_DEV | grep -v $CF_CARD_DEV`
        fi

        if [ -z $USB_DEV ] ; then
            USB_DEV=null
        fi

        echo $USB_DEV | print_log 
}

wait_usb_insert()
{
        echo Please insert the USB key! | print_log
        
        #LED ACT and RDY is on
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr amber on
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr green on

        #detect the usb plugin
        get_usb_device

        while [ $USB_DEV = "null" ]
        do
                #LED ACT and RDY is on
                $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr amber on
                $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr green on

                echo please insert usb
                sleep 2
                get_usb_device
        done

        USB_MNT=$MCU_HOME_DIR/tmp/usb_stick_$USB_DEV
        USB_DEV=/dev/$USB_DEV

        USB_CONFIG_FILE=$USB_MNT/ver_alignment.cfg
        USB_IMG_PATH=$USB_MNT/rmx_versions
        USB_LOG_FILE=$USB_MNT/alignment.log
}

wait_usb_unplug()
{
        echo Please unplug the USB key! | print_log
        umount $USB_DEV        
        
        #LED ACT is on and RDY is blinking
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr amber on
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr green flickering

        while test -e $USB_DEV
        do
                #LED ACT is on and RDY is blinking
                $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr amber on
                $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr green flickering                
                
                echo please unplug usb
                sleep 2
        done
}

get_rtmip_ver()
{
RTMIP_VER=`wget -t 1 -T 10 -O - -o $MCU_HOME_DIR/tmp/cntl_alignment_wget.log http://169.254.128.16/EMA.UI/JavaScript/EMAHtmlVariables.js | grep MCU_BUILD | awk -F ' ' '{ print $4 }' | tr -d "\"\;"`
STRING_RESULT_1=`cat $MCU_HOME_DIR/tmp/cntl_alignment_wget.log | grep 'Connection refused' |awk -F ' ' '{print $5}'`
STRING_RESULT_2=`cat $MCU_HOME_DIR/tmp/cntl_alignment_wget.log | grep 'Connection refused' |awk -F ' ' '{print $6}'`
}

count=1

get_rtmip_ver

while [ -z $RTMIP_VER ] && [ "$count" -le 40 ]
do
        echo Can not get RTM-IP version!| print_log
        sleep 2
        get_rtmip_ver
        count=$(($count+1))

        #LED ACT and RDY is blinking
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr amber flickering
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr green flickering
done

#When perform "comprehensive resotore", Web server is down in RTM-IP. The version of RTM-IP can not be got from Switch.
if [ $STRING_RESULT_1 = "Connection" ] && [ $STRING_RESULT_2 = "refused." ]
then
        echo WEB server is down in RTM-IP, can not get the version of RTM-IP! | print_log
        killall -9 mountd
        sleep 1
        turn_off_alignment
        sync
        /etc/rc.d/22nfsd
        /sbin/mountd
        echo Exit the CNTL Alignment! | print_log
        exit 0
fi


if [ -z $RTMIP_VER ] ; then
        RTMIP_VER=UNKNOWN
fi
echo RTMIP_VER=$RTMIP_VER | print_log

CNTL_VER=`cat /version.txt`
echo CNTL_VER=$CNTL_VER | print_log


if [ $CNTL_VER = $RTMIP_VER ] ; then
        echo Version matched! | print_log
        killall -9 mountd
        sleep 1

        #LED ACT is off, RDY is on
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr amber off
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr green on

        turn_off_alignment
        sync
        /etc/rc.d/22nfsd
        /sbin/mountd
        exit 0
else
        echo Version not matched... | print_log
fi

        wait_usb_insert

        sleep 2

        test -d "$USB_MNT" || mkdir -p "$USB_MNT"
        test -d "$USB_IMG_PATH" || mkdir -p "$USB_IMG_PATH"

        CNTL_MAC=`ifconfig | grep HWaddr | awk -F ' ' '{ print $5; exit}'`
        echo Control Unit mac = $CNTL_MAC | print_log

        touch $USB_CONFIG_FILE

        #USER_SELECTED_VERSION=`cat $USB_CONFIG_FILE | grep USER_SELECTED_VERSION= | awk -F '=' '{ print $2 }' | tr -d "\r"`
        #echo User Selected Version = $USER_SELECTED_VERSION | print_log

        #update the config file for version alignment
        echo update the file ver_alignment.cfg | print_log

        echo UTILITY_VERSION=1.0.0 > $USB_CONFIG_FILE
        echo CONTROL_UNIT_VERSION=$CNTL_VER >> $USB_CONFIG_FILE
        echo CONTROL_UNIT_MAC=$CNTL_MAC >> $USB_CONFIG_FILE
        echo RTM_IP_VERSION=$RTMIP_VER >> $USB_CONFIG_FILE
        echo SYSTEM_RECOMMENDED_VERSION=$RTMIP_VER >> $USB_CONFIG_FILE
        echo USER_SELECTED_VERSION=$USER_SELECTED_VERSION >>$USB_CONFIG_FILE
        sync

while true
do
        touch $USB_CONFIG_FILE 
        RECORD_MAC=`cat $USB_CONFIG_FILE | grep CONTROL_UNIT_MAC= | awk -F '=' '{ print $2 }' |  tr -d "\r"`
        if [ "$RECORD_MAC" != "$CNTL_MAC" ] ; then
                echo The MAC is not matched to the record. Maybe a wrong USB stick is inserted. | print_log
                echo Please change to the binded USB stick or reboot RMX to reset the MAC binding |print_log

                wait_usb_unplug
                
                wait_usb_insert
                continue
        fi


        USER_SELECTED_VERSION=`cat $USB_CONFIG_FILE | grep USER_SELECTED_VERSION= | awk -F '=' '{ print $2 }' | tr -d "\r"`
        echo User Selected Version = $USER_SELECTED_VERSION | print_log

        if [ $USER_SELECTED_VERSION ] ; then
                IMAGE_NAME=`ls $USB_IMG_PATH | grep $USER_SELECTED_VERSION | awk '{ print; exit }'`
                echo Selected Image Name = $IMAGE_NAME | print_log
                if [ $IMAGE_NAME ]
                then
                        update_image2CF $IMAGE_NAME $IMAGE_NAME
                else
                        echo Please upload image of Version:$USER_SELECTED_VERSION to USB stick | print_log
                fi
        elif [ $RTMIP_VER = UNKNOWN ] ; then
                echo Can not get RTM-IP version and please select a version! | print_log
        else
                IMAGE_NAME=`ls $USB_IMG_PATH | grep $RTMIP_VER | awk '{ print; exit }'`
                echo Recommended Image Name = $IMAGE_NAME | print_log
                if [ $IMAGE_NAME ] ; then
                        update_image2CF $IMAGE_NAME $IMAGE_NAME
                else
                        echo Please upload image of Version:$RTMIP_VER to USB stick | print_log
                fi
        fi

 
        wait_usb_unplug

        wait_usb_insert
        
        sleep 2
done
