#!/bin/sh
#usage: Scripts/ddd_core.sh PROCESS_NAME CORE_DUMP (Only from target)

PROCESS=$(basename $1)
ln -sf $PWD $MCU_HOME_DIR/tmp/mcms

mv -f ~/.gdbinit ~/.gdbinit_old >> /dev/null

(echo "set solib-absolute-prefix /dev/null"
echo "set solib-search-path /opt/polycom/OSELAS.Toolchain-1.99.3_64bit/OSELAS.Toolchain-1.99.3/i686-unknown-linux-gnu/gcc-4.3.2-glibc-2.8-binutils-2.18-kernel-2.6.24/lib:/opt/polycom/OSELAS.Toolchain-1.99.3_64bit/OSELAS.Toolchain-1.99.3/i686-unknown-linux-gnu/gcc-4.3.2-glibc-2.8-binutils-2.18-kernel-2.6.24/i686-unknown-linux-gnu/lib:/opt/polycom/OSELAS.Toolchain-1.99.3_64bit/OSELAS.Toolchain-1.99.3/i686-unknown-linux-gnu/gcc-4.3.2-glibc-2.8-binutils-2.18-kernel-2.6.24/sysroot-i686-unknown-linux-gnu/lib:$MCU_HOME_DIR/mcms/Bin"

echo "directory $MCU_HOME_DIR/mcms/Libs/ProcessBase/"
echo "directory $MCU_HOME_DIR/mcms/Libs/SimLinux/"
echo "directory $MCU_HOME_DIR/mcms/Libs/XmlPars/"
echo "directory $MCU_HOME_DIR/mcms/Libs/Common/"
echo "directory $MCU_HOME_DIR/mcms/Processes/"
echo "directory $MCU_HOME_DIR/mcms/Processes/"$PROCESS
echo "directory $MCU_HOME_DIR/mcms/Processes/"$PROCESS"/"$PROCESS"Lib") >> ~/.gdbinit

#ddd --debugger /opt/polycom/OSELAS.Toolchain-1.99.2/i686-unknown-linux-gnu/gcc-4.1.2-glibc-2.5-binutils-2.17-kernel-2.6.18/bin/i686-unknown-linux-gnu-gdb Bin/$PROCESS $2


ddd --debugger /opt/polycom/OSELAS.Toolchain-1.99.3_64bit/OSELAS.Toolchain-1.99.3/i686-unknown-linux-gnu/gcc-4.3.2-glibc-2.8-binutils-2.18-kernel-2.6.24/bin/i686-unknown-linux-gnu-gdb Bin/$PROCESS $2



echo "Don't forget to run make active on your simulation now!"
rm -f ~/.gdbinit
mv -f ~/.gdbinit_old ~/.gdbinit >> /dev/null


