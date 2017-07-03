#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
import sys
import os
from time import *
from McmsConnection import *
from SNMPUtils import *
c = McmsConnection()
from UsersUtils import *

def Sleep(timeToSleep):
    print "sleep " + str(timeToSleep) + " seconds"
    sleep(timeToSleep)

def RunCommand(commandToRun):
    print "----- run " + commandToRun
    os.system(commandToRun)


def ConvertRmxStatusToSnmpFormat(mcuStatus):
       snmpStatus = 3
       if mcuStatus == 0:
       #eMcuState_Startup:
            snmpStatus = 1
       if mcuStatus == 1: 
        #eMcuState_Normal
            snmpStatus = 0
       if mcuStatus == 4: 
        #eMcuState_Minor:
            Status = 2;
       return snmpStatus;


def ConfigSnmpdToLocalhost():
	#c = McmsConnection()
	#c.Connect()
	c = UsersUtilities()
	c.ConnectSpecificUser("SUPPORT","SUPPORT")
	c.LoadXmlFile('Scripts/TransSnmpSetWhiteList.xml')
	c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
	c.ModifyXml("SECURITY","TRAP_VERSION","snmpv1")
	c.ModifyXml("TRAP_DESTINATION","IP","127.0.0.1")
	c.ModifyXml("TRAP_DESTINATION","COMMUNITY_NAME",":8091 public")
	c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","public")
	c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
	c.Send()
def ConfigSnmpdToRemoveLocalhost():
	#c = McmsConnection()
	#c.Connect()
	c = UsersUtilities()
	c.ConnectSpecificUser("SUPPORT","SUPPORT")
	c.LoadXmlFile('Scripts/TransSnmpSetWhiteList.xml')
	c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
	c.ModifyXml("SECURITY","TRAP_VERSION","snmpv2")
	#c.ModifyXml("TRAP_DESTINATION","IP","127.0.0.1")
	#c.ModifyXml("TRAP_DESTINATION","COMMUNITY_NAME",":8091 public")
	c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","public")
	c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
	c.Send()
	
def EnableSNMP():  
    print "________2___Enable_snmp___________________"
    #c = McmsConnection()
    c = UsersUtilities()
    c.ConnectSpecificUser("SUPPORT","SUPPORT")
    #c.Connect()
    c.LoadXmlFile('Scripts/TransSnmpSet.xml')
    c.ModifyXml("SNMP_DATA","LOCATION","location_test")
    c.ModifyXml("SNMP_DATA","CONTACT_NAME","contact_name_test")
    c.ModifyXml("SECURITY","ACCEPT_ALL_REQUESTS","true")
    c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
    c.ModifyXml("SECURITY","TRAP_VERSION","snmpv2")
    c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","public")
    c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
    c.Send()

def ConfigSnmpd():
    command = "echo \"trap2sink 127.0.0.1:8091 public \" >> /tmp/snmpd.conf"
    RunCommand(command)
    command = "kill -1 `ps -e | grep snmpd | awk '{print $1}' `"
    RunCommand(command)

def Get_and_compare_MCU_state():
    snmpGet = SNMPGetUtils()
    #j = McmsConnection()
    #j.Connect()
    j = UsersUtilities()
    j.ConnectSpecificUser("SUPPORT","SUPPORT")
    #j.SendXmlFile("Scripts/GetMCU_State.xml")
    j.SendXmlFile("Scripts/GetMcuState.xml")
   # j.PrintLastResponse()
    http_MCU_state = j.xmlResponse.getElementsByTagName("MCU_STATE")
    http_MCU_status = http_MCU_state[0].getElementsByTagName("ID")[0].firstChild.data
    http_MCU_status_desc = http_MCU_state[0].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    #print "http_MCU_status: "+http_MCU_status+" ("+http_MCU_status_desc +")"
    mcuStatus = snmpGet.GetMCU_State()
    val =ConvertRmxStatusToSnmpFormat(mcuStatus)
    
    print "val :"+ str(val)
    #print "Get_snmp_MCU_State    : " + str(val)
    if ( str(val) <> http_MCU_status):
         print"oy oy oy ... wrong snmp_MCU_State "
         print "Get_snmp_MCU_State    : " + str(val) +" but http_MCU_status: "+http_MCU_status+" ("+http_MCU_status_desc +")"
         exit (1)
    else:
         print "http_MCU_status: "+http_MCU_status+" ("+http_MCU_status_desc +")"
        
    return (val)
    
   
    
def RunSnmpListener(outputFile):
    commandTrapListen = "Scripts/SNMPTrapListener.py > " + outputFile + " &"
    RunCommand(commandTrapListen)


def AddAlerts(alertNum):
    commandAddAlert = "Bin/McuCmd add_aa cdr " + str(alertNum) + " YES"
    RunCommand(commandAddAlert)

def RemoveAlerts(alertNum):
    commandAddAlert = "Bin/McuCmd rm_aa cdr " + str(alertNum)
    RunCommand(commandAddAlert)

def CheckTrapArrival(outputFile, alertNum):
    outputFile2 = outputFile + "1"
#    command = "cat " + outputFile + " | grep \"Version: 2, type: TRAP\" | wc -l > " + outputFile2
    command = "cat " + outputFile + " | grep \"Error in Test\" | wc -l > " + outputFile2

    RunCommand(command)
    
    output = open(outputFile2)
    line = output.readline()
    output.close()
    
    command = "rm " + outputFile2
    RunCommand(command)    

    numOfTraps = int(line)
    
    print "num of traps : " + str(numOfTraps)
    print "num of alerts: " + str(alertNum)
    
    if(numOfTraps < alertNum):
        return False
    return True
    

def KillSnmpListener():
    commandKillTrapListener = "killall SNMPTrapListener.py"
    RunCommand(commandKillTrapListener)


#------------------------------------------------------------
#  Test
#------------------------------------------------------------
p = UsersUtilities()
p.Connect()
p.DelUser("POLYCOM") 

print "1.---Enable SNMP -------------------------------------" 
EnableSNMP()
sleep (14)

KillSnmpListener()
print "2.---Config SNMPD trap destination to local host---"
ConfigSnmpdToLocalhost()

Sleep(10)
print "3.---Test rmxStatus (should be Normal)----------------" 
MCUstatus=Get_and_compare_MCU_state()
print "***" + str(MCUstatus)+"***"


outputFile = "tmpTerminalCommandOutput1.shl"
print "4.---Run Snmp Trap Listener-------------------------"
RunSnmpListener(outputFile)

Sleep(10)

alertNum = 10
print "5.---Add " + str(alertNum) + " Alerts---------------"
AddAlerts(alertNum)

Sleep(10)
print "6.---Check Arrival of the Traps---------------------"
areTrapArrived = CheckTrapArrival(outputFile, alertNum)

if(areTrapArrived):
    message = "All " + str(alertNum)  +" traps were arrived, Good start "
    print message
else:
    message = "Not All " + str(alertNum)  +" traps were arrived, Test Faild"
    print message
    exit(1)
print "7.---test rmxStatus (should be major)---------------------"  
MCUstatus=Get_and_compare_MCU_state()
print "***" + str(MCUstatus)+"***"

print "8---Remove " + str(alertNum) + " Alerts---------------------"  
RemoveAlerts(alertNum)
Sleep(10)

print "9.---Check Arrival of the Traps---------------------"
#outputFile = "tmpTerminalCommandOutput2.shl"
areTrapArrived = CheckTrapArrival(outputFile, 20)

if(areTrapArrived):
    message = "All " + str(alertNum)  +" traps were arrived, Test Faild"
else:    
    message = "\n\n ----No traps were arrived, Good start"
print "10---test rmxStatus (should be Normal)---------------------"     
MCUstatus=Get_and_compare_MCU_state()
print "----------------" + str(MCUstatus)+"--------------------"

print "11.---Kill the Snmp Trap Listener-------------------"
KillSnmpListener()
if(areTrapArrived):
    print "--"
else:
    sys.exit(1)
#command = "rm " + outputFile
#RunCommand(command)    

outputFile = "tmpTerminalCommandOutput3.shl"

print "12.---Run Snmp Trap Listener------------------------"
RunSnmpListener(outputFile)

Sleep(2)

print "13.---Config SNMPD  to remove the trap destination to local host"
ConfigSnmpdToRemoveLocalhost()


Sleep(5)

print "14.---Add " + str(alertNum) + " Alerts---------------"
AddAlerts(alertNum)

Sleep(10)

print "15.---Check Arrival of the Traps---------------------"
areTrapArrived = CheckTrapArrival(outputFile, 0)

if(areTrapArrived):
    message = "\n\n ----No traps were arrived, Test Passed !!!!!!!!!!!!!"
else:
    message = "All " + str(alertNum)  +" traps were arrived, Test Faild"

print message
print "16. Kill the Snmp Trap Listener"
KillSnmpListener()

if(areTrapArrived):
    exitStatus = 0
else:
    exitStatus = 1
command = "rm " + outputFile
RunCommand(command)    

sys.exit(exitStatus)

