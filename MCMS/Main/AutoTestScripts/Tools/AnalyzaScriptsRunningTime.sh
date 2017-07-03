#!/bin/sh


printf "%-60s %-30s %-30s %s \n" "Script Name" "Overall run time" "NoValgrind run time"  "Valdgrind processes script runs with"

cat ""`find . -name ScriptResult.log ` | grep 'Overall Running Time' > tmpOverAllScriptsRunningTime.txt


awk '
BEGIN { 
} 
{ 

	pattern = "^(.*): Overall Running Time - ([0-9]* sec) NoValgrind - ([0-9]* sec) ValdgrindProcesses -(.*)"


	if (match($0, pattern, arr))
	{
		printf("%-60s %-30s %-30s %-30s \n" ,arr[1], arr[2], arr[3], arr[4])

	}
}
' tmpOverAllScriptsRunningTime.txt


rm tmpOverAllScriptsRunningTime.txt

