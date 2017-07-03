#/bin/sh

function PrintHeader ()
{
	echo "<html>"

	echo "<title>" "$1" " test report</title>"
	echo "<head><META HTTP-EQUIV=\"refresh\" CONTENT=\"5\"></head>"
	echo "<body>"

	echo "$1" " test report" "<br><br><br><br>"
	echo "Generated at" $(date) "<br><br>"
	echo $(cat VERSION.LOG  | grep "Version:" | tail -n1) "<br><br>"
	echo "Host name:" $(hostname) "<br><br>"
	echo "Folder:" $(pwd) "<br><br>"
	echo "<table border=\"1\" cellpadding=\"5\" cellspacing=\"5\" width=\"100%\">"
	echo "<th>Test name</th><th>Group</th><th>Status</th><th>Valgrinrd</th>"
}
function PrintStatistics
{
	echo "<table border=\"1\" cellpadding=\"5\" cellspacing=\"5\" width=\"100%\">"
	echo "<th>Group name</th><th>Passed</th><th>Failed</th><th>Valgrind Passed</th><th>Valgrind Failed</th>"
	for group in $NIGHT_FOLDER/groups/*
	do
		echo "<tr>"
		echo "<td>" $(basename $group) "</td>"
		echo "<td align=center>" $(cat $group/passed.txt 2> $NULL | wc -l  2> $NULL) "</td>"
		echo "<td align=center>" $(cat $group/failed.txt 2> $NULL | wc -l  2> $NULL) "</td>"
		echo "<td align=center>" 0 "</td>"
		echo "<td align=center>" 0 "</td>"
		echo "</tr>"
	done	
	echo "<tr>"
	echo "<td><b>" TOTAL: "</b></td>"
	echo "<td align=center><b>"$TESTS_PASSED"</b></td>"
	echo "<td align=center><b>"$TESTS_FAILED"</b></td>"
	echo "<td align=center><b>"0"</b></td>"
	echo "<td align=center><b>"0"</b></td>"
	echo "</tr>"

	echo "</table>"
	echo "<br><br>"
}

function StopMcu
{
	echo "Shuting down..."
	./Scripts/soft.sh stop
	echo "Stoped"
}

if [ "$1" == "" ]
then
	echo "usage: SoftMcuTests.sh MODE"
	echo "MODE is VERSION or NIGHT"
	exit 1
fi

NIGHT_FOLDER=$MCU_HOME_DIR/mcms/LogFiles/softMcuNightResults
NULL=/dev/null
rm -Rf $NIGHT_FOLDER
mkdir $NIGHT_FOLDER

TESTS_DISABLED=0
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_AMOUNT=0
TESTS_PASSEDV=0
TESTS_FAILEDV=0
TESTS_AMOUNTV=0

TestResults=$MCU_HOME_DIR/mcms/LogFiles/softMcuTestResults
rm -Rf $TestResults
mkdir -p $TestResults

REPORTF=$TestResults/$1.html
PrintHeader $1 > $REPORTF

echo "Running "$1 "tests..."
echo "Starting Soft MCU..."
./Scripts/soft.sh start_vm > $NULL 2>&1  &
#./Scripts/soft.sh start > $NULL 2>&1 &


sleep_times=20
echo "waiting for ApacheModule ..."

for i in `seq 1 $sleep_times`;
    do
        printf "."
	ps -e | grep ApacheModule > /dev/null 2>&1
	if [ $? == 0 ]
	then
		break
	fi
	sleep 3
    done
echo " "
sleep 7
echo "Wait for Startup..."
./Scripts/WaitForStartup.py 80
if [ $? != 0 ]
then
	echo "SoftMcu did not start in Normal state"
	StopMcu
	exit 1
fi
sleep 5
 
for test_file_name in `grep -r -l  "# # NAME" Scripts/ | grep "\.py$"`
do
	#echo $test_file_name
	NAME=`grep "# # NAME" $test_file_name | awk -F ' ' '{ print $4 }'`
	OWNER=`grep "# # OWNER" $test_file_name | awk -F ' ' '{ print $4 }'`
	GROUP=`grep "# # GROUP" $test_file_name | awk -F ' ' '{ print $4 }'`
	NIGHT=`grep "# # NIGHT" $test_file_name | awk -F ' ' '{ print $4 }'`
	VERSION=`grep "# # VERSION" $test_file_name | awk -F ' ' '{ print $4 }'`
	RESTART_AFTER=`grep "# # VRESTART_AFTER" $test_file_name | awk -F ' ' '{ print $4 }'`

	if [ $1 == NIGHT -a $NIGHT == "TRUE" ] || [ $1 == VERSION -a $VERSION == "TRUE" ]
	then
		rm -Rf 	$TestResults/$NAME
		mkdir -p $TestResults/$NAME
		TEST_FOLDER=$NIGHT_FOLDER/tests/$NAME
		mkdir -p $TEST_FOLDER
		Scripts/Header.sh  $test_file_name
		START=$(date +%s)
		sleep 2
		$test_file_name > $TestResults/$NAME/$NAME.$1.log
		Status=$?
		END=$(date +%s)
		DURATION=$(($END-$START))
		echo " "
		GROUP_DIR=$NIGHT_FOLDER/groups/$GROUP
		mkdir -p "$GROUP_DIR"
		echo "<tr>" >> $REPORTF
		echo "<td width=\"50%\">"$NAME"</td>" >> $REPORTF
		echo  "<td>"$GROUP"</td>" >> $REPORTF
		echo "status " $NAME $?
		if [ $Status == 0 ]
		then
			echo $NAME "test Succeed"
			echo $NAME >> "$GROUP_DIR/passed.txt"
			echo "<td bgcolor=\"Green\">pass - " $DURATION" seconds</td>" >> $REPORTF
			let TESTS_PASSED=$(($TESTS_PASSED + 1))
		else
			echo $NAME "test Failed"
			echo "Test log file: " $TestResults/$NAME/$NAME.$1.log
			echo $NAME >> "$GROUP_DIR/failed.txt"
			echo "<td bgcolor=\"Red\">failed</td>" >> $REPORTF
			echo "error" $Status
			let TESTS_FAILED=$(($TESTS_FAILED + 1))
		fi
		
		echo "</tr>" >> $REPORTF

		current_log_file=`find LogFiles -type f`
		Bin/McuCmd @flush Logger > $NULL 2>&1 
		sleep 2
		mv $current_log_file $TestResults/$NAME > $NULL 2>&1
		
		#if [ $1 == NIGHT ] || [ $RESTART_AFTER == "TRUE" ]
		#then
		#	./Scripts/soft.sh start_vm
		#fi
	fi
done

echo "</table>" >> $REPORTF
echo "<br><br>" >> $REPORTF
sleep 2

StopMcu

PrintStatistics >> $REPORTF
echo "</body></html>" >> $REPORTF

exit $TESTS_FAILED
