#!/bin/sh


# help
if [ "$1" = "--help" ]
then
        echo "   Stopping oprofiler tool."
        exit 0
fi

OPROF_COMMAND="opcontrol"
# permission elevation - root for RMX
USER=`whoami`

if [ $USER != "root" ]; then
        if [ $USER == "rt3p1aa" ] || [ $USER == "mcms" ]; then
                echo "[mcms]> Root permissions required."
                exit 1
        else
                OPROF_COMMAND="sudo opcontrol"
        fi
fi

$OPROF_COMMAND --stop
$OPROF_COMMAND --dump
$OPROF_COMMAND --shutdown

PROCESS_NAME=`$OPROF_COMMAND --status | grep Image | awk '{print $3}'`
OPROF_DIR=$MCU_HOME_DIR/tmp/oprofiler

echo "[mcms]> Profiler stopped"
echo -n "[mcms]> Use command to review oprofiler results: opreport --session-dir=$OPROF_DIR"
if [ "$PROCESS_NAME" != "none" ]; then
        echo -n " -l $PROCESS_NAME"
else
        echo -n " -l $MCU_HOME_DIR/mcms/Bin/[mcms_process_name]"
fi
echo

exit 0




