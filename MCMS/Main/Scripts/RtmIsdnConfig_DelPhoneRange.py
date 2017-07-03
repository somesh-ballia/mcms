#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script which test RtmIsdn service configuration - deleting phone range
# The expected behavior:
#    - deleting a phone range
#    - ensure the deleted range is not there for next startup
# By: Haggai
#############################################################################


from McmsConnection import *


c = McmsConnection()
c.Connect()

#-------------------------------------------------------------------------------
# -------- Deleting a range
#-------------------------------------------------------------------------------

# -------- search for the range - first phone
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnServicesList.xml")
phoneRanges_list = c.xmlResponse.getElementsByTagName("NET_PHONE")
num_of_phoneRanges = len(phoneRanges_list)

firstPhoneExists_before = 0
for index in range(num_of_phoneRanges):
	fieldValue = phoneRanges_list[index].getElementsByTagName("FIRST_PHONE_NUMBER")[0].firstChild.data
	if fieldValue == "3000":
		firstPhoneExists_before = firstPhoneExists_before+1

print ""
print "Searching for the existing phone range (3000-3999)..."
if firstPhoneExists_before <> 1:
	print "Test Failed, first number from range (3000) does not exist"
	print ""
	sys.exit(1)

# -------- search for the range - last phone
lastPhoneExists_before = 0
for index in range(num_of_phoneRanges):
	fieldValue = phoneRanges_list[index].getElementsByTagName("LAST_PHONE_NUMBER")[0].firstChild.data
	if fieldValue == "3999":
		lastPhoneExists_before = lastPhoneExists_before+1

if lastPhoneExists_before <> 1:
	print "Test Failed, last number from range (3999) does not exist"
	print ""
	sys.exit(1)
else:
	print "Phone range (3000-3999) exists. Test continues..."



# -------- deleting the raneg
print ""
print "Deleting the phone range..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/DelRtmIsdnPhoneRange.xml')
c.ModifyXml("NET_PHONE","FIRST_PHONE_NUMBER","3000")
c.ModifyXml("NET_PHONE","LAST_PHONE_NUMBER","3999")
c.Send()
print "Phone range deleted - test passed"


#-------------------------------------------------------------------------------
# -------- Restart - to check if the new range still exists
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

# -------- search for the range - first phone
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnServicesList.xml")
phoneRanges_list = c.xmlResponse.getElementsByTagName("NET_PHONE")
num_of_phoneRanges = len(phoneRanges_list)

firstPhoneExists_after = 0
for index in range(num_of_phoneRanges):
	fieldValue = phoneRanges_list[index].getElementsByTagName("FIRST_PHONE_NUMBER")[0].firstChild.data
	if fieldValue == "3000":
		firstPhoneExists_after = firstPhoneExists_after+1

print ""
print "Searching for the existing phone range (3000-3999)..."
if firstPhoneExists_after <> 0:
	print "Test Failed, first number from range (3000) still exists (should be deleted)"
	print ""
	sys.exit(1)

# -------- search for the range - last phone
lastPhoneExists_after = 0
for index in range(num_of_phoneRanges):
	fieldValue = phoneRanges_list[index].getElementsByTagName("LAST_PHONE_NUMBER")[0].firstChild.data
	if fieldValue == "444":
		lastPhoneExists_after = lastPhoneExists_after+1

c.Disconnect()

if lastPhoneExists_after <> 0:
	print "Test Failed, last number from range (3999) still exists (should be deleted)"
	print ""
	sys.exit(1)
else:
	print "Test Passed: last number from range (3000-3999) is still deleted"
	print ""
	sys.exit(0)
