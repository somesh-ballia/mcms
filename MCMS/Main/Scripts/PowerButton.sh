#/bin/sh

#while head -n 1 /proc/acpi/event | grep button | grep power
#do
#  echo "Power button was pressed"
#  cd $MCU_HOME_DIR/mcms
#  su -c "./Bin/McuCmd kill McmsDaemon" mcms
#  sleep 2
#  sync
#  reboot
#
#done