#!/bin/sh

export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin
export SASL_PATH=$MCU_HOME_DIR/mcms/Bin

Bin/McuCmd ping Logger < /dev/null
Bin/McuCmd flush Logger < /dev/null

echo Waiting 2 seconds until flush ends
sleep 2

Bin/LogUtil LogFiles/*.log




