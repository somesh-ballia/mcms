#!/bin/bash

AutoGenLibs=(IvrMccf CdrMccf CdrPersist CdrRegistration TimeConfig LicenseAuthorityConfig CdrConfig)
result=""

if [ "$1" == "test" ]; then
	for i in ${AutoGenLibs[@]}; do
		result=`svn stat Libs/${i}/*.h | grep  -v '^\?' | grep -v '^\D' > /dev/null` 
		if [ "$?" == "0" ]; then
			echo "Compilation Failed, Please remove files in folder Libs/${i}/*.h which does not suppose to be in svn source control, use comand:
svn delete --force Libs/${i}/*.h"	
			exit 101
		fi
		result=`svn stat Libs/${i}/*.cpp | grep  -v '^\?' | grep -v '^\D' > /dev/null` 
		if [ "$?" == "0" ]; then
			echo "Compilation Failed, Please remove files in folder Libs/${i}/*.h which does not suppose to be in svn source control, use comand:
svn delete --force Libs/${i}/*.cpp"	
			exit 102
		fi
		result=`svn stat Libs/${i}/*.xml.list | grep  -v '^\?' | grep -v '^\D' > /dev/null` 
		if [ "$?" == "0" ]; then
			echo "Compilation Failed, Please remove files in folder Libs/${i}/*.h which does not suppose to be in svn source control, use comand:
svn delete --force Libs/${i}/*.xml.list"	
			exit 101
		fi

	done	
else
	for i in ${AutoGenLibs[@]}; do
		echo "rm -f Libs/${i}/*.h"
		rm -f Libs/${i}/*.h
		echo "rm -f Libs/${i}/*.cpp"
		rm -f Libs/${i}/*.cpp
		echo "rm -f Libs/${i}/*.xml.list"
		rm -f Libs/${i}/*.xml.list
	done
fi
