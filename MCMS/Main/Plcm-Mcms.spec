##################################################
# Plcm-Mcms.spec                               #
# ------------------                             #
# This is the Plcm-Mcms RPM build spec file.     #
# Written by: Judith Maman                       #
#             judith.shuva-maman@polycom.co.il   #
# Date:       19/03/2012                         #
##################################################

##################################################
# Macros                                         #
# The default macros are kept in                 #
# /usr/lib/rpm/macros. New macros can be set.    #
##################################################
# Setting the user's home directory
%define _homedir %(echo `pwd`)
%define _package %(echo $Filename)
#______________ Update above here ________________

%define _mcuHomeDir /opt/mcu
%define _topdir %{_homedir}/rpmbuild
%define _builddir %{_topdir}/BUILD
%define _buildroot %{_topdir}/BUILDROOT

# For all unpackaged files
%define _unpackaged_files_terminate_build 0

##################################################
# Information and environment                    #
# To be observed by:                             #
# 1. Before installation: rpm -qif package_file  #
# 2. After installation: rpm -qi package_name    #
##################################################
Summary:  Plcm-Mcms RPM build 
#______________ Update above here ________________

Name:   Plcm-Mcms
#______________ Update above here ________________

Version:  7.6
#______________ Update above here ________________

Release:  0
#______________ Update above here ________________

Vendor:   Polycom
Group:    System Environment/Devel
License:  GPL
Source:   %{_package}.tar.gz
#______________ Update above here ________________
Prefix:		%{_mcuHomeDir}
BuildRoot:  %{_topdir}/BUILD

BuildRequires:  make

AutoReqProv:  no

Requires: sudo
Requires: glibc%{?_isa}
Requires: /lib/libgcc_s.so.1
Requires: /usr/lib/libxml2.so.2
#Requires:  openssl%{?_isa}     #Different rpm name and lib path in Suse/RHEL. Leaving our lib in for now.
Requires: zlib%{?_isa}
Requires: libstdc++%{?_isa} 
#Requires: openldap%{?_isa}
#Requires:  /usr/lib/libnetsnmp.so.*  #Different rpm name and lib path in Suse/RHEL. Leaving our lib in for now.
#Requires:  cyrus-sasl%{?_isa}
#Requires:  liblber-2.4.so.2                #No Ldap currently needed for MFW
#Requires:  libldap-2.4.so.2    #No Ldap currently needed for MFW
#Requires:  expat%{?_isa}     #Different rpm name and lib path in Suse/RHEL. Leaving our lib in for now.
#Requires:  libcurl%{?_isa}     #Different rpm name and lib path in Suse/RHEL. Leaving our lib in for now.

# Added by Shachar Bar - to solve GLIBC_PRIVATE dependencies
Provides: ld-linux.so.2(GLIBC_PRIVATE)
Provides: libc.so.6(GLIBC_PRIVATE)
Provides: libdb-4.4.so
Provides: %{_mcuHomeDir}/mcms/python/bin/python2.4
Provides: %{_mcuHomeDir}/mcms/python/bin/python
Provides: libcrypto.so.1.0.1
Provides: libssl.so.1.0.1
Provides: libexpat.so.0

%description
This is the Plcm-Mcms package.
#______________ Update above here ________________

##################################################
# Preparing for build                            #
##################################################
%prep
# Make sure the rest of the RPM Build environment is met
echo "In prep - $PWD"

tar zxf ../SOURCES/${Filename}.tar.gz
# BRIDGE-17589
rm -f Main/Scripts/run_cs.sh
rm -f Main/Bin*/NTP_Bypass_SoftMCU_Server
# Creating a dummy configure file - this is rpmbuild legacy
touch ../BUILD/configure
chmod +x ../BUILD/configure
echo "Prep phase is finished"

%pre
mkdir -p /tmp/startup_logs
#Check if already installed
RPM_NAME=`rpm -qa --queryformat "%{NAME} \\n" | grep 'ibm-sametime-vmcu-9.0.0.0-Mcms\|Plcm-Mcms' 2>/dev/null`
if [ $? -eq 0 ] ; then
    RPM_INST=`rpm -q --queryformat '%{INSTPREFIXES}\n' $RPM_NAME`
    if [[ "$RPM_INSTALL_PREFIX" != "$RPM_INST" ]] ; then
		echo "ERROR: VMCU is already installed in a different folder."
		exit -1
	fi
fi
#Installation validator test
if [ -f /tmp/InstallValidator.sh ];then
  /tmp/InstallValidator.sh pre &> /tmp/startup_logs/InstallValidator_pre.log
  if [[ $? != 0 ]];then
    echo "ERROR: refer to '/tmp/startup_logs/InstallValidator_pre.log' for more info."
    exit -1
  fi
else
  echo "InstallValidator.sh does not exists" > /tmp/startup_logs/InstallValidator_pre.log
fi

#Save the list of directories created by the installation in order to remove them on uninstall
INSTALL_INFO=/tmp/mcu_install.info
rm -rf $INSTALL_INFO &>/dev/null
mkdir -p -v $RPM_INSTALL_PREFIX >> $INSTALL_INFO

#Disk space test
#DISK_SIZE=`df -m $RPM_INSTALL_PREFIX | tail -n -1 | tr -s ' ' | cut -d' ' -f4`
#if [ $DISK_SIZE -lt 20000 ] ; then
#    echo "There is not enough space on device for MCU installation - available: [$DISK_SIZE] required: [20000]" >> /tmp/startup_logs/InstallValidator_pre.log
#    echo "ERROR: refer to '/tmp/startup_logs/InstallValidator_pre.log' for more info."
    #remove installation directory
#    sed -i "s|mkdir: created directory \`|rm -rf \'|g" $INSTALL_INFO
#    chmod 777 $INSTALL_INFO
#    $INSTALL_INFO
#    exit -1
#fi

mkdir -p -v $RPM_INSTALL_PREFIX/tmp/startup_logs >> $INSTALL_INFO
cp -f /tmp/startup_logs/InstallValidator_pre.log $RPM_INSTALL_PREFIX/tmp/startup_logs &>/dev/null

##################################################
# Building the binaries                          #
##################################################
%build
# Calling the built-in Makefile to produce the project
echo "In build phase - Current dir: $PWD"

##################################################
# Creating a valid Makefile, if one is not       #
# supplied. This is a dummy stage, as we don't   #
# use any Makefile.                              #
##################################################
%configure
echo "Passing the configure phase"

##################################################
# Installing the new package files               #
##################################################
%install
# Removing the dummy configure file to prevent unpacked file warning
echo "Install is in: $PWD"
rm -f ../BUILD/configure
echo "Add the creation of the build folders"

DIRS="\
 mcms\
 mcms/Bin\
 mcms/StaticCfg\
 mcms/Scripts\
 mcms/VersionCfg\
 mcms/Cfg\
 mcms/Root\
 mcms/IVRX/RollCall\
 mcms/Diagnostics\
 mcms/TS\
 config\
 config/mcms\
 config/states\
 config/static_states\
 config/ocs/cs1\
 config/ocs/cs2\
 config/ocs/cs3\
 config/ocs/cs4\
 config/ocs/cs5\
 config/ocs/cs6\
 config/ocs/cs7\
 config/ocs/cs8\
 config/keys/cs/cs1\
 config/keys/cs/cs2\
 config/keys/cs/cs3\
 config/keys/cs/cs4\
 config/keys/ca_cert/crl\
 config/sysinfo\
 data\
 data/backup\
 data/DebugBin\
 data/restore\
 output\
 output/audit\
 output/cdr\
 output/core\
 output/faults\
 output/IVR\
 output/links\
 output/log\
 output/media_rec/share\
 output/message_files\
 output/nids\
 output/tcp_dump/emb\
 output/tcp_dump/mcms\
 output/message_files\
 output/backup\
 output/local_tracer\
 output/tmp
 tmp\
 tmp/startup_logs\
 tmp/queue\
 tmp/shared_memory\
 tmp/semaphore\
 tmp/Scripts\
"

mkdir -p ../BUILDROOT/etc/init.d/
mkdir -p ../BUILDROOT%{_mcuHomeDir}
cd ../BUILDROOT%{_mcuHomeDir}
mkdir -p $DIRS
cd -

# Copies specific package files to the installation directory.
cp -a ../BUILD/Main/Bin*/* ../BUILDROOT%{_mcuHomeDir}/mcms/Bin/
cp -a ../BUILD/Main/snmpd ../BUILDROOT%{_mcuHomeDir}/mcms/

#To avoid stripping of Flexera libraries, replace it with lib archive
rm -f ../BUILDROOT%{_mcuHomeDir}/mcms/Bin/libFlx*.so*
(dir=$PWD; cd /opt/polycom/flexera/fne-xt-i86_linux-5.2.0/lib; tar cfz $dir/../BUILDROOT%{_mcuHomeDir}/mcms/Bin/libflx.tgz libFlx*.so*)


DELS="\
 *.Test\
 GideonSim\
 TestClient\
 Demo\
 EndpointsSim\
 libMocks.so\
 libm.so\
 libresolv.*\
 libz.so*\
 libstdc++.so*\
 libxml2.so*\
 libdigestmd5.so*\
 libiconv.so*\
 mod_rewrite.so\
 mod_asis.so\
 libphp5.so\
 snmpd\
 snmpdj
"

cd ../BUILDROOT%{_mcuHomeDir}/mcms/Bin
rm -f $DELS
strip $(ls | grep -v ApacheModule | grep -v "libcrypto.*" | grep -v "libssl.*" | grep -v "libflx.tgz")
cd -

cp -a ../BUILD/Main/StaticCfg/* ../BUILDROOT%{_mcuHomeDir}/mcms/StaticCfg/
cp -a ../BUILD/Main/Scripts/*.sh ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -a ../BUILD/Main/Scripts/*.txt ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -dpR ../BUILD/Main/Scripts/TLS ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/.
cp -f ../BUILD/Main/Scripts/soft_mcu ../BUILDROOT/etc/init.d/
chmod u+x ../BUILDROOT/etc/init.d/soft_mcu
cp -R ../BUILD/Main/MIBS ../BUILDROOT%{_mcuHomeDir}/mcms/
rm -rf ../BUILDROOT%{_mcuHomeDir}/mcms/MIBS/index.html
cp -f ../BUILD/Main/Versions.xml ../BUILDROOT%{_mcuHomeDir}/mcms/
cp -a ../BUILD/Main/VersionCfg/* ../BUILDROOT%{_mcuHomeDir}/mcms/VersionCfg/
cp -Rf ../BUILD/Main/VersionCfg/IVR ../BUILDROOT%{_mcuHomeDir}/mcms/StaticCfg
cp -Rf ../BUILD/Main/VersionCfg/AudibleAlarms ../BUILDROOT%{_mcuHomeDir}/mcms/StaticCfg

# Do not need default IPServiceList.
rm -f ../BUILDROOT%{_mcuHomeDir}/mcms/StaticCfg/IPServiceList.xml
rm -f ../BUILDROOT%{_mcuHomeDir}/mcms/StaticCfg/IPServiceListTmp.xml

#For CG scripts
cp -f ../BUILD/Main/Scripts/ResourceUtilities.py ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/McmsConnection.py ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/SystemCapacityFunctions.py ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/SMCUConference.py ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/SoftMcuVideoParty.py ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/SoftMcuDialFromCG2Cloud.py ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/login.xml ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/SMCUDialOutParty.xml ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/CreateNewProfile.xml ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/GetProfileList.xml ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/AddCpConf.xml ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/TransConfList.xml ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/TransConf2.xml ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/DeleteVoipConf.xml ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/RemoveNewProfile.xml ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/
cp -f ../BUILD/Main/Scripts/Logout.xml ../BUILDROOT%{_mcuHomeDir}/mcms/Scripts/

echo "RMX2000" > ../BUILDROOT%{_mcuHomeDir}/config/product
echo "NO" > ../BUILDROOT%{_mcuHomeDir}/config/JITC_MODE.txt
echo "NO" > ../BUILDROOT%{_mcuHomeDir}/config/JITC_MODE_FIRST_RUN.txt
echo "NO" > ../BUILDROOT%{_mcuHomeDir}/config/SEPARATE_MANAGMENT_NETWORK.txt

echo "#!/bin/sh
export MCU_HOME_DIR=\$(grep MCU_HOME_DIR /home/mcms/.bashrc | cut -d'=' -f2 | awk 'NR==1')
rm -f /tmp/netsnmpfipsflag
sudo /bin/env LD_LIBRARY_PATH=\$MCU_HOME_DIR/mcms/Bin \$MCU_HOME_DIR/mcms/snmpd -Lf \$MCU_HOME_DIR/tmp/snmpd.log -C -c \$MCU_HOME_DIR/mcms/Cfg/snmpd.conf,\$MCU_HOME_DIR/tmp/snmpd.conf" > ../BUILDROOT%{_mcuHomeDir}/mcms/Bin/snmpd

echo "#!/bin/sh
export MCU_HOME_DIR=\$(grep MCU_HOME_DIR /home/mcms/.bashrc | cut -d'=' -f2 | awk 'NR==1')
touch /tmp/netsnmpfipsflag
sudo /bin/env LD_LIBRARY_PATH=\$MCU_HOME_DIR/mcms/Bin \$MCU_HOME_DIR/mcms/snmpd -F -Lf \$MCU_HOME_DIR/tmp/snmpd.log -C -c \$MCU_HOME_DIR/mcms/Cfg/snmpd.conf,\$MCU_HOME_DIR/tmp/snmpd.conf" > ../BUILDROOT%{_mcuHomeDir}/mcms/Bin/snmpdj

chmod a+x ../BUILDROOT%{_mcuHomeDir}/mcms/Bin/snmpd ../BUILDROOT%{_mcuHomeDir}/mcms/Bin/snmpdj

buildpath="$PWD"
cd ../BUILDROOT%{_mcuHomeDir}

# Prepares file list for RPM update.
echo "%defattr(-,root,root,-)" > $buildpath/mcms-file-list

# The list contains directories name on first level and files. The list
# doesn't contain configuration directories.
find -maxdepth 2 -type d -o -type f -o -type l | sed 's/^.//g' | grep -E "^\/.{1,}\/" | grep -E -v '^/config|^/mcms/Cfg' >> $buildpath/mcms-file-list


# Add Prefix to each file
sed -i 's!^/!%{_mcuHomeDir}/!g' $buildpath/mcms-file-list

echo "/etc/init.d/soft_mcu" >> $buildpath/mcms-file-list
cd $buildpath

# Create 'dummy' configuration files to allow their definition in the spec file - supports upgrade
touch ../BUILDROOT%{_mcuHomeDir}/mcms/Cfg/dummy

echo "At the end we are in: $PWD"

##################################################
# Post install - libraries should be relinked    #
##################################################
%post

##################################
# This part is common for both install and upgrade    #
##################################
export MCU_HOME_DIR=$RPM_INSTALL_PREFIX
export MCU_LIBS=$RPM_INSTALL_PREFIX/lib:$RPM_INSTALL_PREFIX/lib64:$RPM_INSTALL_PREFIX/usr/lib:$RPM_INSTALL_PREFIX/usr/lib64

echo "\
#!/bin/sh

CMD='/bin/env MCU_HOME_DIR=$RPM_INSTALL_PREFIX LD_LIBRARY_PATH=$RPM_INSTALL_PREFIX/mcms/Bin $RPM_INSTALL_PREFIX/mcms/Bin/httpd'
CFG=$RPM_INSTALL_PREFIX/mcms/StaticCfg/httpd.conf
LOG=$RPM_INSTALL_PREFIX/tmp/startup_logs/DaemonProcessesLoad.log

sleep 1
\$CMD -f \$CFG 2>&1 | tee -a \$LOG
while true; do
    sleep 10
    ps -e | grep -v grep | grep httpd >/dev/null || \$CMD -f \$CFG 2>&1 | tee -a \$LOG
done\
" > $RPM_INSTALL_PREFIX/mcms/Bin/ApacheModule

chmod a+x $RPM_INSTALL_PREFIX/mcms/Bin/ApacheModule

##################################
# This part is for upgrade only                                  #
##################################
if [[ $1 == 2 ]];then

  $RPM_INSTALL_PREFIX/mcms/Scripts/CopyDefaultIVR_no_overwrite.sh &> /dev/null
fi

##################################
# This part is for install only                                      #
##################################
if [[ $1 == 1 ]];then
  userdel -r mcms  &> /dev/null
  groupdel polycom_mcu  &> /dev/null
  groupdel mcms  &> /dev/null
  groupadd mcms
  groupadd polycom_mcu
  #useradd mcms -g mcms -m -p $1$4fVsZ0$d.Zpukxy3nEZNEMJj5nVB0 -G polycom_mcu
  useradd mcms -g mcms -m -G polycom_mcu 

  #Don't chkconfig in case of SUSE - BRIDGE-12821
  if [[ `cat /etc/*release*  2> /dev/null | grep SUSE` ]]; then
    insserv soft_mcu -d
  else
    chkconfig --add soft_mcu 2> /dev/null
    chkconfig --level 345 soft_mcu on 2> /dev/null
  fi

  # BRIDGE-14280
  if [ ! -f /etc/rc.d/rc3.d/S99soft_mcu ]; then
        find /etc/rc.d/* -name "S*soft_mcu" -exec rm -f {} \;
        find /etc/rc.d/* -name "K*soft_mcu" -exec rm -f {} \;
  fi
  cd /etc/rc.d
  ln -sf /etc/init.d/soft_mcu rc3.d/S99soft_mcu
  ln -sf /etc/init.d/soft_mcu rc4.d/S99soft_mcu
  ln -sf /etc/init.d/soft_mcu rc5.d/S99soft_mcu
  cd -
  if [ ! -f /etc/rc.d/rc1.d/K01soft_mcu ]; then
        find /etc/rc.d/* -name "K*soft_mcu" -exec rm -f {} \;
  fi
  cd /etc/rc.d
  ln -sf /etc/init.d/soft_mcu rc0.d/K01soft_mcu
  ln -sf /etc/init.d/soft_mcu rc1.d/K01soft_mcu
  ln -sf /etc/init.d/soft_mcu rc2.d/K01soft_mcu
  ln -sf /etc/init.d/soft_mcu rc6.d/K01soft_mcu
  cd -

  echo "export MCU_HOME_DIR=$RPM_INSTALL_PREFIX" >> /home/mcms/.bashrc
  echo "export MCU_LIBS=$RPM_INSTALL_PREFIX/lib:$RPM_INSTALL_PREFIX/lib64:$RPM_INSTALL_PREFIX/usr/lib:$RPM_INSTALL_PREFIX/usr/lib64" >> /home/mcms/.bashrc

  chmod 1777 $RPM_INSTALL_PREFIX/tmp

  $RPM_INSTALL_PREFIX/mcms/Scripts/CopyDefaultIVR.sh

  sed -i 's/Defaults.*requiretty/Defaults    !requiretty/' /etc/sudoers

  # Create a dedicated loopdevice FS file for core dumps.
  dd if=/dev/zero of=$RPM_INSTALL_PREFIX/mcms/cores.ext3 bs=1 count=0 seek=9G 2>/dev/null
  yes | mkfs -t ext3 $RPM_INSTALL_PREFIX/mcms/cores.ext3 &>/dev/null

  sed -i '/cores.ext3/d' /etc/fstab
  echo -e "$RPM_INSTALL_PREFIX/mcms/cores.ext3\t$RPM_INSTALL_PREFIX/output/core\text3\tdefaults,loop\t0 0" >> /etc/fstab
  mount $RPM_INSTALL_PREFIX/output/core
fi

INSTALL_INFO=/tmp/mcu_install.info
mkdir -p -v $RPM_INSTALL_PREFIX/usr/lib >> $INSTALL_INFO
mkdir -p -v $RPM_INSTALL_PREFIX/usr/lib64 >> $INSTALL_INFO
mkdir -p -v $RPM_INSTALL_PREFIX/lib >> $INSTALL_INFO
mkdir -p -v $RPM_INSTALL_PREFIX/lib64 >> $INSTALL_INFO
mkdir -p -v $RPM_INSTALL_PREFIX/etc >> $INSTALL_INFO
mkdir -p -v $RPM_INSTALL_PREFIX/var >> $INSTALL_INFO
mkdir -p -v $RPM_INSTALL_PREFIX/opt >> $INSTALL_INFO
mkdir -p -v $RPM_INSTALL_PREFIX/etc/ld.so.conf.d >> $INSTALL_INFO
mkdir -p -v $RPM_INSTALL_PREFIX/var/log/trace >> $INSTALL_INFO
mkdir -p -v $RPM_INSTALL_PREFIX/var/log/opr >> $INSTALL_INFO

ln -sf $RPM_INSTALL_PREFIX/output/log $RPM_INSTALL_PREFIX/mcms/Diagnostics/logs
ln -sf $RPM_INSTALL_PREFIX/output/core $RPM_INSTALL_PREFIX/mcms/Diagnostics/cores
ln -sf $RPM_INSTALL_PREFIX/output/faults $RPM_INSTALL_PREFIX/mcms/Diagnostics/faults
ln -sf $RPM_INSTALL_PREFIX/output/cslogs $RPM_INSTALL_PREFIX/mcms/Diagnostics/cs
ln -sf $RPM_INSTALL_PREFIX/output/mylogs $RPM_INSTALL_PREFIX/mcms/Diagnostics/os
ln -sf $RPM_INSTALL_PREFIX/output/apache $RPM_INSTALL_PREFIX/mcms/Diagnostics/apache
ln -sf $RPM_INSTALL_PREFIX/output/media_rec $RPM_INSTALL_PREFIX/mcms/Diagnostics/MediaRecording
ln -sf $RPM_INSTALL_PREFIX/output/log  $RPM_INSTALL_PREFIX/mcms/Root/LogFiles
ln -sf $RPM_INSTALL_PREFIX/output/cdr $RPM_INSTALL_PREFIX/mcms/Root/CdrFiles
ln -sf $RPM_INSTALL_PREFIX/output/nids $RPM_INSTALL_PREFIX/mcms/Root/NIDS
ln -sf $RPM_INSTALL_PREFIX/output/log $RPM_INSTALL_PREFIX/mcms/LogFiles
ln -sf $RPM_INSTALL_PREFIX/output/ts $RPM_INSTALL_PREFIX/mcms/TS
ln -sf $RPM_INSTALL_PREFIX/output/audit $RPM_INSTALL_PREFIX/mcms/Audit
ln -sf $RPM_INSTALL_PREFIX/output/core $RPM_INSTALL_PREFIX/mcms/Cores
ln -sf $RPM_INSTALL_PREFIX/output/tcp_dump/emb $RPM_INSTALL_PREFIX/mcms/TcpDumpEmb
ln -sf $RPM_INSTALL_PREFIX/output/tcp_dump/mcms $RPM_INSTALL_PREFIX/mcms/TcpDumpMcms
ln -sf $RPM_INSTALL_PREFIX/output/cdr $RPM_INSTALL_PREFIX/mcms/CdrFiles
ln -sf $RPM_INSTALL_PREFIX/output/faults $RPM_INSTALL_PREFIX/mcms/Faults
ln -sf $RPM_INSTALL_PREFIX/output/nids $RPM_INSTALL_PREFIX/mcms/NIDS
ln -sf $RPM_INSTALL_PREFIX/output/links $RPM_INSTALL_PREFIX/mcms/Links
ln -sf $RPM_INSTALL_PREFIX/output/media_rec $RPM_INSTALL_PREFIX/mcms/MediaRecording
ln -sf $RPM_INSTALL_PREFIX/output/message_files $RPM_INSTALL_PREFIX/mcms/MessageFiles
ln -sf $RPM_INSTALL_PREFIX/output/local_tracer $RPM_INSTALL_PREFIX/mcms/LocalTracer

ln -sf $RPM_INSTALL_PREFIX/mcms/Diagnostics $RPM_INSTALL_PREFIX/mcms/Root/Diagnostics
ln -sf $RPM_INSTALL_PREFIX/mcms/MIBS $RPM_INSTALL_PREFIX/mcms/Root/MIBS

ln -sf $RPM_INSTALL_PREFIX/data/DebugBin $RPM_INSTALL_PREFIX/mcms/DebugBin
ln -sf $RPM_INSTALL_PREFIX/data/new_version $RPM_INSTALL_PREFIX/mcms/Install
ln -sf $RPM_INSTALL_PREFIX/data/backup $RPM_INSTALL_PREFIX/mcms/Backup
ln -sf $RPM_INSTALL_PREFIX/data/restore $RPM_INSTALL_PREFIX/mcms/Restore

ln -sf $RPM_INSTALL_PREFIX/config/JITC_MODE.txt $RPM_INSTALL_PREFIX/mcms/JITC_MODE.txt
ln -sf $RPM_INSTALL_PREFIX/config/JITC_MODE_FIRST_RUN.txt $RPM_INSTALL_PREFIX/mcms/JITC_MODE_FIRST_RUN.txt
ln -sf $RPM_INSTALL_PREFIX/config/SEPARATE_MANAGMENT_NETWORK.txt $RPM_INSTALL_PREFIX/mcms/SEPARATE_MANAGMENT_NETWORK.txt
ln -sf $RPM_INSTALL_PREFIX/config/mcms $RPM_INSTALL_PREFIX/mcms/Cfg
ln -sf $RPM_INSTALL_PREFIX/config/states $RPM_INSTALL_PREFIX/mcms/States
ln -sf $RPM_INSTALL_PREFIX/config/keys $RPM_INSTALL_PREFIX/mcms/Keys
ln -sf $RPM_INSTALL_PREFIX/config/keys/cs $RPM_INSTALL_PREFIX/mcms/KeysForCS
ln -sf $RPM_INSTALL_PREFIX/config/keys/ca_cert $RPM_INSTALL_PREFIX/mcms/CACert
ln -sf $RPM_INSTALL_PREFIX/config/keys/ca_cert/crl $RPM_INSTALL_PREFIX/mcms/CRL
ln -sf $RPM_INSTALL_PREFIX/config/static_states $RPM_INSTALL_PREFIX/mcms/StaticStates
ln -sf $RPM_INSTALL_PREFIX/config/ocs $RPM_INSTALL_PREFIX/mcms/OCS
ln -sf $RPM_INSTALL_PREFIX/config/product $RPM_INSTALL_PREFIX/mcms/ProductType
ln -sf $RPM_INSTALL_PREFIX/usr $RPM_INSTALL_PREFIX/mcms/python
ln -sf $RPM_INSTALL_PREFIX/mcms/Bin/libProcessBase.so $RPM_INSTALL_PREFIX/mcms/Bin/libiconv.so.2

sed -i '/VMCU_BEG/,/VMCU_END/d' /etc/sudoers
echo -e "\
# VMCU_BEG
ALL\tALL=NOPASSWD:\
 $RPM_INSTALL_PREFIX/mcms/Scripts/run_cs.sh,\
 /usr/bin/sudo

mcms\tALL=NOPASSWD:\
 /bin/env MCU_HOME_DIR=$RPM_INSTALL_PREFIX LD_LIBRARY_PATH=$RPM_INSTALL_PREFIX/mcms/Bin $RPM_INSTALL_PREFIX/mcms/Bin/httpd -f $RPM_INSTALL_PREFIX/mcms/StaticCfg/httpd.conf,\
 /bin/env MCU_HOME_DIR=$RPM_INSTALL_PREFIX LD_LIBRARY_PATH=$RPM_INSTALL_PREFIX/usr/local/apache2/lib\:$RPM_INSTALL_PREFIX/mcms/Bin $RPM_INSTALL_PREFIX/mcms/Bin/httpd -f $RPM_INSTALL_PREFIX/mcms/StaticCfg/httpd.conf -k restart,\
 /usr/sbin/tcpdump,\
 /usr/bin/killall tcpdump,\
 $RPM_INSTALL_PREFIX/tmp/tmpSysCallScript.sh,\
 /sbin/service soft_mcu restart,\
 /bin/env LD_LIBRARY_PATH=$RPM_INSTALL_PREFIX/mcms/Bin $RPM_INSTALL_PREFIX/mcms/snmpd -Lf $RPM_INSTALL_PREFIX/tmp/snmpd.log -C -c $RPM_INSTALL_PREFIX/mcms/Cfg/snmpd.conf\,$RPM_INSTALL_PREFIX/tmp/snmpd.conf,\
 /bin/env LD_LIBRARY_PATH=$RPM_INSTALL_PREFIX/mcms/Bin $RPM_INSTALL_PREFIX/mcms/snmpd -F -Lf $RPM_INSTALL_PREFIX/tmp/snmpd.log -C -c $RPM_INSTALL_PREFIX/mcms/Cfg/snmpd.conf\,$RPM_INSTALL_PREFIX/tmp/snmpd.conf,\
 /usr/bin/killall -q snmpd,\
 /usr/bin/killall -q -1 snmpd,\
 /bin/cat /sys/class/dmi/id/*,\
 /usr/bin/killall NTP_Bypass_SoftMCU_Server,\
 /sbin/hwclock,\
 /bin/date,\
 /sbin/service httpd stop,\
 /sbin/service httpd start,\
 /bin/sync,\
 /usr/bin/killall -2 IpmcSim.x86 LinuxSysCallPro mpproxy audio_soft video mfa,\
 /usr/bin/killall -9 IpmcSim.x86 LinuxSysCallPro mpproxy audio_soft video mfa,\
 /usr/bin/killall -9 ASS-AH AMP-AmpAHAc AMP-AmpAHDec0 AMP-AmpAHDec1 AMP-AmpAHDec2 AMP-AmpAHDec3 AMP-AmpAHDec4 AMP-AmpAHDec5 AMP-AmpAHDec6 AMP-AmpAHDec7 AMP-AmpAHDec8 AMP-AmpAHDec9,\
 /usr/bin/killall -9 AMP-AmpAHEnc0 AMP-AmpAHEnc1 AMP-AmpAHEnc2 AMP-AmpAHEnc3 AMP-AmpAHEnc4 AMP-AmpAHEnc5 AMP-AmpAHEnc6 AMP-AmpAHEnc7 AMP-AmpAHEnc8 AMP-AmpAHEnc9 AMP-AmpAHFp,\
/usr/bin/killall -9 AMP-AmpAHIvr0 AMP-AmpAHIvr1 AMP-AmpAHIvr2 AMP-AmpAHIvr3 AMP-AmpAHIvr4 AMP-AmpAHIvr5 AMP-AmpAHIvr6 AMP-AmpAHIvr7 AMP-AmpAHIvr8 AMP-AmpAHIvr9 AMP-AmpAHMgr,\
 /usr/bin/killall -9 ASS-AMPLog ASS-AMPMgr ASS-AMPUdpRx0 ASS-AMPUdpRx1 ASS-AMPUdpRx2 ASS-AMPUdpRx3 ASS-AMPUdpRx4,\
 /usr/bin/killall -9 ASS-AMPUdpTx0 ASS-AMPUdpTx1 ASS-AMPUdpTx2 ASS-AMPUdpTx3 ASS-AMPUdpTx4 ASS-AMPUdpTx5 ASS-AMPUdpTx6 ASS-AMPUdpTx7 ASS-AMPUdpTx8 ASS-AMPUdpTx9,\
/bin/sed -i s/^\#DHCPV6C=.*$/DHCPV6C=yes/g /etc/sysconfig/network-scripts/ifcfg-*,\
/bin/sed -i s/^DHCPV6C=.*$/\#DHCPV6C=yes/g /etc/sysconfig/network-scripts/ifcfg-*,\
 $RPM_INSTALL_PREFIX/cs/bin/csping, $RPM_INSTALL_PREFIX/cs/bin/csping6, /usr/bin/killall -q -9 httpd lt-httpd
# VMCU_END\
" >> /etc/sudoers

sed -i '/VMCU_BEG/,/VMCU_END/d' /etc/security/limits.conf

echo -e "\
# VMCU_BEG
mcms hard stack  10240
mcms soft stack  10240
mcms hard nproc  10240
mcms soft nproc  10240
mcms hard nofile 61440
mcms soft nofile 17408
# VMCU_END\
" >> /etc/security/limits.conf

(cd $RPM_INSTALL_PREFIX/mcms/Bin; tar xfz libflx.tgz; rm libflx.tgz)

DIRS="\
 $RPM_INSTALL_PREFIX/config\
 $RPM_INSTALL_PREFIX/mcms\
 $RPM_INSTALL_PREFIX/data\
 $RPM_INSTALL_PREFIX/output\
 $RPM_INSTALL_PREFIX/tmp/startup_logs\
 $RPM_INSTALL_PREFIX/tmp/queue\
 $RPM_INSTALL_PREFIX/tmp/shared_memory\
 $RPM_INSTALL_PREFIX/tmp/semaphore\
"

cat $INSTALL_INFO >> $RPM_INSTALL_PREFIX/mcms/mcu_install.info
rm -rf $INSTALL_INFO
chown -R mcms:mcms $DIRS
chmod a-w $RPM_INSTALL_PREFIX/mcms/Bin/*

##################################################
# Post uninstall - libraries should be relinked  #
##################################################
%postun
if [[ $1 == 0 ]];then
  umount $RPM_INSTALL_PREFIX/output/core/

  INSTALL_INFO=$RPM_INSTALL_PREFIX/mcms/mcu_install.info
  sed -i "s|mkdir: created directory \`|rm -rf \'|g" $INSTALL_INFO
  chmod 777 $INSTALL_INFO
  $INSTALL_INFO
  rm -rf $RPM_INSTALL_PREFIX/mcms $RPM_INSTALL_PREFIX/output $RPM_INSTALL_PREFIX/data $RPM_INSTALL_PREFIX/config

  groupdel polycom_mcu
  userdel -r mcms

  sed -i '/cores.ext3/d' /etc/fstab
  sed -i '/VMCU_BEG/,/VMCU_END/d' /etc/sudoers
  sed -i '/VMCU_BEG/,/VMCU_END/d' /etc/security/limits.conf
  
  # BRIDGE-14280
  find /etc/rc.d/* -name "S*soft_mcu" -exec rm -f {} \;
  find /etc/rc.d/* -name "K*soft_mcu" -exec rm -f {} \;
fi

##################################################
# Cleaning up and finishing ...                  #
##################################################
%clean
echo "Exiting $0 build process of with $?"

##################################################
# The files to be installed  are listed here.    #
# These file's path relates to                   #
# %{_topdir}/BUILD                               #
# but will be installed in this full path.       #
##################################################
%files -f mcms-file-list
%config(noreplace)
%{_mcuHomeDir}/mcms/Cfg
%{_mcuHomeDir}/config

#______________ Update above here ________________

%changelog
* Thu Jul 17 2014 Penrod Li <penrod.li@polycom.com>
  remove debug info
* Mon May 26 2014 Ori Pugatzky <ori.pugatzky@polycom.co.il>
  RPM now creates new snmpd, snmpdj
* Sun Feb 16 2014 David Rabkin <david.rabkin@polycom.co.il>
  Minimized code in post install and post uninstall sections in order
  to support smooth update.
* Wed Nov 6 2013 Ori Pugatzky <ori.pugatzky@polycom.co.il>
  Increase open file descriptors limit to 61440 per need to support 1000 ICE EPs.
* Sun Jul 21 2013 Ori Pugatzky <ori.pugatzky@polycom.co.il>
  Remove OpenSource public available libs, and add their package names to 'Requires' fields.
* Thu Jun 6 2013 Ori Pugatzky <ori.pugatzky@polycom.co.il>
  Tune limits.conf
* Sun Mar 17 2013 Nati Shauli <nati.shauli@polycom.co.il>
  Change resolv.conf permissions
* Sun Mar 10 2013 Nati Shauli <nati.shauli@polycom.co.il>
  NTP support
* Fri Nov 23 2012 Ori Pugatzky <ori.pugatzky@polycom.co.il>
  Outputs are copied to BUILDROOT
* Thu Nov 15 2012 Nati Shauli <nati.shauli@polycom.co.il>
- add sudo and tcpdump to sudoers.
* Tue Sep 04 2012 Judith Maman <judith.shuva@polycom.co.il>
- core setup - pattern, ulimit - move them to SoftMcuMain spec file
* Wed Jul 18 2012 Judith Maman <judith.shuva@polycom.co.il>
- Remove version number from the spec file. 
* Tue Jul 10 2012 Ori Pugatzky <ori.pugatzky@polycom.co.il>
- core setup - pattern, ulimit
* Wed Jul 4 2012 Ori Pugatzky <ori.pugatzky@polycom.co.il>
- Dedicated 2G FS for core files mounted under $RPM_INSTALL_PREFIX/mcms/Cores
* Sun May 6 2012 Ori Pugatzky <ori.pugatzky@polycom.co.il>
- Updated to support upgrade
* Thu Mar 22 2012 Shachar Bar <shachar.bar@polycom.co.il>
- solve GLIBC_PRIVATE dependencies
* Mon Mar 19 2012 Judith Maman <judith.shuva@polycom.co.il>
- Creating the rpm

