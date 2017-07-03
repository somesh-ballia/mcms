#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script which test RtmIsdn service configuration - changing the default service
# The expected behavior:
#    - the default service is changed, and is stored for next startup
# By: Haggai
#############################################################################


from McmsConnection import *


c = McmsConnection()
c.Connect()

#-------------------------------------------------------------------------------
# -------- Change the default service
#-------------------------------------------------------------------------------

# -------- search for current default service
print ""
print "Adding a service named 'NewServiceForScripts'..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/NewRtmIsdnService.xml')
c.Send()


# -------- search for current default service
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnServicesList.xml")
services_list = c.xmlResponse.getElementsByTagName("ISDN_SERVICE_LIST")
num_of_services = len(services_list)

currentDefault = 0
for index in range(num_of_services):
	fieldValue = services_list[index].getElementsByTagName("DEFAULT_NAME")[0].firstChild.data
	if fieldValue == "ISDN":
		currentDefault = currentDefault+1

print ""
print "Checking current default service..."
if currentDefault <> 1:
	print "Test Failed, the default service is not 'ISDN' as expected"
	print ""
	sys.exit(1)
else:
	print "Current default service is 'ISDN', as expected. Test continues..."


# -------- change default service
print ""
print "Setting 'NewServiceForScripts' as default service..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/SetDefaultRtmIsdnService.xml')
c.ModifyXml("SET_DEFAULT_ISDN_SERVICE","NAME","NewServiceForScripts")
c.Send()



# -------- search for current default service
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnServicesList.xml")
services_list = c.xmlResponse.getElementsByTagName("ISDN_SERVICE_LIST")

currentDefault = 0
for index in range(num_of_services):
	fieldValue = services_list[index].getElementsByTagName("DEFAULT_NAME")[0].firstChild.data
	if fieldValue == "NewServiceForScripts":
		currentDefault = currentDefault+1

print ""
print "Checking current default service..."
if currentDefault <> 1:
	print "Test Failed, the default service is not 'NewServiceForScripts' as expected"
	print ""
	sys.exit(1)
else:
	print "Test Passed, current default service is 'NewServiceForScripts', as expected"

c.Disconnect()


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


# -------- search for current default service
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnServicesList.xml")
services_list = c.xmlResponse.getElementsByTagName("ISDN_SERVICE_LIST")

currentDefault = 0
for index in range(num_of_services):
	fieldValue = services_list[index].getElementsByTagName("DEFAULT_NAME")[0].firstChild.data
	if fieldValue == "NewServiceForScripts":
		currentDefault = currentDefault+1

c.Disconnect()

print ""
print "Checking current default service (after reset)..."
if currentDefault <> 1:
	print "Test Failed, the default service is not 'NewServiceForScripts' as expected"
	print ""
	sys.exit(1)
else:
	print "Test Passed, current default service is 'NewServiceForScripts', as expected"
	print ""
	sys.exit(0)
