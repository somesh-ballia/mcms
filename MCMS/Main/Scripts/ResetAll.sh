#!/bin/sh

echo reset MFA1
(sleep 1; echo "root" ;echo "polycom"; sleep 1 ; echo "reboot"; sleep 1; echo "exit") | nc 169.254.128.67 23 &
sleep 3
echo reset MFA2
(sleep 1; echo "root" ;echo "polycom"; sleep 1 ; echo "reboot"; sleep 1; echo "exit") | nc 169.254.128.68 23 &
sleep 3
echo reset Shelf
(sleep 1; echo "root" ;echo "polycom"; sleep 1 ; echo "reboot"; sleep 1; echo "exit") | nc 169.254.128.16 23 &
sleep 3
echo reset MCMS
$MCU_HOME_DIR/mcms/Scripts/Reset.sh > /dev/null

exit 0

