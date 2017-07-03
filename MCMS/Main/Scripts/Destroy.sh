#!/bin/sh

export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin
PROCESSES="\
          McmsDaemon \
          Configurator \
          Logger \
          Auditor \
          Faults \
          IPMCInterface \
          SNMPProcess \
          CSMngr \
          CertMngr \
          McuMngr \
          ConfParty \
          Cards \
          CDR \
          Resource \
          SipProxy \
		  Ice \
          DNSAgent \
          Gatekeeper \
          QAAPI \
          ExchangeModule \
          EncryptionKeyServer \
          Authentication \
          Installer \
          RtmIsdnMngr \
          BackupRestore \
          MplApi \
          CSApi \
          Collector \
          SystemMonitoring \
          MediaMngr \
          Failover \
          LdapModule \
          Utility \
          MCCFMngr \
          ApachModule \
          GideonSim \
          EndpointsSim \
          McuCmd \
          ClientLogger \
          Diagnostics \
          NotificationMngr \
          "

echo Wait 10 seconds until MCMS is destroyed.
#kobi&shimon pgrep -x did not work -> hence Gesher stop used many other trials to kill the Mcms
pgrep McmsDaemon && $MCU_HOME_DIR/mcms/Bin/McuCmd kill McmsDaemon && sleep 10

#This closure was mooved into the McmsDaemon Code to have aneaty closure of those 2 proccesses during Simulation Closure
#$MCU_HOME_DIR/mcms/Bin/McuCmd kill EndpointsSim 
#$MCU_HOME_DIR/mcms/Bin/McuCmd kill GideonSim 
#sleep 4

# Limits process names to the 15 characters.
for p in $PROCESSES
do
  procId=`ps -o pid -C  $p | tail -n 1`
  if [[ $procId != "" ]];then
        if [[ $procId != "  PID" ]]; then
          echo "$procId is still alive, finish him." && kill -9 $procId 2>&1 &
        fi
  fi
done

wait

killall -9 -q snmpd
killall -9 -q valgrind 
killall -9 -q httpd

# Next line is for Single Apache
sudo /usr/bin/pkill httpd

sleep 1

for p in $PROCESSES
do
  pgrep -x ${p:0:15} && echo "Failed to kill $p."
done

exit 0
