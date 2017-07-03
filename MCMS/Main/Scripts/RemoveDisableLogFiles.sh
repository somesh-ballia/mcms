#!/bin/sh

Bin/McuCmd ping Logger < /dev/null
Bin/McuCmd stop_file_sys Logger < /dev/null

echo Waiting 2 seconds until file system will be disable 
sleep 2

rm LogFiles/*
