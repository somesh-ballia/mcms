#!/bin/sh
#this script show a list of errors and leaks per process (need to add day)
#$1 is the process name
#$2 is the directory name
# was cat `find Thursday/ | grep .pid | grep $1` | grep -A1 SUMMARY
#cat `find Thursday/ | grep .pid | grep $1` | grep "definitely lost:"
#cat `find Thursday/ | grep .pid | grep $1` | grep "ERROR SUMMARY:"

for myfile in `find $2 | grep valgrind.$1`;
do
    #echo $myfile

   #echo ===$1 Memeory Leaks===
   LEAK="`cat $myfile | grep "definitely lost:"`";
   if [ "$LEAK" != "" ]
   then
     echo "LEAK: "$LEAK",file:"$myfile | grep -v "definitely lost: 0 bytes";
   fi
   
   #echo ===$1 Memeory Errors===
   MEMERR="`cat $myfile | grep "ERROR SUMMARY:"`";
   if [ "$MEMERR" != "" ]
   then
     echo "MEMORY ERROR: "$MEMERR",file:"$myfile | grep -v "0 contexts";
   fi

   #echo ===$1 Core Dumps===
   COREDUMP="`cat $myfile | grep "SystemCoreDump"`";
   if [ "$COREDUMP" != "" ]
   then
     echo "CORE DUMP: "$COREDUMP",file:"$myfile;
   fi

   #echo ===$1 Process terminating===
   SIGSEGV="`cat $myfile | grep "(SIGSEGV)"`";
   if [ "$SIGSEGV" != "" ]
   then
     echo "SIGSEGV: "$SIGSEGV",file:"$myfile;
   fi

done
