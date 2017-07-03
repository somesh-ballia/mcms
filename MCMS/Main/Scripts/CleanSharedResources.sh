#!/bin/bash 

###################################################################################
#If not running 'stand-alone' but as part of soft mcu - don't clean resources
if [[ ${SOFT_MCU_FAMILY} == "YES" ]]; then
	echo "Not runnning CleanSharedResources.sh under soft mcu" 
	exit 1
fi
###################################################################################

export SHORT_NAME=${LOGNAME:0:8}
# remove semaphores 
IDS=`ipcs -s | grep $SHORT_NAME| awk '{print $2}' 2> /dev/null` 
for i in $IDS ; do ipcrm -s $i ; done 


# remove shared memory 
IDS=`ipcs -m | grep $SHORT_NAME| awk '{print $2}' 2> /dev/null` 
for i in $IDS ; do ipcrm -m $i; done 


# remove message queues 
IDS=`ipcs -q | grep $SHORT_NAME| awk '{print $2}' 2> /dev/null` 
for i in $IDS ; do ipcrm -q $i; done 

# Doing cleanup the right way
ipcs -m | tail -n +4 | tr -s ' ' | cut -d ' ' -f2 | xargs -i ipcrm -m {} >& /dev/null
ipcs -s | tail -n +4 | tr -s ' ' | cut -d ' ' -f2 | xargs -i ipcrm -s {} >& /dev/null
ipcs -q | tail -n +4 | tr -s ' ' | cut -d ' ' -f2 | xargs -i ipcrm -q {} >& /dev/null

