#!/bin/sh

Scripts/batchkill.sh
rm -fR $MCU_HOME_DIR/tmp/queue
rm -fR $MCU_HOME_DIR/tmp/shared_memory
rm -fR $MCU_HOME_DIR/tmp/802_1xCtrl
mkdir $MCU_HOME_DIR/tmp/queue
mkdir $MCU_HOME_DIR/tmp/shared_memory
mkdir $MCU_HOME_DIR/tmp/802_1xCtrl
#rm -f $MCU_HOME_DIR/tmp/loglog.txt
Scripts/CleanSharedResources.sh > /dev/null

#remove old ip services
#rm -f Cfg/IPServiceList.xml


