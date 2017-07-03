# Install.sh

APACHEMODULE=mcms/Bin/ApacheModule
SNMPD=mcms/Bin/snmpd
SNMPDJ=mcms/Bin/snmpdj
#STRIP=/opt/polycom/OSELAS.Toolchain-1.99.2/i686-unknown-linux-gnu/gcc-4.1.2-glibc-2.5-binutils-2.17-kernel-2.6.18/bin/i686-unknown-linux-gnu-strip
STRIP=/opt/polycom/sim_pack/OSELAS.Toolchain-1.99.3/i686-unknown-linux-gnu/gcc-4.3.2-glibc-2.8-binutils-2.18-kernel-2.6.24/bin/i686-unknown-linux-gnu-strip

echo "Copy MCMS files..."
rm -Rf mcms
mkdir -p mcms
mkdir -p mcms/Bin
mkdir -p mcms/StaticCfg
mkdir -p mcms/IVRX/RollCall
mkdir -p mcms/Root
mkdir -p mcms/Scripts

chown -R mcms:mcms $MCU_HOME_DIR/mcms

echo "Creating Diagnostics virtual folders..."
mkdir -p mcms/Diagnostics

ln -sf $MCU_HOME_DIR/output/log mcms/Diagnostics/logs
ln -sf $MCU_HOME_DIR/output/core mcms/Diagnostics/cores
ln -sf $MCU_HOME_DIR/output/faults mcms/Diagnostics/faults
ln -sf $MCU_HOME_DIR/output/cslogs mcms/Diagnostics/cs
ln -sf $MCU_HOME_DIR/output/mylogs mcms/Diagnostics/os
ln -sf $MCU_HOME_DIR/output/apache mcms/Diagnostics/apache
ln -sf $MCU_HOME_DIR/output/media_rec mcms/Diagnostics/MediaRecording
ln -sf $MCU_HOME_DIR/output/log  mcms/Root/LogFiles
ln -sf $MCU_HOME_DIR/output/cdr mcms/Root/CdrFiles
ln -sf $MCU_HOME_DIR/output/media_rec mcms/MediaRecording
ln -sf $MCU_HOME_DIR/output/nids mcms/Root/NIDS
ln -sf $MCU_HOME_DIR/output/links mcms/Links
ln -sf $MCU_HOME_DIR/mcms/Diagnostics mcms/Root/Diagnostics
ln -sf $MCU_HOME_DIR/mcms/MIBS mcms/Root/MIBS
ln -sf $MCU_HOME_DIR/data/DebugBin mcms/DebugBin
ln -sf $MCU_HOME_DIR/config/JITC_MODE.txt mcms/JITC_MODE.txt
ln -sf $MCU_HOME_DIR/config/JITC_MODE_FIRST_RUN.txt mcms/JITC_MODE_FIRST_RUN.txt
ln -sf $MCU_HOME_DIR/config/SEPARATE_MANAGMENT_NETWORK.txt mcms/SEPARATE_MANAGMENT_NETWORK.txt
ln -sf $MCU_HOME_DIR/config/ENABLE_ACCEPTING_ICMP_REDIRECT.txt mcms/ENABLE_ACCEPTING_ICMP_REDIRECT.txt
ln -sf $MCU_HOME_DIR/output/message_files mcms/MessageFiles
ln -sf $MCU_HOME_DIR/config/ENABLE_BONDING.txt mcms/ENABLE_BONDING.txt

if [ "$NPG" = 'true' ]
then
  touch mcms/ProductType
  echo -n 'NPG2000' > mcms/ProductType
else if [ "$MEDIA_MANAGER" = 'true' ]
then
  rm mcms/ProductType
  echo -n 'CALL_GENERATOR' > mcms/ProductType
  echo "Copy Media files - MediaManager"
  mkdir mcms/Media
  cp -R $SOURCE_MEDIA_DIR/* mcms/Media
else
  ln -sf $MCU_HOME_DIR/config/product mcms/ProductType
fi
fi

echo "Copy MCMS bins..."

cp -R $SOURCE_MCMS_PROJECT/Bin.i32ptx/* mcms/Bin
find mcms/Bin -type f | grep -v lib | grep -v snmpd | grep -v ApacheModule | grep -v httpd | grep -v openssl | (
  while read bin; do echo "Stripping "$(basename $bin)"..."; $STRIP $bin; done
)
find mcms/Bin -type f | grep lib | (
  while read bin; do echo "Stripping "$(basename $bin)"..."; $STRIP $bin; done
)


echo "Copy utilities..."
cp -Rf /opt/polycom/sim_pack/mcms_100.0_v016/util/* mcms/Bin

rm -f mcms/Bin/*.Test
rm -f mcms/Bin/TestClient
rm -f mcms/Bin/Demo
rm -f mcms/Bin/EndpointsSim

rm -f mcms/Bin/NotificationMngr
rm -f mcms/Bin/libMocks.so
rm -f mcms/Bin/libnetsnmp*
rm -f mcms/Bin/libcryp*
rm -f mcms/Bin/libssl*

if [ "$MEDIA_MANAGER" = 'true' ]
then
   rm -f mcms/Bin/SyncTime
   rm -f mcms/Bin/Diagnostics
   rm -f mcms/Bin/IPMCInterface
else
   rm -f mcms/Bin/GideonSim
fi

cp -f $SOURCE_MCMS_PROJECT/Scripts/ValgrindSupTarget.txt lib/valgrind/default.supp

# The lines regarding Apachemodule are made for the difference in location
# between the Pizza and the Target.
rm -f $APACHEMODULE

echo "#!/bin/sh" > $APACHEMODULE
echo "" >> $APACHEMODULE
echo "CMD=/usr/sbin/httpd" >> $APACHEMODULE
echo "CFG=$MCU_HOME_DIR/mcms/StaticCfg/httpd.conf" >> $APACHEMODULE
echo "LOG=$MCU_HOME_DIR/tmp/startup_logs/DaemonProcessesLoad.log" >> $APACHEMODULE
echo "" >> $APACHEMODULE 
echo "sleep 1" >> $APACHEMODULE
echo "\$CMD -f \$CFG 2>&1 | tee -a \$LOG" >> $APACHEMODULE
echo "while true; do" >> $APACHEMODULE
echo "  sleep 10" >> $APACHEMODULE
echo "  ps -e | grep -v grep | grep httpd >/dev/null || \$CMD -f \$CFG 2>&1 | tee -a \$LOG" >> $APACHEMODULE
echo "done" >> $APACHEMODULE

chmod +x $APACHEMODULE

echo "Copy ActionRedirection File..."
cp -R $SOURCE_MCMS_PROJECT/StaticCfg/* mcms/StaticCfg/

chmod a-w mcms/Bin/*


echo "Copy MCMS scripts..."
cp $SOURCE_MCMS_PROJECT/Scripts/*.sh mcms/Scripts
cp $SOURCE_MCMS_PROJECT/Scripts/*.txt mcms/Scripts
cp -dpR $SOURCE_MCMS_PROJECT/Scripts/TLS mcms/Scripts

echo "Copy MCMS MIBS..."
cp -R $SOURCE_MCMS_PROJECT/MIBS mcms

echo "Copy MCMS Versions.xml.."
cp -f $SOURCE_MCMS_PROJECT/Versions.xml mcms/

echo "Copy Default IVR folder..."
cp -Rf $SOURCE_MCMS_PROJECT/VersionCfg/IVR mcms/StaticCfg

echo "Copy Audible Alarms folder..."
cp -Rf $SOURCE_MCMS_PROJECT/VersionCfg/AudibleAlarms mcms/StaticCfg

echo "Create MCMS links..."
ln -sf $MCU_HOME_DIR/config/mcms mcms/Cfg
ln -sf $MCU_HOME_DIR/config/states mcms/States
ln -sf $MCU_HOME_DIR/output/external_ivr mcms/External
ln -sf $MCU_HOME_DIR/output/log mcms/LogFiles
ln -sf $MCU_HOME_DIR/output/audit mcms/Audit
ln -sf $MCU_HOME_DIR/output/core mcms/Cores
ln -sf $MCU_HOME_DIR/output/tcp_dump/emb mcms/TcpDumpEmb
ln -sf $MCU_HOME_DIR/output/tcp_dump/mcms mcms/TcpDumpMcms
ln -sf $MCU_HOME_DIR/output/cdr mcms/CdrFiles
ln -sf $MCU_HOME_DIR/output/faults mcms/Faults
ln -sf $MCU_HOME_DIR/output/nids mcms/NIDS
ln -sf $MCU_HOME_DIR/data/new_version mcms/Install
ln -sf $MCU_HOME_DIR/output/backup mcms/Backup
ln -sf $MCU_HOME_DIR/data/restore mcms/Restore
ln -sf $MCU_HOME_DIR/output/links mcms/Links
ln -sf $MCU_HOME_DIR/output/media_rec mcms/MediaRecording
ln -sf $MCU_HOME_DIR/config/keys mcms/Keys
ln -sf $MCU_HOME_DIR/config/keys/cs mcms/KeysForCS
ln -sf $MCU_HOME_DIR/config/keys/ca_cert mcms/CACert
ln -sf $MCU_HOME_DIR/config/keys/ca_cert/crl mcms/CRL
ln -sf $MCU_HOME_DIR/config/static_states mcms/StaticStates
ln -sf $MCU_HOME_DIR/config/ocs mcms/OCS
ln -sf $MCU_HOME_DIR/output/802_1x/emb mcms/802_1xEmb
ln -sf $MCU_HOME_DIR/output/local_tracer mcms/LocalTracer

chown mcms:mcms mcms/Backup
chown mcms:mcms mcms/Restore
chown mcms:mcms mcms/Install

# The following lines are for running the syslogd
# on the HardDrive instead of in the RAM
rmdir var/log/mcms
rmdir var/log
ln -sf $MCU_HOME_DIR/output/mylogs var/log  


echo "Creating links to snmpd snmpdj..."
rm -f $SNMPD
rm -f $SNMPDJ
echo "#!/bin/sh" > $SNMPD
echo "#!/bin/sh" > $SNMPDJ
echo "umask 111" >> $SNMPD
echo "umask 111" >> $SNMPDJ
echo "rm $MCU_HOME_DIR/tmp/netsnmpfipsflag" >> $SNMPD
echo "/sbin/snmpd -C -c $MCU_HOME_DIR/mcms/Cfg/snmpd.conf,$MCU_HOME_DIR/tmp/snmpd.conf > /dev/null 2> /dev/null " >> $SNMPD
echo "touch $MCU_HOME_DIR/tmp/netsnmpfipsflag" >> $SNMPDJ
echo "/sbin/snmpd -F -C -c $MCU_HOME_DIR/mcms/Cfg/snmpd.conf,$MCU_HOME_DIR/tmp/snmpd.conf > /dev/null 2> /dev/null " >> $SNMPDJ
chmod +x $SNMPD
chmod +x $SNMPDJ

echo "Set file permissions..."

# VNGR-17277 stig: The inetd.conf file is more permissive than 440
chmod o-rwx,ug-wx etc/inetd.conf

# VNGR-17265 stig: The root account home directory (other than '/') is more permissive than 700
chmod go-rwx home

# VNGR-17447 stig: Linux /boot/grub/grub.conf file is more permissive than 600 
chmod go-rwx boot/grub/menu.lst

#Assert	On $MCU_HOME_DIR/mcms/TLS folder due	to permission failure on this folder for mcms User
#chmod 776 $MCU_HOME_DIR/mcms/Scripts/TLS


chmod -R o-w $MCU_HOME_DIR/config
chown -R mcms:mcms $MCU_HOME_DIR/config/states
chown -R mcms:mcms $MCU_HOME_DIR/config/static_states
chmod -R o-w output

# we refresh /etc/exports list VNGR-22799 
#exportfs -a
