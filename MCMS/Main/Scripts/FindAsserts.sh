#!/bin/sh

grep "#-- EXPECTED_ASSERT" $1 | while read line
do
    openParen=$(expr index "$line" "\(")
    closeParen=$(expr index "$line" "\)")
    let lengthOfNum=$closeParen-$openParen
    let "lengthOfNum -= 1"
    numOfAssert=${line:$openParen:$lengthOfNum}
    let "closeParen += 1"
    errMsg=${line:$closeParen}
    res=0
    for log_file_created in `ls LogFiles/Log_*.txt`
    do
	tmpNum=`grep -c "$errMsg" $log_file_created`
	let "res += $tmpNum"
    done
    
    if [ "$res" != "$numOfAssert" ]
    then
	echo ""$1 "Error: Find $res lines while excpecting to find $numOfAssert lines"
	echo "---------------------------------------------------"
	echo $errMsg 
	exit 105
    fi
done

if [ "$?" != "0" ]
then
    exit 105
fi

