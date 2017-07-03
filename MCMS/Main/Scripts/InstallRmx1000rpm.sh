#!/bin/bash

export RPM_PATH="$MCU_HOME_DIR/tmp/rmx1000.rpm"
export RPM_NAME=`rpm -qp $RPM_PATH`
#export RPM_NAME=`readlink $MCU_HOME_DIR/tmp/rmx1000.rpm |  awk -F/ '{print $NF'}`

#install rpm
echo -n "Looking for ${RPM_NAME}..."
rpm -qa | grep ${RPM_NAME} &> /dev/null
if [[ $? != 0 ]]; then
	echo ""
	rpm -qa | grep Plcm-Rmx1000*  &> /dev/null
	if [ $? -eq 0 ]; then
		echo "Uninstalling old Rmx1000 rpm "
		sudo rpm -e $(rpm -qa | grep Plcm-Rmx1000*) --nodeps 
	fi

	echo "Installing new Rmx1000 rpm"
	sudo rpm -ivh --prefix / ${RPM_PATH} --nodeps &> /dev/null
	##	Old RPMs don't support a prefix tag, therefore the previous installation fails, so we run it again without prefix.
	sudo rpm -ivh ${RPM_PATH} --nodeps &> /dev/null	
	sleep 5
	sudo killall launcher sys mp audio_soft traced Proxy sys_status_moni ManageMcuMenu menu httpd mpproxy video sys_status_monitor &> /dev/null
else
	echo "Found"
fi

#edit license
sudo chmod o+w $MCU_HOME_DIR/etc/rmx1000/sysinfo
sed -i 's/^key.*/key=XDST-TEST-7506-0000-0000/' $MCU_HOME_DIR/etc/rmx1000/sysinfo/license  
sudo chmod o-w $MCU_HOME_DIR/etc/rmx1000/sysinfo

#edit launcher
sudo chmod o+w $MCU_HOME_DIR/etc/rmx1000/sysconfig/imgs_to_launch.conf
cp $MRMX_DIR/sysconfig/imgs_to_launch.conf $MCU_HOME_DIR/etc/rmx1000/sysconfig/imgs_to_launch.conf 
sudo chmod o-w  $MCU_HOME_DIR/etc/rmx1000/sysconfig/imgs_to_launch.conf



#edit rmx1000 links
sudo chmod o+w $MCU_HOME_DIR/usr/rmx1000/bin

#if [ ! -e $MCU_HOME_DIR/usr/rmx1000/bin/mermaid.orig ]; then
#	mv $MCU_HOME_DIR/usr/rmx1000/bin/mermaid $MCU_HOME_DIR/usr/rmx1000/bin/mermaid.orig
#fi

#ln -sf $MRMX_DIR/mermaid/mermaid $MCU_HOME_DIR/usr/rmx1000/bin/mermaid
ln -sf $MRMX_DIR/mp_proxy/proxy/mpproxy $MCU_HOME_DIR/usr/rmx1000/bin/mpproxy
ln -sf $MRMX_DIR/ampSoft/bin/audio_soft $MCU_HOME_DIR/usr/rmx1000/bin/audio_soft
ln -sf $MRMX_DIR/ampSoft/bin/SetAudioSoftPriority.sh $MCU_HOME_DIR/usr/rmx1000/bin/SetAudioSoftPriority.sh
ln -sf $MRMX_DIR/vmp/video/bin/video_binary	$MCU_HOME_DIR/usr/rmx1000/bin/video
ln -sf $MRMX_DIR/vmp/video/bin/videomon		$MCU_HOME_DIR/usr/rmx1000/bin/videomon
ln -sf $MRMX_DIR/vmp/video/bin/avcecfg_BL.cfg	$MCU_HOME_DIR/usr/rmx1000/bin/avcecfg_BL.cfg
ln -sf $MRMX_DIR/vmp/video/bin/encoder.cfg	$MCU_HOME_DIR/usr/rmx1000/bin/encoder.cfg
ln -sf $MRMX_DIR/vmp/video/bin/SVCE.cfg		$MCU_HOME_DIR/usr/rmx1000/bin/SVCE.cfg

ln -sf $MRMX_DIR/vmp/video/bin/libsvcdd.so $MCU_HOME_DIR/usr/rmx1000/bin/libsvcdd.so 
ln -sf $MRMX_DIR/vmp/video/bin/libsvced.so $MCU_HOME_DIR/usr/rmx1000/bin/libsvced.so
ln -sf $MRMX_DIR/vmp/video/bin/libvpx.a    $MCU_HOME_DIR/usr/rmx1000/bin/libvpx.a

sudo chmod o-w $MCU_HOME_DIR/usr/rmx1000/bin


#edit ldconfig lib
sudo chmod 777 $MCU_HOME_DIR/etc/ld.so.conf.d/
echo "$MCU_HOME_DIR/usr/rmx1000/bin/" > $MCU_HOME_DIR/etc/ld.so.conf.d/plcm_soft_mcu.conf
sudo ldconfig
sudo chmod 755 $MCU_HOME_DIR/etc/ld.so.conf.d/


sudo chmod 777 /var/log/trace/
