#! /usr/bin/expect -f

spawn ssh -o StrictHostKeychecking=no carmel@[lindex $argv 0]



expect "password"
send "Polycom1\r"
expect "carmel@"
#send "cd `readlink $MCU_HOME_DIR/tmp/mcms`\r"
send "echo run command  [lindex $argv 1]\r"
send "[lindex $argv 1]\r"
#send "echo start timeout for [lindex $argv 2]\r"
#send "sleep [lindex $argv 2]\r"
#sleep [lindex $argv 2]
#send "echo clean sipp process \r"
#send "/Carmel-Versions/TEMP/Sipp_Scripts/clean_sipp_tasks.sh\r"
sleep 3
#interact
