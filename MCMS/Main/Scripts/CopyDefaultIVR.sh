#!/bin/sh

# to prevent partlly copying of the files, we first copy all the files to tmp directory
# if the copy succseed we rename to directory name to the right one.

mkdir $MCU_HOME_DIR/mcms/Cfg/IVR.tmp
cp -dpR $MCU_HOME_DIR/mcms/StaticCfg/IVR/* $MCU_HOME_DIR/mcms/Cfg/IVR.tmp/ > /dev/null && mv $MCU_HOME_DIR/mcms/Cfg/IVR.tmp $MCU_HOME_DIR/mcms/Cfg/IVR > /dev/null

chown -R mcms:mcms $MCU_HOME_DIR/output/IVR/
chmod -R +w $MCU_HOME_DIR/output/IVR/
chmod -R o-w $MCU_HOME_DIR/output/IVR/
chmod -R o-w $MCU_HOME_DIR/config/mcms
chmod -R o-w $MCU_HOME_DIR/mcms/Cfg/IVR/msg/English/*
