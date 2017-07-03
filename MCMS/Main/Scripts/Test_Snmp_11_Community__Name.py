#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_11
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

def ConfigSnmpdToLocalhost_Community_name():
	#c = McmsConnection()
	#c.Connect()
	c = UsersUtilities()
	c.ConnectSpecificUser("SUPPORT","SUPPORT")
	c.LoadXmlFile('Scripts/TransSnmpSetWhiteList.xml')
	c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
	c.ModifyXml("SECURITY","SEND_AUTHENTICATION_TRAPS","true")
	
	c.ModifyXml("SECURITY","ACCEPT_ALL_REQUESTS","true")
	c.ModifyXml("SECURITY","TRAP_VERSION","snmpv1")
	c.ModifyXml("TRAP_DESTINATION","IP","127.0.0.1")
	c.ModifyXml("TRAP_DESTINATION","COMMUNITY_NAME","public 8091")
	c.ModifyXml("COMMUNITY_PERMISSION","COMMUNITY_NAME","public1")
	c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
	c.Send()

def ConfigSnmpdToLocalhost_Community_name2(snmpv="snmpv1"):
	#c = McmsConnection()
	#c.Connect()
	c = UsersUtilities()
	c.ConnectSpecificUser("SUPPORT","SUPPORT")
	c.LoadXmlFile('Scripts/TransSnmp_trap__authentication.xml')
	c.ModifyXml("TRAP_DESTINATION","IP","127.0.0.1")
	c.ModifyXml("TRAP_DESTINATION","COMMUNITY_NAME","public 8091")
	c.ModifyXml("SECURITY","TRAP_VERSION","snmpv1")
	#c.ModifyXml("COMMUNITY_PERMISSION","COMMUNITY_NAME","public")
	#c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
	c.Send()	
def ConfigSnmpdToLocalhost_Community_name1():
	#c = McmsConnection()
	#c.Connect()
	c = UsersUtilities()
	c.ConnectSpecificUser("SUPPORT","SUPPORT")
	c.SendXmlFile('Scripts/TransSnmp_trap__authentication.xml')
	
"""
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
"""
def ConfigSnmpdToRemoveLocalhost():
	#c = McmsConnection()
	#c.Connect()
	c = UsersUtilities()
	c.ConnectSpecificUser("SUPPORT","SUPPORT")
	c.LoadXmlFile('Scripts/TransSnmpSetWhiteList.xml')
	c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
	c.ModifyXml("SECURITY","TRAP_VERSION","snmpv1")
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
    c.ModifyXml("SECURITY","TRAP_VERSION","snmpv1")
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
    j.SendXmlFile("Scripts/GetMCU_State.xml")
   # j.PrintLastResponse()
    http_MCU_state = j.xmlResponse.getElementsByTagName("MCU_STATE")
    http_MCU_status = http_MCU_state[0].getElementsByTagName("ID")[0].firstChild.data
    http_MCU_status_desc = http_MCU_state[0].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    #print "http_MCU_status: "+http_MCU_status+" ("+http_MCU_status_desc +")"
    val = snmpGet.GetMCU_State(1)
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

def CheckTrapArrival(outputFile, alertNum,trap_desc="TRAPREQUEST"):
    outputFile2 = outputFile + "1"
#    command = "cat " + outputFile + " | grep \"Version: 2, type: TRAP\" | wc -l > " + outputFile2
    command = "cat " + outputFile + " | grep " + trap_desc + " | wc -l > " + outputFile2

    RunCommand(command)
    
    output = open(outputFile2)
    line = output.readline()
    output.close()
    
   # command = "rm " + outputFile
   # RunCommand(command)
    
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
print "1.---Enable SNMP -------------------------------------" 
EnableSNMP()
sleep (14)

KillSnmpListener()
print "2.---Config SNMPD trap destination to local host and set Community name as public1---"
ConfigSnmpdToLocalhost_Community_name2()
Sleep(10)

outputFile = "tmpTerminalCommandOutput1.shl"
print "4.---Run Snmp Trap Listener-------------------------"
RunSnmpListener(outputFile)
Sleep(10)
print "3.---create the authentication trap by connect with bad community name----------------" 
snmpGet = SNMPGetUtils()
version=1
val = snmpGet.GetDescriptionBadCommunityName(version)
print "Location    : " + str(val)
if ( str(val) <> snmpGet.GetErrorVal()):
#if ( str(val) <> "location_test"):
	print str(val)
	print"Test_failed "
	exit (1)

alertNum = 5

Sleep(1)

print "6.---Check Arrival of the Traps---------------------"
areTrapArrived = CheckTrapArrival(outputFile, alertNum,"AUTHENTICATIONFAILURE")

if(areTrapArrived):
    message = "All " + str(alertNum)  +" traps were arrived, Good "
    print message
else:
    message = "Not All " + str(alertNum)  +" traps were arrived, Test Faild"
    print message
    exit(1)

print "7.---good, we got the AUTHENTICATIONFAILURE trap (for v1)---------------------"
KillSnmpListener()
sleep(2)
outputFile = "tmpTerminalCommandOutput2.shl"
ConfigSnmpdToLocalhost_Community_name2("snmpv2")
#sleep(10)
print "8.---create the authentication trap by connect with bad community name (for v2)----------------" 
RunSnmpListener(outputFile)
sleep(2)
version=2
val = snmpGet.GetDescriptionBadCommunityName(version)
print "Location    : " + str(val)
if ( str(val) <> snmpGet.GetErrorVal()):
#if ( str(val) <> "location_test"):
	print str(val)
	print"Test_failed "
	exit (1)

alertNum = 5

Sleep(1)
print "9.---Check Arrival of the Traps---------------------"
areTrapArrived = CheckTrapArrival(outputFile,alertNum,"AUTHENTICATIONFAILURE")
print "areTrapArrived :" + str(areTrapArrived) +" --"
KillSnmpListener()
if(areTrapArrived):
    message = "All " + str(alertNum)  +" traps were arrived, Good "
    print message
else:
    message = "Not All " + str(alertNum)  +" traps were arrived, Test Faild"
    print message
    exit(1)

print "10.---Test Passed, we got the AUTHENTICATIONFAILURE trap (for v2)---------------------"
exit (0)