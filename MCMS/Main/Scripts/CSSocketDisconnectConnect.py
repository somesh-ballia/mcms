#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_7
 
from McmsConnection import *
 
c = McmsConnection()
c.Connect()

print "Monitor active alarms..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
#c.SendXmlFile("Scripts/GetFaults.xml")

fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")

num_of_faults = len(fault_elm_list)
 
noConnetion = 0
 
for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "NO CONNECTION WITH CS":
        noConnetion = noConnetion+1
 
if noConnetion <> 0:
    print "Test failed, NO CONNECTION WITH CS found in active alararms BEFORE the test started"
    sys.exit(1)
#################DISCONNECT SOCKET####################################################
print "Send socket disconnect to End point sim...and wait 160 seconds to reconnect "
c.SendXmlFile("Scripts/CSSimSocketDisconnect.xml")
sleep (170)

print "Monitor active alarms 170 seconds after the socket disconnect..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")

fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)

for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "NO CONNECTION WITH CS":
        noConnetion = noConnetion+1
            
if noConnetion == 0:
    print "Test failed,  no NO CONNECTION WITH CS found in active alararms after disconnect the socket for 160 seconds"
    sys.exit(1)

if noConnetion > 1:
    print "Test failed, More than 1 NO CONNECTION WITH CS found in active alararms after disconnect the socket for 160 seconds"
    sys.exit(1) 
    
if noConnetion == 1:
    print "Test passed, 1 NO CONNECTION WITH CS found in active alararms after disconnect the socket for 160 seconds"
   
 #################CONNECT SOCKET####################################################
  
print "Send socket Connect to End point sim...and wait 30 seconds"
c.SendXmlFile("Scripts/CSSimSocketConnect.xml")
sleep (30)

print "Monitor active alarms 170 seconds after the socket disconnect..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "NO CONNECTION WITH CS":
        print "Test failed, NO_CONNECTION_WITH_CS found in active alararms 30 seconds after reconnect the socket."
        sys.exit(1)

print "Test passed The NO CONNECTION WITH CS removed from the Active alarm"
c.Disconnect()
