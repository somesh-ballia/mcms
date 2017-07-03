#!/mcms/python/bin/python

import sys
import os
from time import *




def Sleep(timeToSleep):
    print "sleep " + str(timeToSleep) + " seconds"
    sleep(timeToSleep)

def RunCommand(commandToRun):
    print "----- run " + commandToRun
    os.system(commandToRun)





def ConfigSnmpd():
    command = "echo \"trap2sink localhost:8091 public \" >> /tmp/snmpd.conf"
    RunCommand(command)

    command = "kill -1 `ps -e | grep snmpd | awk '{print $1}' `"
    RunCommand(command)


def RunSnmpListener(outputFile):
    commandTrapListen = "Scripts/SNMPTrapListener.py > " + outputFile + " &"
    RunCommand(commandTrapListen)


def AddAlerts(alertNum):
    commandAddAlert = "Bin/McuCmd add_aa cdr " + str(alertNum) + " YES"
    RunCommand(commandAddAlert)


def CheckTrapArrival(outputFile, alertNum):
    outputFile2 = outputFile + "1"
#    command = "cat " + outputFile + " | grep \"Version: 2, type: TRAP\" | wc -l > " + outputFile2
    command = "cat " + outputFile + " | grep \"Error in Test\" | wc -l > " + outputFile2

    RunCommand(command)
    
    output = open(outputFile2)
    line = output.readline()
    output.close()
    
    command = "rm " + outputFile
    RunCommand(command)
    
    command = "rm " + outputFile2
    RunCommand(command)    

    numOfTraps = int(line)
    
    print "num of traps : " + str(numOfTraps)
    print "num of alerts: " + str(alertNum)
    
    if(numOfTraps <> alertNum):
        return False
    return True
    

def KillSnmpListener():
    commandKillTrapListener = "killall SNMPTrapListener.py"
    RunCommand(commandKillTrapListener)






#------------------------------------------------------------
#  Test
#------------------------------------------------------------



print "1. Config SNMPD"
ConfigSnmpd()

Sleep(5)


outputFile = "tmpTerminalCommandOutput.yura"

print "2. Run Snmp Trap Listener"
RunSnmpListener(outputFile)

Sleep(2)



alertNum = 10
print "3. Add " + str(alertNum) + " Alerts"
AddAlerts(alertNum)

Sleep(10)



print "4. Check Arrival of the Traps"
areTrapArrived = CheckTrapArrival(outputFile, alertNum)

if(areTrapArrived):
    message = "All " + str(alertNum)  +" traps were arrived, Very Good"
else:
    message = "Not All " + str(alertNum)  +" traps were arrived, Very Bad"

print ""
print message
print ""


print "5. Kill the Snmp Trap Listener"
KillSnmpListener()



if(areTrapArrived):
    exitStatus = 0
else:
    exitStatus = 1

sys.exit(exitStatus)


