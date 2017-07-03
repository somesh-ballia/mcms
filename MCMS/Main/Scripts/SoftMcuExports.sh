#!/bin/bash

#This script exports for each project in an RPM installed system

export MCU_HOME_DIR=
export MCU_HOME_DIR=$(grep MCU_HOME_DIR /home/mcms/.bashrc | cut -d'=' -f2 | awk 'NR==1')
export MCU_LIBS=$MCU_HOME_DIR/lib:$MCU_HOME_DIR/lib64:$MCU_HOME_DIR/usr/lib:$MCU_HOME_DIR/usr/lib64

mkdir -p $MCU_HOME_DIR/tmp/Scripts

export MCMS_DIR=$MCU_HOME_DIR/mcms/
export CS_DIR=$MCU_HOME_DIR/cs/
export ENGINE_DIR=$MCU_HOME_DIR/usr/share/EngineMRM/
export MPMX_DIR=$MCU_HOME_DIR/usr/share/MFA/
export MRMX_DIR=$MCU_HOME_DIR/usr/rmx1000/bin/
