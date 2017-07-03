#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6

 
from McmsConnection import *
 
c = McmsConnection()
c.Connect()

print "Monitor Faults..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")

fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")

num_of_faults = len(fault_elm_list)
 
noConnetion = 0
 
for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "NO_CONNECTION_WITH_CARD":
        noConnetion = noConnetion+1
 
if noConnetion <> 0:
    print "Test failed, NO_CONNECTION_WITH_CARD found in Active Alarms BEFORE the test started"
    sys.exit(1)
#################DISCONNECT SOCKET####################################################
print "Send socket disconnect to gideon sim...and wait 2 seconds to reconnect "
c.SendXmlFile("Scripts/SimSocketDisconnectMfa.xml")
sleep (21)
print "Monitor Faults 21 seconds after the socket disconnect..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")

fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)

for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "NO_CONNECTION_WITH_CARD":
        noConnetion = noConnetion+1
            
if noConnetion == 0:
    print "Test failed,  no NO_CONNECTION_WITH_CARD found in Active Alarms after disconnect the socket for 11 seconds"
  #  sys.exit(1)

if noConnetion > 1:
    print "Test failed, More than 1 NO_CONNECTION_WITH_CARD found in FActive Alarms after disconnect the socket for 11 seconds"
    sys.exit(1) 
    
if noConnetion == 1:
    print "Good Start, 1 NO_CONNECTION_WITH_CARD found in Active Alarms after disconnect the socket for 11 seconds"
   
 #################CONNECT SOCKET####################################################
  
print "Send socket reconnect to gideon sim...and wait 11 seconds"
c.SendXmlFile("Scripts/SimSocketConnectMfa.xml")
sleep (11)
noConnetion = 0

print "Monitor Faults 11 seconds after the socket resconnect..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "NO_CONNECTION_WITH_CARD":
        noConnetion = noConnetion+1
            
if noConnetion == 0:
    print "Test passed,  no NO_CONNECTION_WITH_CARD found in Active Alarms after disconnect the socket for 11 seconds"
  #  sys.exit(1)

if noConnetion > 1:
    print "Test failed, More than 1 NO_CONNECTION_WITH_CARD found in Active Alarms after disconnect the socket for 11 seconds"
    sys.exit(1) 
    
if noConnetion == 1:
    print "Test failed, 1 NO_CONNECTION_WITH_CARD found in Active Alarms after disconnect the socket for 11 seconds"

