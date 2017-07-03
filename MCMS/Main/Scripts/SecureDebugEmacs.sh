#!/bin/sh


if [ "$1" == "" ]
then
    echo "usage: SecureDebugEmacs.sh USER@TARGET_IP MCMS_PROCESS_NAME [REMOTE_CORE_FILE]"
    echo "when core file name is not specified, the debugger will attach the running process"
    echo "type only the base name of the core file (wildcards allowed.)"
    exit 1
fi
 


rm -Rf $MCU_HOME_DIR/tmp/dir.txt $MCU_HOME_DIR/tmp/files.txt $MCU_HOME_DIR/tmp/empty
export CUR=$PWD
find -type d > $MCU_HOME_DIR/tmp/dir.txt
find -type f -iname "*.c*" > $MCU_HOME_DIR/tmp/files.txt
find -type f -iname "*.h" >> $MCU_HOME_DIR/tmp/files.txt
mkdir -p $MCU_HOME_DIR/tmp/empty
cd $MCU_HOME_DIR/tmp/empty
cat $MCU_HOME_DIR/tmp/dir.txt | xargs mkdir -p

cat $MCU_HOME_DIR/tmp/files.txt | (

    read line;
    while [ "$line" != "" ];
      do 
      sed  s/^.*// <  $CUR/$line > $MCU_HOME_DIR/tmp/empty/$line
      read line;
    done
)

touch $MCU_HOME_DIR/tmp/empty/.gdbinit

cat $MCU_HOME_DIR/tmp/dir.txt | (

    read line;
    while [ "$line" != "" ];
      do 
      echo "directory "$line >> $MCU_HOME_DIR/tmp/empty/.gdbinit
      read line;
    done
)

scp -rq $MCU_HOME_DIR/tmp/empty $MCU_HOME_DIR/tmp/empty/.gdbinit $1:$MCU_HOME_DIR/tmp

rm -Rf $MCU_HOME_DIR/tmp/dir.txt $MCU_HOME_DIR/tmp/files.txt $MCU_HOME_DIR/tmp/empty

ln -sf $CUR $MCU_HOME_DIR/tmp/empty
cd $MCU_HOME_DIR/tmp/empty


if [ "$3" != "" ]
then
    echo "remote debugging core file "$3" of process "$2" on " $1
    echo "(progn (gdb \"ssh "$1 "cd $MCU_HOME_DIR/tmp/empty ; $MCU_HOME_DIR/mcms/Bin/McuCmd set all WATCH_DOG NO ; gdb $MCU_HOME_DIR/mcms/Bin/"$2 "$MCU_HOME_DIR/output/core/$3 \"))" > $MCU_HOME_DIR/tmp/run-gud.el;
else
    echo "attaching remote process "$2" on " $1
    echo "(progn (gdb \"ssh "$1 "cd $MCU_HOME_DIR/tmp/empty ; $MCU_HOME_DIR/mcms/Bin/McuCmd set all WATCH_DOG NO ; gdb $MCU_HOME_DIR/mcms/Bin/"$2 "\`pidof "$2"\`\"))" > $MCU_HOME_DIR/tmp/run-gud.el;
fi

emacs -l $MCU_HOME_DIR/tmp/run-gud.el




