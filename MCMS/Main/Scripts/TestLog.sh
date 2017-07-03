#!/bin/sh

if grep "`echo $2`"  $MCU_HOME_DIR/tmp/loglog.txt > /dev/null
then
    echo "DONE"
else
    echo "FAILED"
    Scripts/ErrorMsg.sh $1 Test Failed
    exit 101
fi

exit 0
