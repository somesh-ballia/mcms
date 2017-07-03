#!/bin/sh

$MCU_HOME_DIR/mcms/Bin/McuCmd reset McmsDaemon > /dev/null
killall -q httpd lt-httpd `ls $MCU_HOME_DIR/cs/bin` 2> /dev/null
sleep 4
killall -9 httpd lt-httpd `ls $MCU_HOME_DIR/mcms/Bin` `ls $MCU_HOME_DIR/cs/bin` 2> /dev/null


