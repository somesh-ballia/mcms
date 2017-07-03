#!/bin/sh

if [ $# -lt 2 ];
then
	echo "Usage: SummarizeResults.sh <RESULT_PATH> <DAYDIR> <LOGS_DIRECTORY> <VERSION> <MAIL>"
	exit
fi
RESULT_PATH=$1
DAYDIR=$2

INPDIR='TestResults'

if [ $# -gt 4 ];
then
	echo "Taking paramters."		
	LOGS_DIRECTORY=$3
	VERSION=$4
	MAIL=$5

else
	echo "Taking default parameters."		
	LOGS_DIRECTORY='log_files_for_analyze'
	VERSION='NA'
	MAIL='Yael.Azulay@polycom.co.il'

fi

echo "SuumarizeResults: INPDIR: $INPDIR RESULT_PATH: $RESULT_PATH DAYDIR $DAYDIR LOGS_DIRECTORY: $LOGS_DIRECTORY VERSION: $VERSION MAIL: $MAIL"		

TEST_RESULT_PATH=$RESULT_PATH/$DAYDIR

echo "Execute log analyzer"
cd $RESULT_PATH


../AutoTestScripts/Tools/McmsLogParser/McmsLogParser $LOGS_DIRECTORY/
 
mv Output.txt Log_Parser_Results.txt
cd -
echo "Finish log analyzer"

AutoTestScripts/Tools/night_run_report.sh "$RESULT_PATH" "$DAYDIR" "AutoTestScripts/Tools/" "$VERSION" "$MAIL" "$RESULT_PATH" .

echo "End SummarixeResults"

