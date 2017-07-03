#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script 
# By: Hillel
#############################################################################


from McmsConnection import *


c = McmsConnection()
c.Connect()

# change valid value, No => Yes, or alike
c.SendXmlFile("Scripts/SysConfig/SysConfigGet.xml")
services_list = c.xmlResponse.getElementsByTagName("CFG_SECTION")
num_of_services = len(services_list)

data = "NO"
for index in range(num_of_services):
	fieldValue = services_list[index].getElementsByTagName("ENABLE_EXTERNAL_DB_ACCESS")[0].firstChild.data
	if fieldValue == "NO":
		data = "YES"

c.LoadXmlFile('Scripts/SysConfig/SysConfigSet.xml')
c.ModifyXml("CFG_SECTION", "ENABLE_EXTERNAL_DB_ACCESS", data)
c.Send()


# -------- check Alerts
print "Monitor System Alerts list..."      

c.SendXmlFile("Scripts/GetActiveAlarms.xml")
alerts_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_alerts = len(alerts_list)

specAlertFound = 0
for index in range(num_of_alerts):
	alert_desc = alerts_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
	if alert_desc == "CONFIGURATION_CHANGED":
		specAlertFound = specAlertFound+1
    
c.Disconnect()
if specAlertFound <> 1:
	sys.exit("Test Failed, Alert CONFIGURATION_CHANGED does not exist")
else:
	print "Test Passed, Alert 'ISDN_SERVICES_CONFIGURATION_CHANGED' exists"

#-------------------------------------------------------------------------------
# -------- Restart - to check if it still updates
#-------------------------------------------------------------------------------
print "Restart (for starting over)..."

import os
import sys

os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")

c = McmsConnection()
c.Connect()

# -------- for next startups
os.environ["CLEAN_CFG"]="YES"


# -------- search for the updated field
c.SendXmlFile("Scripts/SysConfig/SysConfigGet.xml")
services_list = c.xmlResponse.getElementsByTagName("CFG_SECTION")
num_of_services = len(services_list)

for index in range(num_of_services):
	fieldValue = services_list[index].getElementsByTagName("ENABLE_EXTERNAL_DB_ACCESS")[0].firstChild.data
	if fieldValue != data:
		sys.exit("Test failed: ENABLE_EXTERNAL_DB_ACCESS wasn't changed to "+data)
	else:
		print "Test passed: field was changed"		
		
c.Disconnect()

