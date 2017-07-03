#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6

 
from McmsConnection import *
 
c = McmsConnection()
c.Connect()

print "Monitor Faults..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)

for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    

if num_of_faults <> 1:
	print""
	print "Test failed, Active Alarms does not contain only the expected Alert"
	sys.exit(1)
		
if num_of_faults == 1:
	if "DEFAULT_USER_EXISTS" <> fault_desc:
		print""
		print "Test failed, Active Alarms contains other Alert then the expected DEFAULT_USER_EXISTS"
		sys.exit(1)
	else:
		print""
		print "Test Passed, Active Alarms contains only the expected Alert - DEFAULT_USER_EXISTS"
		sys.exit(0)

c.Disconnect()
   
