#!/bin/bash
# GetMCUVersion.sh
#
# Written by David Rabkin
# <david.rabkin@polycom.co.il>

MCU_HOME_DIR=$(grep MCU_HOME_DIR /home/mcms/.bashrc | cut -d'=' -f2 | awk 'NR==1')

TAGS="\
     MAIN \
     MAJOR \
     MINOR \
     INTERNAL \
     "

for t in $TAGS
do
  echo -n `grep $t $MCU_HOME_DIR/mcms/Versions.xml | cut -f2 -d">"|cut -f1 -d"<" | head -1`
  if [[ $t != "INTERNAL" ]]; then
    echo -n "."
  fi
done

echo ""
