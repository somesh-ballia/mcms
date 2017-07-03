#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script which test RtmIsdn service configuration - deleting service
# The expected behavior:
#    - adding a service (otherwise the default service will be deleted)
#    - deleting the service
#    - ensure the deleted service is not there for next startup
# By: Haggai
#############################################################################


from McmsConnection import *


c = McmsConnection()
c.Connect()


#-------------------------------------------------------------------------------
# -------- Deleting a service
#-------------------------------------------------------------------------------

# -------- adding the service
print ""
print "Adding new ISDN service..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/NewRtmIsdnService.xml')
c.Send()
print "service 'NewServiceForScripts' accepted. Test continues..."


# -------- deleting the service
print ""
print "Deleting the service..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/DelRtmIsdnService.xml')
c.ModifyXml("DEL_ISDN_SERVICE","NAME","NewServiceForScripts")
c.Send()
print "Service 'NewServiceForScripts' deleted - test passed"


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

# -------- search for the range - first phone
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnServicesList.xml")
services_list = c.xmlResponse.getElementsByTagName("COMMON_SERVICE_PARAMS")
num_of_services = len(services_list)

serviceExists_after = 0
for index in range(num_of_services):
	fieldValue = services_list[index].getElementsByTagName("NAME")[0].firstChild.data
	if fieldValue == "NewServiceForScripts":
		serviceExists_after = serviceExists_after+1

c.Disconnect()

print ""
print "Searching for the existing service ('ISDN')..."
if serviceExists_after <> 0:
	print "Test Failed, service 'NewServiceForScripts' still exists (should be deleted)"
	print ""
	sys.exit(1)
else:
	print "Test Passed: service 'NewServiceForScripts' is still deleted"
	print ""
	sys.exit(0)
