#!/mcms/python/bin/python
 
from McmsConnection import *
 
c = McmsConnection()
c.Connect()
print "Monitor active alarms..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")

fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
 
noConnetion = 0
 
for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    fault_time = fault_desc.getElementsByTagName("TIME_STAMP")[0].firstChild.data
   # print
    # print fault_desc
     #print fault_time
    print "SHLOMIT PRINT**************************************************************"
    if fault_desc == "NO_CONNECTION_WITH_CARD":
        noConnetion = noConnetion+1
 
if noConnetion == 0:
    print "Test failed, no SYSTEM STARTUP found in active alararms"
    sys.exit(1)
    
if noConnetion > 1:
    print "Test failed, More than 1 SYSTEM STARTUP found in active alararms"
    sys.exit(1) 
    
if noConnetion == 1:
    print "Test passed, 1 SYSTEM STARTUP found in active alararms"
    sys.exit(1)
               

c.Disconnect()
