#!/bin/sh


# $1 is valgrind file name
# $2 is Script name
# $3 is process under valgrind


#echo "AnalizeValgrindFile: `date` ; `hostname` ; `whoami` ; $1 ; $2 ; $3"

LEAK="`cat $1 | grep "definitely lost:" | grep -v "definitely lost: 0 bytes"`";
HAVE_ERROR="NO"
HAVE_LEAK=0

if [ "$LEAK" != "" ]
then
HAVE_LEAK=1
# echo "LEAK: "$LEAK",file:"$1;
fi
   
#echo ===$1 Memeory Errors===
MEMERR="`cat $1 | grep "ERROR SUMMARY:" | grep -v "0 contexts"`";
if [ "$MEMERR" != "" ]
then
    HAVE_ERROR="YES"
    #echo "MEMORY ERROR: "$MEMERR",file:"$1;
fi

#echo "HAVE_ERROR $HAVE_ERROR HAVE_LEAK $HAVE_LEAK"
if [ "$HAVE_ERROR" == "YES" ] 
then
	if [ "$HAVE_LEAK" -eq 1 ]
	then 
				echo "13"
				exit 13
				
	fi			
fi

if [ "$HAVE_ERROR" == "YES" ]
then 
				echo "12"
				exit 12
				
fi

if [ "$HAVE_LEAK"  -eq 1 ]
then 
				echo "11"
				exit 11
				
fi
echo "10"
exit 10

