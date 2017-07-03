#!/bin/bash

rm -f Scripts/Integration/IntegTest.xml

CSIPVAR=$1
MFAIPVAR=$2

if [ -n "$CSIPVAR" ]  &&  [ -n "$MFAIPVAR" ]
then
   echo `sed s/CSIP/$CSIPVAR/g Scripts/Integration/IntegrationAddIpService.xml` | sed s/MFAIP/$MFAIPVAR/g  > Scripts/Integration/IntegTest.xml
else
    echo "usage: ChangeAddIpXml.sh CS_IP MFAIP"
    echo "example:  ChangeAddIpXml.sh 172.22.192.15 172.22.192.46"
fi


