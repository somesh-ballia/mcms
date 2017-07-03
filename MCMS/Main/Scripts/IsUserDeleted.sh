#!/bin/sh

ISDELETED=`sed -n "/^$1:!/p" /etc/shadow`
if [ "$ISDELETED" = "" ]
then
    exit 0;
else
    echo "User has not been deleted";
    exit 1;
fi

