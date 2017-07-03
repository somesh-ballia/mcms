#!/bin/bash

make active

export REMOTE_TARGET=$1
export PROCESS_NAME=$2

export REMOTE="./plink POLYCOM@$REMOTE_TARGET -pw POLYCOM"
$REMOTE "cat /version.txt"

export PROCESS_PID=`$REMOTE pidof $PROCESS_NAME`
export GDB_SERVER_PORT=10002

echo $PROCESS_NAME" pid is : "$PROCESS_PID

$REMOTE "killall gdbserver"
$REMOTE "gdbserver 127.0.0.1:"$GDB_SERVER_PORT"  --attach "$PROCESS_PID &

sleep 3


mv -f ~/.gdbinit ~/.gdbinit_old >> /dev/null


(echo "set solib-absolute-prefix /dev/null"
echo "set solib-search-path /opt/polycom/carmel/tool_chain/v6/gcc-3.4.2-glibc-2.3.5/i686-polycom-linux-gnu/i686-polycom-linux-gnu/lib:/opt/polycom/carmel/tool_chain/v6/i686-polycom-linux-gnu/lib:$MCU_HOME_DIR/mcms/Bin"
echo "directory $MCU_HOME_DIR/mcms/Libs/ProcessBase/"
echo "directory $MCU_HOME_DIR/mcms/Libs/SimLinux/"
echo "directory $MCU_HOME_DIR/mcms/Libs/XmlPars/"
echo "directory $MCU_HOME_DIR/mcms/Libs/Common/"
echo "directory $MCU_HOME_DIR/mcms/Processes/"
echo "directory $MCU_HOME_DIR/mcms/Processes/$PROCESS_NAME"
echo "directory $MCU_HOME_DIR/mcms/Processes/"$PROCESS_NAME"/"$PROCESS_NAME"Lib"
echo "handle SIGSTOP nopass"
#echo "target remote "$REMOTE_TARGET:$GDB_SERVER_PORT) >> ~/.gdbinit
echo "define mcms"
echo "target remote "$REMOTE_TARGET:$GDB_SERVER_PORT
echo "end")>> ~/.gdbinit

echo "Opening DDD GUI - please type in DDD console \"mcms\" to attach to remote process"

ddd --debugger /opt/polycom/carmel/tool_chain/v5/bin/i686-polycom-linux-gnu-gdb ./Bin/$PROCESS_NAME

rm -f ~/.gdbinit
mv -f ~/.gdbinit_old ~/.gdbinit >> /dev/null
