#!/bin/sh

# $2 is the test result
exiterror=$2

 if [[ "$SOFT_MCU" == "YES" || "$SOFT_MCU_EDGE" == "YES" || "$SOFT_MCU_MFW" == "YES" ]]; then    
    Scripts/soft.sh stop || exiterror=1233
else	
    Scripts/Destroy.sh || exiterror=1233
fi

if [ "ApacheModule" == "$3" ]
   then
   echo "sleep 10 seconds - waiting for valgrind's report"
   sleep 10
fi


Scripts/Flush.sh

#---------------------------------------------------------
scriptName=${1%%_With_*}
data=$(grep "#-- SKIP_ASSERTS" $scriptName)
if [ "$data" == "" ]
   then
   data=$(grep "#-- EXPECTED_ASSERT" $scriptName)
   if [ "$data" == "" ] #in case no error messages in test script
   then
      Scripts/SumExceptions.sh $1 || exiterror=1234
   else
      Scripts/FindAsserts.sh $scriptName || exiterror=1235
   fi
fi
#-----------------------------------------------------------


if [ $exiterror == "0" ]
then
    echo
	echo $COLORGREEN"||  "
	echo "||  " $(basename $1 .sh) "Test PASSED"
	echo "||  "$TXTRESET
	echo
#was    echo "Test" $1 "Passed"
else
    #echo $TXTBOLD$COLORWHITE$BGCOLORGREEN"EndTest has following params: "$@$TXTRESET
    Scripts/ErrorMsg.sh $1 "Test FAILED $exiterror"
fi  

Scripts/batchkill.sh 

if  [ $exiterror == "0" ]
then
    echo $(basename $1 .sh) " PASSED" >> TestResults/ScriptResult.log
else
    if  [ $exiterror == "1233" ]
    then
      echo $COLORRED$(basename $1 .sh) "FAILED (process kill failed)"$TXTRESET
      echo $(basename $1 .sh) " FAILED (process kill failed)" >> TestResults/ScriptResult.log
    fi
    if  [ $exiterror == "1234" ]
    then
      echo $COLORRED$(basename $1 .sh) "FAILED (exception traces found)"$TXTRESET
      echo $(basename $1 .sh) " FAILED (exception traces found)" >> TestResults/ScriptResult.log
    fi

    if  [ $exiterror == "1235" ]
    then
	echo $COLORRED$(basename $1 .sh) "FAILED (EXPECTED_ASSERTS Could not be found)"$TXTRESET
	echo $(basename $1 .sh) " FAILED (EXPECTED_ASSERTS Could not be found)" >> TestResults/ScriptResult.log
    fi
    
    if  [ $exiterror != "1233" -a $exiterror != "1234" -a $exiterror != "1235" ] 
    then
      echo $COLORRED$(basename $1 .sh) "FAILED (script found an error)"$TXTRESET
      echo $(basename $1 .sh) " FAILED (script found an error)" >> TestResults/ScriptResult.log
    fi
fi

#echo $COLORCYAN$(basename $1 .sh) "EndTest.sh returns $exiterror"$TXTRESET
exit $exiterror



