#!/bin/bash
#
# This script verifies glibc 81 is installed on the machine, to allow full gdb functionality (overcome old libthread issue)
#


grep -q "\[more_updates\]" /etc/yum.repos.d/CentOS-Base.repo
if [[ $? == 1 ]]; then
	echo -e "\n[more_updates]\nname=CentOS-\$releasever - More Updates\nmirrorlist=http://mirrorlist.centos.org/?release=\$releasever&arch=\$basearch&repo=updates\n#baseurl=http://mirror.centos.org/centos/\$releasever/updates/\$basearch/\ngpgcheck=1\ngpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-CentOS-5" >> /etc/yum.repos.d/CentOS-Base.repo

fi

rpm -qa | grep glibc | grep 81
if [[ $? == 0 ]]; then
	echo ====REINSTALL===
	yum reinstall -y glibc
else
	echo ===UPGRADE===
	yum upgrade -y glibc
fi

yum install -y gdb

echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "IMPORTANT - run 'service soft_mcu restart' before running gdb"
echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
