#!/bin/sh


# permission elevation
if [ "`whoami`" != "root" ]; then
	echo "Root permissions please."
	exit 0
fi

killall Start.sh
$MCU_HOME_DIR/mcms/Bin/McuCmd kill all
rm -Rf $MCU_HOME_DIR/mcms/Links/*

echo -n "Waiting until all valgrind processes will finish..."

while (ps | grep valgrind)>/dev/null ; do sleep 1; echo -n ".";  done

echo

cd $MCU_HOME_DIR/output/tmp

find -name "*mem.*" | xargs grep "ERROR SUMMARY"
find -name "*mem.*" | xargs grep -A3 "LEAK SUMMARY"


echo "When you finish taking the result files from $MCU_HOME_DIR/output/tmp,"
echo "please reset the system (hard / reboot)"



