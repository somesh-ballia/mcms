#!/bin/sh

LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_10_YES_v100.0.cfs" # for 5 ports in license
#LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_20_YES_v100.0.cfs" # for 20 ports in license
#LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_40_YES_v100.0.cfs" # for 40 ports in license

export LD_LIBRARY_PATH=`echo $PWD`/Bin/
export ENDPOINTSSIM="NO"
export GIDEONSIM="NO"
Scripts/Integration/IntegrationCleanup.sh
usleep 200000

Scripts/BinLinks.sh

if [ "$1" ]
then
  rm -f Links/$1
  echo "#!/bin/sh" > Links/$1
  echo -n "valgrind --tool=memcheck --leak-check=yes --trace-children=yes" >> Links/$1
  echo "--suppressions=Scripts/ValgrindSup.txt --log-file=TestResults/$1 Bin/$1 & " >> Links/$1
  echo $1 "will run in valgrind"
  chmod u+x Links/$1
fi


   cp VersionCfg/License.cfs Cfg/
    if [ "$LICENSE_FILE" != "" ]
	then
	echo Copy $LICENSE_FILE
    chmod +w Cfg/License.cfs
	cp $LICENSE_FILE Cfg/License.cfs
    fi


if [ "$CLEAN_FAULTS" != "NO" ]
then
    echo Cleaning Faults directory
    rm -Rf Faults
    mkdir Faults
fi


echo "Running McmsDaemon"
Bin/McmsDaemon &
usleep 200000

if [ "$GIDEONSIM" == "YES" ]
then
    echo Running GideonSim
    Links/GideonSim&
    usleep 200000
fi

#if [ "$ENDPOINTSSIM" == "YES" ]
#then
#    echo Running EndpointsSim
#    Links/EndpointsSim&
#    usleep 200000
#fi

#usleep 500000
##/opt/polycom/apache/bin/apachectl start
##/opt/polycom/apache/bin/httpd -X

#if [ "$DISABLE_WATCH_DOG" == "YES" ]
#then
#    echo "Disabling watch dog (debug mode)"
#    Bin/McuCmd set mcms WATCH_DOG NO < /dev/null
#fi

sleep 2
exit 435
