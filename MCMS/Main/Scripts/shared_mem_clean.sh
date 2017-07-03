#!/bin/sh

###################################################################################
#If not running 'stand-alone' but as part of soft mcu - don't clean resources
if [[ ${SOFT_MCU_FAMILY} == "YES" ]]; then
	echo "Not runnning shared_mem_clean.sh under soft mcu"
	exit 1
fi
###################################################################################

rm -f $MCU_HOME_DIR/tmp/shared_memory/*

for flag in m
do
        for id in `ipcs -$flag | egrep -v '^(-----|key|$)' | cut -d' ' -f2`
        do
                case $flag in
                        m)
                                type=shm
                                ;;
                esac
                if [ "$type" != "" ] && [ $id -gt 1 ]; then
                        ipcrm $type $id
                fi
        done
done 

rm -f $MCU_HOME_DIR/tmp/shared_memory/*

# remove apahce semaphores
ipcs -s | grep $USER | perl -e 'while (<STDIN>) { @a=split(/\s+/); print `ipcrm sem $a[1]`}'
