#!/bin/sh

grep "* EXCEPTION *" $MCU_HOME_DIR/tmp/loglog.txt > /dev/null || exit 0
usleep 10000
killall tail
usleep 10000
Scripts/batchkill.sh

#echo -e '\E[37;31m'"\033[1m
echo             @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
echo                  Test FAILED - EXCEPTION
echo             @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#\033[0m"
exit 999