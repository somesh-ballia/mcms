
#!/bin/sh


echo "EPSOFF"
#EPSOFF
unset ENDPOINTSSIM 

echo "export CLEAN_CFG=NO"
export CLEAN_CFG=NO

#echo "make MEDIA_MANAGER"
#make MEDIA_MANAGER

echo "export CG=YES"
export CG=YES

echo "make active"
make active

Scripts/Startup.sh





#if [ "$1" ]
#then
#    echo Waiting extra 2 seconds since one of the processes runs under valgrind
#   sleep 2
#fi

#if [ "$DISABLE_WATCH_DOG" == "YES" ]
#then
#    echo "Disabling watch dog (debug mode)"
#    Bin/McuCmd set mcms WATCH_DOG NO < /dev/null
#fi

#echo "Waiting 1 second (Otherwise login fails)..."
#sleep 1

#echo "and another 3 seconds (for gk_registraion)..."
#sleep 3


