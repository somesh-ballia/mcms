#!/bin/sh

Scripts/batchkill.sh
rm -fR $MCU_HOME_DIR/tmp/queue
rm -fR $MCU_HOME_DIR/tmp/shared_memory
rm -fR $MCU_HOME_DIR/tmp/semaphore
rm -fR $MCU_HOME_DIR/tmp/802_1xCtrl
mkdir $MCU_HOME_DIR/tmp/queue
mkdir $MCU_HOME_DIR/tmp/shared_memory
mkdir $MCU_HOME_DIR/tmp/semaphore
mkdir $MCU_HOME_DIR/tmp/802_1xCtrl
rm -f $MCU_HOME_DIR/tmp/loglog.txt
if [ "$CLEAN_LOCAL_TRACER_LOGS" != "NO" ]; then
        rm -f $MCU_HOME_DIR/tmp/startup_logs/*.Ind
fi
Scripts/CleanSharedResources.sh > /dev/null



