#!/bin/bash


if [[ $# == 1 ]];then
	PACKAGE="Plcm-SoftMcuMain-8.0.0.${1}"
else
	PACKAGE="Plcm-SoftMcuMain"
fi

IS_INSTALLED=`rpm -qa | grep Plcm`
if [[ $IS_INSTALLED != "" ]]; then
	echo "Upgrading system"
	yum -y upgrade ${PACKAGE}
	exit $?
else
	echo "Fresh Installing system"
	yum -y install ${PACKAGE}
	exit $?
fi

















