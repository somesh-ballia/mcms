#!/bin/sh 
#this script print a summary of all valgrind errors found per process in a day report
# $1 is the location of the report
# $2 is the location of the tools dir location


OUTPUTDIR=$1
#TOOLSDIR=$2
#DAYDIR=`date +%A`
DAYDIR=$2
TOOLSDIR=$3


cd $OUTPUTDIR

#this part print the results of the night scripts run
#echo "# Night Script Report for $DAYDIR" 
echo "# Functionality only - no valgrind"
$TOOLSDIR/passfail.sh $DAYDIR
echo +-----------------------------
echo "# Valgrind Report for $DAYDIR:"

PROCESS_TO_RUN="Auditor ApacheModule Authentication CDR CSApi CSMngr Cards ClientLogger Collector ConfParty
Configurator DNSAgent Demo Diagnostics EncryptionKeyServer EndpointsSim Faults
Gatekeeper GideonSim IPMCInterface Installer LogUtil Logger McmsDaemon McuCmd
McuMngr MplApi QAAPI Resource SipProxy SystemMonitoring TestClient"
	   
cd $OUTPUTDIR
find $DAYDIR/ -printf "CORE DUMP FILE:%p\n" | grep "\/core\." > crlist.txt 
find $DAYDIR/ -printf "CORE DUMP FILE:%p\n" | grep "\.core\." > crvalgrindlist.txt


rm -f tempfile.txt temp2file.txt process_valgrind_summary.txt > /dev/null
touch tempfile.txt  #create empty file
touch temp2file.txt #create empty file
echo  >> temp2file.txt

for process in $PROCESS_TO_RUN;
do
    echo "seaching process $process" >> temp2file.txt
    $TOOLSDIR/process_day_summary.sh $process $DAYDIR > tempfile.txt
    cat $OUTPUTDIR/crlist.txt | grep $process.core >> $OUTPUTDIR/tempfile.txt

    SGV=`grep -c "SIGSEGV" tempfile.txt`
    CORE=`grep -c "CORE DUMP FILE" tempfile.txt`
    MEMORY=`grep -c "MEMORY ERROR" tempfile.txt`
    LEAK=`grep -c "LEAK" tempfile.txt`

    ERRORFOUND=0
    printf "%-20s : " $process

    if [ "$SGV" != "0" ];
    then
	printf "%3d SEG FAULTS " $SGV
	ERRORFOUND=1
	echo " SEG FAULTS" >> temp2file.txt
    fi

    if [ "$CORE" != "0" ];
    then
        if [ "$ERRORFOUND" -gt 0 ]
        then 
            printf "and %3d CORE DUMPS " $CORE
        else
            printf "%3d CORE DUMPS " $CORE
        fi
	ERRORFOUND=1
	echo " CORE" >> temp2file.txt
    fi

    if [ "$MEMORY" != "0" ];
    then
        if [ "$ERRORFOUND" -gt 0 ]
        then
            printf "and %3d Scripts runs found MEMORY ERRORS " $MEMORY
        else 
		    printf "%3d Scripts runs found MEMORY ERRORS " $MEMORY
		fi
	ERRORFOUND=1
	echo " MEMORY" >> temp2file.txt
    fi
    if [ "$LEAK" != "0" ];
    then
        if [ "$ERRORFOUND" -gt 0 ]
        then
        	printf "and out of them %3d Scripts had also LEAKs" $LEAK
        else
		    printf "and out of them %3d Scripts had also LEAKs" $LEAK
		fi
	ERRORFOUND=1
	echo " LEAKS" >> temp2file.txt
    fi

    if [ "$ERRORFOUND" = "1" ];
    then
        echo -------------------$process-------------------------- >> temp2file.txt 
        PIDLIST=`find $DAYDIR | grep valgrind.$process`
        for FileToCat in  $PIDLIST;
        do
	    cat $FileToCat >> temp2file.txt
        done
        sleep 2
        echo >> temp2file.txt
    else
	printf " NO ERRORS "
    fi

    printf "\n"
done

mv temp2file.txt process_valgrind_summary.txt
