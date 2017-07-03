#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_11
#-- EXPECTED_ASSERT(1)=conId does not exist

from McmsConnection import *
 
c = McmsConnection()
c.Connect()

print "Monitor Faults..."
#c.SendXmlFile("Scripts/GetActiveAlarms.xml")
c.SendXmlFile("Scripts/GetFaults.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")

num_of_faults = len(fault_elm_list)
 
noConnetion = 0
 
for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "SOCKET_DISCONNECT":
        noConnetion = noConnetion+1
 
if noConnetion <> 0:
    print "Test failed, SOCKET_DISCONNECT found in Active Alarms BEFORE the test started"
    sys.exit(1)

#################DISCONNECT SOCKET####################################################
print "Send socket disconnect to cs sim...and wait 8 seconds to reconnect "
c.SendXmlFile("Scripts/CSSimSocketDisconnect.xml")
sleep (8)
print "Monitor Faults 3 seconds after the socket disconnect..."


c.SendXmlFile("Scripts/GetFaults.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "SOCKET_DISCONNECT":
        noConnetion = noConnetion+1

if noConnetion == 1:
    print "Good, 1 SOCKET_DISCONNECT found in Active Alarms after disconnect the socket for 3 seconds"
    sys.exit(0)
            
if noConnetion == 0:
    print "Test failed,  no SOCKET_DISCONNECT found in Active Alarms after disconnect the socket for 3 seconds"
    sys.exit(1)

if noConnetion > 1:
    print "Test failed, More than 1 SOCKET_DISCONNECT found in FActive Alarms after disconnect the socket for 3 seconds"
    sys.exit(1) 
    
   

  
