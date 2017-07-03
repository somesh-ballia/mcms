#!/bin/sh 
#pass faild script
#developer : eran decker(416)
# $1 is results folder


cd $1
SCRIPTS_TO_LIST="`ls -rt1 . | grep -v "\."`"

# Eitan 01/2009 - print a pass/fail report without valgrind
for ScriptName in $SCRIPTS_TO_LIST
   do
      N_PASSED_NO_VALGRIND=`grep -c -w $ScriptName PassedScriptsNoValgrind.log`
      N_FAILED_NO_VALGRIND=`grep -c -w $ScriptName FailedScriptsNoValgrind.log`
      if [ "$N_FAILED_NO_VALGRIND" != "0" ];
      then
        grep -w -m1 $ScriptName FailedScriptsNoValgrind.log | 
         awk '{ gsub(/.py /, " ---"); gsub(/.py/, " "); gsub(/_With_/, " "); gsub(/_under_valdgrind/, ""); gsub(/\(/, ""); gsub(/\)/, "");  printf( "%-65s %-10s %-10s %s %s %s %s %s\n",$1,$2,$3,$4,$5,$6,$7,$8,$9);}'
#
#printf("%-65s %s %s %s %s %s",$1,$3,,$5,$6,$7);}' 
#        printf " ( %2d / %2d )\n" $N_FAILED $N_PASSED
#        grep $ScriptName FailedScripts.log 
      elif [ "$N_PASSED_NO_VALGRIND" != "0" ]; then
         printf "%-65s %-10s %-10s\n" $ScriptName " " OK
      fi

   done

echo +-----------------------------------------
echo "* Failed Scripts when running under valgrind"

for ScriptName in $SCRIPTS_TO_LIST
   do
      N_PASSED=`grep -c -w $ScriptName PassedScripts.log`
      N_FAILED=`grep -c -w $ScriptName FailedScripts.log`
#      ls -dl $ScriptName | awk '{ printf("%s %s ",$8,$9);}' 
      if [ "$N_FAILED" != "0" ];
      then
        grep -w -m1 $ScriptName FailedScripts.log | 
         awk '{ gsub(/.py /, " ---"); gsub(/.py/, " "); gsub(/_With_/, " "); gsub(/_under_valdgrind/, ""); gsub(/\(/, ""); gsub(/\)/, "");  printf( "%-65s %-10s %-10s %s %s %s %s %s\n",$1,$2,$3,$4,$5,$6,$7,$8,$9);}'
#
#printf("%-65s %s %s %s %s %s",$1,$3,,$5,$6,$7);}' 
#        printf " ( %2d / %2d )\n" $N_FAILED $N_PASSED
#        grep $ScriptName FailedScripts.log 
#      else
#        printf "%-65s %-10s %-10s\n" $ScriptName " " OK
      fi

   done


