#!/bin/bash

# usage:
# $1 is the name of the test script
# $2 is the name of the test run log file

cd TestResults/$1
grep  -H -c  "Invalid"  *.pid* > Invalid_count.log
grep  -A3    "Invalid"  *.pid* > Invalid_list.log
grep "exceptions -  [^0]" $2 >> Invalid_count.log

echo -----------------------------------------------------------------------------
echo Nightly MCMS Carmel test results - $1
date 
echo -----------------------------------------------------------------------------
echo 
echo number of show-stopper errors found: 
echo -------------------------------------
echo
if 
   grep  -v :0 Invalid_count.log 
then
 echo =============================
 echo ==== Nightly Test FAILED ====
 echo =============================
 echo
 echo =========================
 echo full list of issues found
 echo =========================
 echo
 echo Valgrind errors found: 
 echo ----------------------
 echo
 cat Invalid_list.log
 echo
 echo Night test results log: 
 echo -----------------------
 echo
 cat $2
 echo

else
 echo
 echo =============================
 echo ==== Nightly Test PASSED ====
 echo =============================
 echo
fi

echo ---------------------------------
echo end of report for
date
echo ---------------------------------
