#!/bin/bash

# this script create a sammery report for a single test script
# the script should be run after the test script was run
# input parameters:
# $1 is the name of the test script
# $2 is the name of the test results log file 
# assuptions:
# under 'TestResults' there is a folder with the name $1

cd TestResults

mv *.pid* $1
mv *.log $1
mv *core*dump* $1
mv valgrind.* $1
mv callgrind.* $1
mv *.core* $1

cd $1
grep  -H -c  "Invalid"  *.pid* > Invalid_count.log
grep  -A3    "Invalid"  *.pid* > Invalid_list.log
grep "exceptions -  [^0]" $2   > exceptions_list.log
grep "NameError:" $2 > error_list.log
grep "TypeError:" $2 >> error_list.log
grep "SyntaxError:" $2 >>  error_list.log
grep "IndexError:" $2 >>  error_list.log

echo ---------------------------------------------------------------------
echo Script : $1
echo ---------------------------------------------------------------------
if 
  grep  -v :0 Invalid_count.log > /dev/null 
then
 echo ==============================
 echo ==== VALGRIND TEST FAILED ====
 echo ==============================
 echo
 echo Valgrind errors : 
 echo -----------------
 grep  -v :0 Invalid_count.log
 echo
 cat Invalid_list.log
 echo
 echo exception traces : 
 echo ------------------
 echo
 cat exceptions_list.log
 echo
else
 echo
 echo exception traces and asserts : 
 echo ------------------
 echo
 cat exceptions_list.log
 echo
 echo
 echo python run time errors:
 echo -----------------------
 echo
 cat error_list.log
 echo
fi
echo where to find more infomation:
echo ------------------------------
echo The valgrind XXX.pidNNN files are stored under TestResults/$1
echo Test log with the asserts and exceptions is under TestResults/$1/test_run.txt
echo ---

cd ..


