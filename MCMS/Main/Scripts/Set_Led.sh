#!/bin/sh

STARTUP_LOG=$MCU_HOME_DIR/tmp/startup_logs/start_sh.log

get_cp_process_state()
{
        echo get_cp_process_state | tee -a $STARTUP_LOG
        CP_PROCESS_STATE=`ps |grep cp | grep rmx_versions/$1 | awk -F ' ' '{print$4}'`
        echo CP_PROCESS_STATE=$CP_PROCESS_STATE | tee -a $STARTUP_LOG
}

echo parameter=$1 | tee -a $STARTUP_LOG

sleep 3
get_cp_process_state $1

while [ "$CP_PROCESS_STATE" = "cp" ]
do
        echo set LED of ACT and RDY blink | tee -a $STARTUP_LOG

        #LED ACT and RDY is blink
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr amber flickering
        $MCU_HOME_DIR/mcms/Bin/McuCmd change_led McuMngr green flickering

        sleep 2

        get_cp_process_state $1
done

exit 0
