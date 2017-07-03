#!/bin/sh

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_15

#Test the validity of a file without a trailer
Bin/McuCmd @test_new_ver_validity Installer Scripts/InstallerTestFiles/file_without_trailer.bin | grep "fail to retrieve trailer information"
if [ $? != 0 ]
then
	echo "Test #1 - check validity of a file without a trailer - failed"
	exit 1
else
	echo "Test #1 - check validity of a file without a trailer - succeeded"
fi

#Test the validity of a file which its MD5 does not match trailer's MD5
Bin/McuCmd @test_new_ver_validity Installer Scripts/InstallerTestFiles/file_with_trailer_bad_md5.bin | grep "MD5 mismatch"
if [ $? != 0 ]
then
	echo "Test #2 - check validity of a file which its MD5 does not match trailer's MD5 - failed"
	exit 1
else
	echo "Test #2 - check validity of a file which its MD5 does not match trailer's MD5 - succeeded"
fi

#Test the validity of a file which contains only trailer's information (no data)
Bin/McuCmd @test_new_ver_validity Installer Scripts/InstallerTestFiles/only_trailer.txt | grep "fail to calculate file's MD5"
if [ $? != 0 ]
then
	echo "Test #3 - check validity of a file which contains only trailer's information (no data) - failed"
	exit 1
else
	echo "Test #3 - check validity of a file which contains only trailer's information (no data) - succeeded"
fi

#Test the validity of a valid file
Bin/McuCmd @test_new_ver_validity Installer Scripts/InstallerTestFiles/file_with_trailer.bin | grep "version is valid."
if [ $? != 0 ]
then
	echo "Test #4 - check validity of a valid file - failed"
	exit 1
else
	echo "Test #4 - check validity of a valid file - succeeded"
fi

echo "TestNewVersionDownload.sh - succeeded"

exit 0