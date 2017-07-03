#!/bin/sh
NUM_OF_EXCEPTIONS=0

ProcessesForIgnoringAsserts=( "LicenseServer")

GetNumIgnoredAssertsAsserts()
{
ignoredAsserts=0

grep "ASSERT Code" $1 |( while read line
do
    process=`echo $line | awk '{ print $3 }'`

    processPos=$(expr index "$process" "\:")
    process=${process:$processPos}   
    for i in "${ProcessesForIgnoringAsserts[@]}"
    do
    if [ "$process" == "$i" ]
    then
      let "ignoredAsserts += 1"
    fi
    done   
done

return $ignoredAsserts)
}
 


for log_file_created in `ls LogFiles/Log_*.txt`
 do

    tmpNum=`grep -c "* EXCEPTION *" $log_file_created`    
    let "NUM_OF_EXCEPTIONS += $tmpNum"
    
    GetNumIgnoredAssertsAsserts $log_file_created
    numIgnoredAsserts=$?
    let "NUM_OF_EXCEPTIONS -= $numIgnoredAsserts"
 done

if [ "$NUM_OF_EXCEPTIONS" == "0" ]
then
    exit 0

fi

echo 
echo $COLORRED
echo "---------------------------------------------------------------------"
echo $1 ": Number of exceptions = " $NUM_OF_EXCEPTIONS 
echo "---------------------------------------------------------------------"
echo $TXTRESET
echo 

for log_file_created in `ls LogFiles/Log_*.txt`
 do
    grep -A5 -B0 "* EXCEPTION *" $log_file_created
done

exit $NUM_OF_EXCEPTIONS
