#!/mcms/python/bin/python
 
from McmsConnection import *
 
c = McmsConnection()
c.Connect()
print "Monitor active alarms..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")

fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
 
startup = 0  
 
for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "SYSTEM STARTUP":
        startup = 1
 
if startup == 0:
    print "Test failed, no SYSTEM STARTUP found"
    sys.exit(1)
        
print "waiting 4 seconds..."
sleep(3)
print "Killing EndpointsSim..."
os.system("killall EndpointsSim")
print "waiting 3 seconds..."
sleep(3)
print "Monitor active alarms..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
num_of_faults_after = len(c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT"))
 
if num_of_faults >= num_of_faults_after:
    print "Test failed, no new active alarm created"
    c.PrintLastResponse()
    c.Disconnect()
    sys.exit(1)
 
c.Disconnect()


#----------------------------------------------------------------------------------------------- 

