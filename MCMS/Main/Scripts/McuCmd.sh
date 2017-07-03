#!/bin/sh
export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin 
export SASL_PATH=$MCU_HOME_DIR/mcms/Bin
$MCU_HOME_DIR/mcms/Bin/McuCmd $*
