#!/bin/sh

HOST=`hostname`
last_machine="NONE"

for machine in $(cat nt_machines.txt)
do
	if [[ $machine =~ "UU" ]] 
	then 
               echo "" 
	else
		if [ "$HOST" == "$machine" ]
		then
			last_machine=$machine
			echo "saving $last_machine to last"
		else
			echo $machine
			command="spawn ssh yglick@$machine; expect password ; send \"^T^T6t6t\n\" ; expect $; send \"sudo /usr/bin/reboot\n\"; expect password ; send \"^T^T6t6t\n\" ; interact"
	echo $command
			expect -c "$command"
		fi
	fi
done
	
if [ "$last_machine" != "NONE" ]
then
	command="spawn ssh yglick@$last_machine; expect password ; send \"^T^T6t6t\n\" ; expect $; send \"sudo /usr/bin/reboot\n\"; expect password ; send \"^T^T6t6t\n\" ; interact"
	expect -c "$command"
fi

echo Reboot Done!


