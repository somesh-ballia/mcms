echo "Sleeping 30 seconds before validate"
sleep 30

for machine in $(cat nt_machines.txt)
do
	echo $machine
	status=1
	until [ "$status" -eq "0" ]
	do
	ping -c 1 $machine > /dev/null
    status=`echo $?`
    echo $status
    if [ "$status" = "1" ];
    then
	    echo "The machine $machine seems to be down!"
    else
    	echo "The machine $machine is up"
    fi
	done
done

echo Done!
