#!/bin/sh
# automated report script
# this script creates a report and send it via email
# 1 = The full path to night test results
# 2 = The full path to the folder which will hold all diagnostic results
# 3 = The full path to the parsing scripts folder
# 4 = The specified CC baseline whihch are checking
# 5 = The mailing address of the result reprot
# 6 = The name of the testing version (Beta,Carmel...)
# 7 = The full path to the mail output  location (usually same as 1)
# 8 = Projetc Name 
# 9 = Card Type (e.g. MPMX)
# 10 = Mode (usually DEFAULT)

# shalom
echo "$0 : `date +"%Y%m%d%H%M%S"` : `hostname -s` : `whoami`"
    echo $# params: $@

#load enviroment variables
. ~/.bashrc

# not in used. 
#INPUTDIR=$PWD/$1
#OUTPUTDIR=$PWD/$2
OUTPUTDIR=$PWD/$1
DAYDIR=$2
FULLDAYDIR=$OUTPUTDIR/$2
TOOLSDIR=$PWD/$3
CC_BASELINE=$4
MAIL_ADDR=$5

MAIL_OUTPUTDIR=$PWD/$6

MCMS_MAIN_PATH=$PWD/${7}

#echo INPUTDIR=$INPUTDIR
echo OUTPUTDIR=$OUTPUTDIR
echo TOOLSDIR=$TOOLSDIR
echo CC_BASELINE=$CC_BASELINE
echo MAIL_ADDR=$MAIL_ADDR
echo MAIL_OUTPUTDIR=$MAIL_OUTPUTDIR
echo MCMS_MAIN_PATH=$MCMS_MAIN_PATH

echo mkdir -pmgo-w $OUTPUTDIR
mkdir -pmgo-w $OUTPUTDIR

#cd $OUTPUTDIR/`date +%A`
cd $FULLDAYDIR
OUTDIR=`pwd`

# compose common report file
#cat */test_report.txt > $OUTDIR/test_report.txt 

#create summery report for valgrind
#report includes all valgrind errors reported in all process for current day scripts
echo `date`: start of report generation

cd $OUTDIR

rm -f full_valgrind_report.txt

echo start create full_valgrind_report report
find | grep .pid > full_valgrind_report.txt

echo Create Failed and Passed list of tests

# fixed: PassedScripts now count only passed with no valgrind!!
#cat ""`find . -name ScriptResult.log ` | grep PASSED > PassedScripts.log
echo cat ""`find . -name ScriptResult.log ` | grep 'under_valdgrind  PASSED' > PassedScripts.log
cat ""`find . -name ScriptResult.log ` | grep 'under_valdgrind  PASSED' > PassedScripts.log
cat ""`find . -name ScriptResult.log ` | grep 'py  PASSED' > PassedScriptsNoValgrind.log
cat ""`find . -name ScriptResult.log ` | grep 'under_valdgrind  FAILED' > FailedScripts.log
cat ""`find . -name ScriptResult.log ` | grep 'py  FAILED' > FailedScriptsNoValgrind.log
cat ""`find . -name ScriptResult.log ` | grep 'under_profiler  PASSED' > PassedProfilerScripts.log
cat ""`find . -name ScriptResult.log ` | grep 'under_profiler  FAILED' > FailedProfilerScripts.log


echo >> $OUTDIR/test_report.txt

echo create proccess day report

rm -f $OUTPUTDIR/short_report.txt

#add core dumps and failed scripts summary
echo `date`: add core dumps to the report

cd $OUTDIR

#yaela - deleted crlist is not exist on this phase..
#if [ -s $OUTPUTDIR/crlist.txt ]; then
#	cat $OUTPUTDIR/crlist.txt >> $OUTDIR/test_report.txt
#fi

echo +-------------------------------------------------------------- >> $OUTDIR/test_report.txt
if [ -s $OUTDIR/FailedScripts.log ] || [ -s $OUTDIR/PassedScripts.log ] ||
	[ -s $OUTDIR/FailedScriptsNoValgrind.log ] || [ -s $OUTDIR/PassedScriptsNoValgrind.log ]; then
	echo "# Test Results Summary:" >> $OUTDIR/test_report.txt
	if [ -s $OUTDIR/FailedScriptsNoValgrind.log ]; then
		echo "* "`grep -c "" $OUTDIR/FailedScriptsNoValgrind.log` tests Failed - no valgrind >> $OUTDIR/test_report.txt
	fi
	if [ -s $OUTDIR/PassedScriptsNoValgrind.log ]; then
		echo " "`grep -c "" $OUTDIR/PassedScriptsNoValgrind.log` tests Passed - no valgrind >> $OUTDIR/test_report.txt
	fi
	if [ -s $OUTDIR/FailedScripts.log ]; then
#		echo "* "`grep -c "" $OUTDIR/FailedScripts.log` tests Failed Under Valgrind >> $OUTDIR/test_report.txt
               echo "* "`grep -c "" $OUTDIR/FailedScripts.log` test runs Failed Under Valgrind >> $OUTDIR/test_report.txt

	fi
	if [ -s $OUTDIR/PassedScripts.log ] ; then
#		echo " "`grep -c "" $OUTDIR/PassedScripts.log` test Passed Under Valgrind >> $OUTDIR/test_report.txt
                echo " "`grep -c "" $OUTDIR/PassedScripts.log` test runs Passed Under Valgrind >> $OUTDIR/test_report.txt

	fi
		
#	if [ -s $OUTDIR/FailedScripts.log ] || [ -s $OUTDIR/FailedScriptsNoValgrind.log ]; then
#		echo +-------------------------------------------------------------- >> $OUTDIR/test_report.txt
#		echo "* Detailed failed tests list :" >> $OUTDIR/test_report.txt
#		cat $OUTDIR/FailedScripts.log >> $OUTDIR/test_report.txt
#		cat $OUTDIR/FailedScriptsNoValgrind.log >> $OUTDIR/test_report.txt
#	fi
#	if [ -s $OUTDIR/PassedScripts.log ] || [ -s $OUTDIR/PassedScriptsNoValgrind.log ]; then
#		echo +-------------------------------------------------------------- >> $OUTDIR/test_report.txt
#		echo "# Detaild passed tests list :" >> $OUTDIR/test_report.txt
#		cat $OUTDIR/PassedScripts.log >> $OUTDIR/test_report.txt
#		cat $OUTDIR/PassedScriptsNoValgrind.log >> $OUTDIR/test_report.txt
#	fi
fi

if [ -s $OUTDIR/FailedProfilerScripts.log ] ; then
	echo "* "`grep -c "" $OUTDIR/FailedProfilerScripts.log` tests Failed Under Profiler >> $OUTDIR/test_report.txt
fi
if [ -s $OUTDIR/PassedProfilerScripts.log ] ; then
	echo " "`grep -c "" $OUTDIR/PassedProfilerScripts.log` tests Passed Under Profiler >> $OUTDIR/test_report.txt
fi

echo "" $OUTDIR/test_report.txt

echo +-------------------------------------------------------------- >> $OUTDIR/test_report.txt





#add to short report
echo +-------------------------------------------------------------- >> $OUTPUTDIR/short_report.txt
echo "# Core Dumps Created:" >> $OUTPUTDIR/short_report.txt
echo >> $OUTPUTDIR/short_report.txt
cat $OUTPUTDIR/crlist.txt >> $OUTPUTDIR/short_report.txt
cat $OUTPUTDIR/crvalgrindlist.txt >> $OUTPUTDIR/short_report.txt

echo +-------------------------------------------------------------- >> $OUTPUTDIR/short_report.txt
echo "# Test Results Summary:" >> $OUTPUTDIR/short_report.txt
echo " "`grep -c "" $OUTDIR/FailedScripts.log` Scripts Failed >> $OUTPUTDIR/short_report.txt
echo " "`grep -c "" $OUTDIR/PassedScripts.log` Scripts Passed >> $OUTPUTDIR/short_report.txt
echo " "`grep -c "" $OUTDIR/FailedProfilerScripts.log` Profiler Scripts Failed >> $OUTPUTDIR/short_report.txt
echo " "`grep -c "" $OUTDIR/PassedProfilerScripts.log` Profiler Scripts Passed >> $OUTPUTDIR/short_report.txt

#create short report


$TOOLSDIR/create_process_day_report.sh $OUTPUTDIR $DAYDIR $TOOLSDIR >> $OUTDIR/test_report.txt

cat $OUTDIR/test_report.txt >> $OUTPUTDIR/short_report.txt

#rm -f $OUTPUTDIR/crlist.txt

cd $OUTPUTDIR

if [ -s process_valgrind_summary.txt ]; then
#	cat process_valgrind_summary.txt >> $OUTDIR/valgrind_report.txt
	echo "@@ for details see attached process_valgrind_summary.txt"
fi

# create exception trace summery report
echo `date`: create exception trace summary

echo >> short_report.txt
$TOOLSDIR/CreateExceptionFile.py $DAYDIR CreateExceptionFile.log >> short_report.txt
echo >> short_report.txt
if [ -s CreateExceptionFile.log ]; then
	cat CreateExceptionFile.log >> $OUTDIR/test_report.txt
fi

unix2dos $OUTDIR/test_report.txt

#create leaks report
echo `date` " : create leaks report"

cd $OUTPUTDIR


$TOOLSDIR/Parsing_Leaks.py $OUTDIR/test_report.txt Parsing_Leaks.log
#echo >> short_report.txt
echo >> $OUTDIR/test_report.txt
#echo "detected leaks report:" >> short_report.txt
if [ -s Parsing_Leaks.log ]; then
	echo +------------------------------------------------------------- >> $OUTDIR/test_report.txt
	echo "# Detected Leaks Report:" >> $OUTDIR/test_report.txt
	#cat Parsing_Leaks.log >> short_report.txt
	cat Parsing_Leaks.log >> $OUTDIR/test_report.txt
fi

echo "" $OUTDIR/test_report.txt

echo +------------------------------------------------------------- >> $OUTDIR/test_report.txt

echo "# Scripts Run Time:" >> $OUTDIR/test_report.txt

$TOOLSDIR/AnalyzaScriptsRunningTime.sh  >> $OUTDIR/test_report.txt

echo +-------------------------------------------------------------- >> $OUTDIR/test_report.txt


unix2dos $OUTDIR/test_report.txt

#add owners list to report

cd $OUTPUTDIR

$TOOLSDIR/owners.py > owners_list.txt
cat owners_list.txt >> short_report.txt

echo The Source Control Revision is $CC_FULL_BASELINE 
#edit the mail format
rm -rf $OUTPUTDIR/mail.txt

#echo Nightly MCMS Carmel test results summary >> $OUTPUTDIR/mail.txt
#echo >> $OUTPUTDIR/mail.txt
#Relevant to clear case 
#grep MCMS $MCMS_MAIN_PATH/VERSION.LOG | grep Version: | tail -1 >> $OUTPUTDIR/mail.txt

#echo >> $OUTPUTDIR/mail.txt

RelativeURL=`svn info $MCMS_MAIN_PATH | grep Relative | grep URL`

echo -n "$RelativeURL "  >> $OUTPUTDIR/mail.txt

echo   >> $OUTPUTDIR/mail.txt

CC_FULL_BASELINE=$CC_BASELINE
echo -n "@ Source Control Revision: " >> $OUTPUTDIR/mail.txt

echo $CC_FULL_BASELINE >> $OUTPUTDIR/mail.txt

date >> $OUTPUTDIR/mail.txt
echo >> $OUTPUTDIR/mail.txt
cat owners_list.txt >> $OUTPUTDIR/mail.txt
cat $OUTDIR/test_report.txt >> $OUTPUTDIR/mail.txt
#echo +-------------------------------------------------------------- >> $OUTPUTDIR/mail.txt

if [ -s $OUTPUTDIR/crlist.txt ]; then
	echo "# Core Dumps Created:" >> $OUTPUTDIR/mail.txt
	echo >> $OUTPUTDIR/mail.txt
	cat $OUTPUTDIR/crlist.txt >> $OUTPUTDIR/mail.txt
fi

if [ -s $OUTPUTDIR/crvalgrindlist.txt ]; then
        echo "# Valgrind Core Dumps Created:" >> $OUTPUTDIR/mail.txt
        echo >> $OUTPUTDIR/mail.txt
        cat $OUTPUTDIR/crvalgrindlist.txt >> $OUTPUTDIR/mail.txt
fi

if [ -s $OUTPUTDIR/Log_Parser_Results.txt ]; then
	#echo +-------------------------------------------------------------- >> $OUTPUTDIR/mail.txt
	echo "# Log Analyze Table: " >> $OUTPUTDIR/mail.txt
	echo >> $OUTPUTDIR/mail.txt
	cat $OUTPUTDIR/Log_Parser_Results.txt >> $OUTPUTDIR/mail.txt
fi

VERSION_NAME="Unknown branch"
if [[ $RelativeURL =~ "/branches/" ]]
then
	branchNum=`expr match "$RelativeURL" 'Relative URL: ^/Main/branches/\([0-9]*\)/'`
	                
	VERSION_NAME="FSN-$branchNum"
else
	if [[ $RelativeURL =~ "/trunk/" ]]
	then
		VERSION_NAME="Main"
	fi                             
fi


if [ "$MAIL_ADDR" == "NoMail" ]
then
	echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	echo "~~~~~~  The script doesn't send mail  ~~~~~~~"
	echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
else
	echo Send report to: $MAIL_ADDR
	echo $TOOLSDIR/sendhtmlreport.py $MAIL_ADDR $OUTPUTDIR $VERSION_NAME $CC_FULL_BASELINE $MAIL_OUTPUTDIR 	2>&1 | tee sendhtmlreport.log

	THISHOST=$( hostname )
	
	if [ "$THISHOST" == "lnx-vm-53" ]
	then
		MNGNT_MAIL="shimon.tanny@polycom.co.il,Kobi.Ginon@polycom.co.il,Oded.Har-Tal@polycom.co.il"
		CONF_MAIl="Anat.Gavish@polycom.co.il,Ron.Lazarovich@polycom.co.il,Yoella.Bourshan@polycom.co.il,Richer.Ding@polycom.com"
		CALL_CNTRL="Shmuel.Lange@polycom.co.il,Noa.Reiter@polycom.co.il,Ami.Noy@polycom.co.il"
		FRAMEWORK="Boris.Sirotin@polycom.co.il,Tsahi.Zilcha@polycom.co.il,Judith.Shuva-Maman@polycom.co.il"
		SYSTEM="ori.pugatzky@polycom.co.il"
		SW_DEP="Dovev.Liberman@polycom.co.il,Matvey.Digilov@polycom.co.il,Moshe.Lipsker@polycom.co.il,Shay.Weiss@polycom.co.il,Oded.Har-Tal@polycom.co.il,Bing.Yuan@polycom.com,shimon.tanny@polycom.co.il,Kobi.Ginon@polycom.co.il"
		
		$TOOLSDIR/sendhtmlreportV2.py "$MAIL_ADDR" "$OUTPUTDIR" "$DAYDIR" "$CC_FULL_BASELINE" "$VERSION_NAME" All 2>&1 | tee $OUTPUTDIR/sendhtmlreportV2_All.log
		#$TOOLSDIR/sendhtmlreportV2.py "$MNGNT_MAIL" "$OUTPUTDIR" "$DAYDIR" "$CC_FULL_BASELINE" "$VERSION_NAME" Management 2>&1 | tee $OUTPUTDIR/sendhtmlreportV2_MNGNT.log		
		#$TOOLSDIR/sendhtmlreportV2.py "$CONF_MAIlL" "$OUTPUTDIR" "$DAYDIR" "$CC_FULL_BASELINE" "$VERSION_NAME" Conference 2>&1 | tee $OUTPUTDIR/sendhtmlreportV2_CONF.log
		#$TOOLSDIR/sendhtmlreportV2.py "$CALL_CNTRL" "$OUTPUTDIR" "$DAYDIR" "$CC_FULL_BASELINE" "$VERSION_NAME" CallControl 2>&1 | tee $OUTPUTDIR/sendhtmlreportV2_CALLC.log
		#$TOOLSDIR/sendhtmlreportV2.py "$FRAMEWORK" "$OUTPUTDIR" "$DAYDIR" "$CC_FULL_BASELINE" "$VERSION_NAME"  Framework 2>&1 | tee $OUTPUTDIR/sendhtmlreportV2_FRAME.log
		
		$TOOLSDIR/sendhtmlreport.py "$MAIL_ADDR" "$OUTPUTDIR" "$DAYDIR" "$CC_FULL_BASELINE" "$VERSION_NAME" 2>&1 | tee sendhtmlreport.log		
	else
		$TOOLSDIR/sendhtmlreportV2.py "$MAIL_ADDR" "$OUTPUTDIR" "$DAYDIR" "$CC_FULL_BASELINE" "$VERSION_NAME" All 2>&1 | tee $OUTPUTDIR/sendhtmlreportV2_All.log
		$TOOLSDIR/sendhtmlreport.py "$MAIL_ADDR" "$OUTPUTDIR" "$DAYDIR" "$CC_FULL_BASELINE" "$VERSION_NAME" 2>&1 | tee sendhtmlreport.log								
	fi


#$TOOLSDIR/sendhtmlreport.py $MAIL_ADDR $OUTPUTDIR $DAYDIR $VERSION_NAME $CC_FULL_BASELINE $PROJECT_NAME $CARD_TYPE $MODE_TYPE 2>&1 | tee sendhtmlreport.log





fi

#send notice to owners only in Team-stream


#echo zip all directory  
#$TOOLSDIR/DotarIns.py  $OUTPUTDIR/`date +%A` &> DotarIns.log

echo `date`: end of night run report
