#!/mcms/python/bin/python
 
from McmsConnection import *
 
c = McmsConnection()
c.Connect()

print "Monitor Faults..."
c.SendXmlFile("Scripts/GetFaults.xml")

fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")

num_of_faults = len(fault_elm_list)
 
noConnetion = 0
reConnetion = 0 
for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "CARD RECONNECT":
        noConnetion = noConnetion+1
    if noConnetion <> 0:
       print "Test failed, CARD RECONNECT found in Faults BEFORE the test started"
       sys.exit(1)
    
#################DISCONNECT SOCKET####################################################

print "Send socket disconnect to gideon sim...and wait 3 seconds to reconnect "
c.SendXmlFile("Scripts/SimSocketDisconnectMfa.xml")

sleep (3)
print "Monitor Faults 3 seconds after the socket disconnect..."
c.SendXmlFile("Scripts/GetFaults.xml")

fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)

for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "SOCKET_DISCONNECT":
        noConnetion = noConnetion+1
            
if noConnetion == 0:
    print "Test failed,  no SOCKET_DISCONNECT found in Faults after disconnect the socket for 3 seconds"
  #  sys.exit(1)

if noConnetion > 1:
    print "Test failed, More than 1 SOCKET_DISCONNECT found in Faults after disconnect the socket for 3 seconds"
    sys.exit(1) 
    
if noConnetion == 1:
    print "Test passed, 1 SOCKET_DISCONNECT found in Faults after disconnect the socket for 3 seconds"
   
#################RECONNECT SOCKET####################################################
  
print "Send socket reconnect to gideon sim...and wait 3 seconds"
c.SendXmlFile("Scripts/SimSocketConnectMfa.xml")
sleep (3)

print "Monitor Faults 3 seconds after the socket resconnect..."
c.SendXmlFile("Scripts/GetFaults.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "CARD RECONNECT":
        reConnetion = reConnetion+1
            
if reConnetion == 0:
    print "Test failed,  no CARD RECONNECT found in Faults after reconnect"
  #  sys.exit(1)

if reConnetion > 1:
    print "Test failed, More than 1 CARD RECONNECT found in Faults after resconnect"
    sys.exit(1) 
    
if reConnetion == 1:
    print "Test passed, 1 CARD RECONNECT found in Faults after reconnect"


c.Disconnect()
