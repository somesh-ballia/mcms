#!/bin/sh


DAN=`ps -e | grep $1 | head -1 | awk '{print $1}'`
echo $DAN
gdbserver 127.0.0.1:9999 --attach $DAN