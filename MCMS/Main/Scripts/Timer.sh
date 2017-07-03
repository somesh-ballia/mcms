#!/bin/sh
echo Starting $1 seconds timer for this test
export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin
export SASL_PATH=$MCU_HOME_DIR/mcms/Bin

sleep $1

echo TIMEOUT !!!
Scripts/batchkill.sh
killall Startup.sh

