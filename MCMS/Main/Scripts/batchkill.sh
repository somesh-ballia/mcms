#!/bin/sh
#export PROCESSES=`ls Processes/ | grep -v '\.'`
export PROCESSES='McmsDaemon ApacheModule Auditor  Authentication BackupRestore CDR CSApi CSMngr Cards CertMngr Collector ConfParty Configurator DNSAgent EncryptionKeySe ExchangeModule Failover Faults Gatekeeper LdapModule Logger McuMngr MplApi NotificationMngr QAAPI Resource RtmIsdnMngr SNMPProcess SipProxy Ice SystemMonitorin Utility GideonSim EndpointsSim Installer IPMCInterface MCCFMngr LicenseServer'

export LD_LIBRARY_PATH=$MCU_HOME_DIR/mcms/Bin
export SASL_PATH=$MCU_HOME_DIR/mcms/Bin

echo "$0 : `date +"%Y:%m:%d :%H:%M:%S"` : `hostname -s` : `whoami`"
    
# ./Bin/httpd -f $MCU_HOME_DIR/mcms/StaticCfg/httpd.conf.sim -k stop
# Next if was added as part of the Single Apache
if [ `whoami` == 'root' ]; then
	pkill httpd
else
	sudo pkill httpd
fi
 
for process in $PROCESSES
do
  #first kill 'softly'
  pid=`pgrep "^\$process$"`
  if [[ $pid != "" ]];then
  	  kill $pid
	  #wait 5 seconds and force kill if needed
	  (
	   sleep 5
	   kill -18 $pid
	   kill -9 $pid
	  ) &> /dev/null &
  fi
done


killall -q -9 menu
killall -q -9 ManageMcuMenu
killall -q -9 Start.sh
if [ `whoami` == 'root' ]; then
	killall -q -9 httpd lt-httpd
else
	sudo killall -q -9 httpd lt-httpd
fi
killall -q -9 valgrind

for KILLPID in `ps ax | grep httpd | grep valgrind | grep ApacheModule | grep -v grep | awk ' { print $1;}'`; do
  kill -9 $KILLPID;
done

# Next if was added as part of the Single Apache
if [ `whoami` == 'root' ]; then
	killall -q -9 snmpd
else
	sudo killall -q snmpd
fi

exit 0
