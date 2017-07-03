#!/bin/sh


# help
if [ "$1" = "--help" ]
then
        echo "   Start to run oprofiler tool. Usage:"
        echo "   [MCU_HOME_DIR]/Scripts/OprofStart.sh                     - count all processes events"
        echo "   [MCU_HOME_DIR]/Scripts/OprofStart.sh [mcms_process_name] - filter specific process events"
        exit 0
fi

OPROF_COMMAND="opcontrol"
# permission elevation - root for RMX
USER=`whoami`

#if [ "`whoami`" != "root" ]; then
if [ $USER != "root" ]; then
        if [ $USER == "rt3p1aa" ] || [ $USER == "mcms" ]; then
                echo "[mcms]> Root permissions required."
                exit 1
        else
                OPROF_COMMAND="sudo opcontrol"
        fi
fi

PROCESS_NAME="all"
if [ "$1" ]; then

        # find executable by 'param':
        # 1. 'param' is absolute path
        # 2. 'param' is file in [MCU_HOME]/mcms/Links
        # 3. 'param' is file in [MCU_HOME]/mcms/Bin

        PARAM="$1"
        if test -e $PARAM; then
                PROCESS_NAME=$PARAM
        elif test -e $MCU_HOME_DIR/mcms/Links/$1; then
                PROCESS_NAME=$MCU_HOME_DIR/mcms/Links/$1
        elif test -e $MCU_HOME_DIR/mcms/Bin/$1; then
                PROCESS_NAME=$MCU_HOME_DIR/mcms/Bin/$1
        else
                echo "[mcms]> ERROR: Process with name '$PARAM' not found. Try again."
                exit 0
        fi
        echo "[mcms]> Starting oprofiler for '$PROCESS_NAME' process..."
else
        echo "[mcms]> Starting oprofiler for all processes..."
fi

#OPROF_DIR=/output/oprofiler
OPROF_DIR=$MCU_HOME_DIR/tmp/oprofiler

mkdir -p $OPROF_DIR

$OPROF_COMMAND --deinit
$OPROF_COMMAND --no-vmlinux
$OPROF_COMMAND --separate=kernel
$OPROF_COMMAND --event=default
$OPROF_COMMAND --session-dir=$OPROF_DIR
$OPROF_COMMAND --init
$OPROF_COMMAND --reset
$OPROF_COMMAND --callgraph=0 --image=$PROCESS_NAME
$OPROF_COMMAND --start

echo "[mcms]> oprofiler running for $PROCESS_NAME, session dir is $OPROF_DIR"
echo "[mcms]> Use $MCU_HOME_DIR/mcms/Scripts/OprofStop.sh to stop oprofiler"

exit 0





