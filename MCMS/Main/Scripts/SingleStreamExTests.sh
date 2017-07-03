#!/bin/sh

echo "*************************************"
echo "Running Single Stream developer tests"
echo "*************************************"

echo -n "Enter OFFICIAL BUILD (i.e. 158) and press [ENTER]:"
read OFFICIAL_BUILD

export OFFICIAL_BUILD
export MCMS_DIR=$PWD

make test_scripts && ./Scripts/soft.sh test
 
if [[ $? == 0 ]]; then
	$( ./Scripts/soft.sh start_vm &> /dev/null )&
	./Scripts/Deliverance.sh
	echo "Please wait for MCU stop" 
	./Scripts/soft.sh stop &> /dev/null
fi
