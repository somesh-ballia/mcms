#!/bin/sh

if [ `mount -t ext3 /dev/hda1 /mnt/hda1` ]; then
        cp /boot/grub/* /mnt/hda1/grub/
        umount /mnt/hda1
elif [ `mount -t ext3 /dev/sda1 /mnt/sda1` ]; then
        cp /boot/grub/* /mnt/sda1/grub/
        umount /mnt/sda1
elif [ `mount -t ext3 /dev/sdb1 /mnt/sdb1` ]; then
        cp /boot/grub/* /mnt/sdb1/grub/
        umount /mnt/sdb1
fi


(echo "root (hd0,0)" ; echo "setup (hd0)") | grub --config-file=/grub/menu.lst
