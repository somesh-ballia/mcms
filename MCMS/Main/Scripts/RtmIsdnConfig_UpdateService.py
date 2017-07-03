#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script which test RtmIsdn service configuration - updating service
# The expected behavior:
#    - updated field is accepted, and is stored for next startup
# By: Haggai
#############################################################################


from McmsConnection import *


c = McmsConnection()
c.Connect()

#-------------------------------------------------------------------------------
# -------- Update 'net spec' field
#-------------------------------------------------------------------------------

# -------- update the service
print ""
print "Update service - change Network_Specific field to 'att_1800'..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/UpdateRtmIsdnService.xml')
c.ModifyXml("ISDN_SERVICE","NET_SPECIFIC","att_1800")
c.Send()
print "service updated"

print "[ sleep 2 sec before checking Alerts (for valgrind checking) ]"
sleep(2)

# -------- check Alerts
print ""
print "Monitor System Alerts list..."      

c.SendXmlFile("Scripts/GetActiveAlarms.xml")
alerts_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_alerts = len(alerts_list)

specAlertFound = 0
for index in range(num_of_alerts):
	alert_desc = alerts_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
	if alert_desc == "ISDN_SERVICES_CONFIGURATION_CHANGED":
		specAlertFound = specAlertFound+1
    
c.Disconnect()
if specAlertFound <> 1:
	print "Test Failed, Alert 'ISDN_SERVICES_CONFIGURATION_CHANGED' does not exist"
	sys.exit(1)
else:
	print "Test Passed, Alert 'ISDN_SERVICES_CONFIGURATION_CHANGED' exists"


#-------------------------------------------------------------------------------
# -------- Restart - to check if it's still updates
#-------------------------------------------------------------------------------

# -------- restart
print ""
print "Restart (for starting over)..."
print ""

import os
import sys

os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")

c = McmsConnection()
c.Connect()

# -------- for next startups
os.environ["CLEAN_CFG"]="YES"


# -------- search for the updated field
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnServicesList.xml")
services_list = c.xmlResponse.getElementsByTagName("ISDN_SERVICE")
num_of_services = len(services_list)

fieldUpdated = 0
for index in range(num_of_services):
	fieldValue = services_list[index].getElementsByTagName("NET_SPECIFIC")[0].firstChild.data
	if fieldValue == "att_1800":
		fieldUpdated = fieldUpdated+1

c.Disconnect()

print ""
if fieldUpdated <> 1:
	print "Test Failed, the field is not updated"
	print ""
	sys.exit(1)
else:
	print "Test Passed, the field is updated"
	print ""
	sys.exit(0)
