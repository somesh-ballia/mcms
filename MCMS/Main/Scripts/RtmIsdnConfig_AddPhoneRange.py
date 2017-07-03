#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script which test RtmIsdn service configuration - adding phone range
# The expected behavior:
#    - adding a 'good' range: accepted
#    - adding overlapping ranges: rejected
#    - adding a range with different num of digits: rejected
#    - adding a range with firstNum larger then lastNum: rejected
#    - ensure the 'good' range is stored for next startup
# By: Haggai
#############################################################################


from McmsConnection import *


c = McmsConnection()
c.Connect()

#-------------------------------------------------------------------------------
# -------- Add a 'good' range
#-------------------------------------------------------------------------------
print ""
print "Adding new phone range (200-299)..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/NewRtmIsdnPhoneRange.xml')
c.ModifyXml("NET_PHONE","FIRST_PHONE_NUMBER","200")
c.ModifyXml("NET_PHONE","LAST_PHONE_NUMBER","299")
c.Send()
print "Phone range accepted - test passed"

#-------------------------------------------------------------------------------
# -------- Add overlapping ranges
#-------------------------------------------------------------------------------
print ""
print "Adding new phone range (230-234)..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/NewRtmIsdnPhoneRange.xml')
c.ModifyXml("NET_PHONE","FIRST_PHONE_NUMBER","230")
c.ModifyXml("NET_PHONE","LAST_PHONE_NUMBER","234")
c.Send("Phone number already exists")
print "Phone range rejected (overlaps) - test passed"

print ""
print "Adding new phone range (180-213)..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/NewRtmIsdnPhoneRange.xml')
c.ModifyXml("NET_PHONE","FIRST_PHONE_NUMBER","180")
c.ModifyXml("NET_PHONE","LAST_PHONE_NUMBER","213")
c.Send("Phone number already exists")
print "Phone range rejected (overlaps) - test passed"

print ""
print "Adding new phone range (236-347)..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/NewRtmIsdnPhoneRange.xml')
c.ModifyXml("NET_PHONE","FIRST_PHONE_NUMBER","236")
c.ModifyXml("NET_PHONE","LAST_PHONE_NUMBER","347")
c.Send("Phone number already exists")
print "Phone range rejected (overlaps) - test passed"

print ""
print "Adding new phone range (186-315)..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/NewRtmIsdnPhoneRange.xml')
c.ModifyXml("NET_PHONE","FIRST_PHONE_NUMBER","186")
c.ModifyXml("NET_PHONE","LAST_PHONE_NUMBER","315")
c.Send("Phone number already exists")
print "Phone range rejected (overlaps) - test passed"


#-------------------------------------------------------------------------------
# -------- Add a range with different num of digits
#-------------------------------------------------------------------------------
print ""
print "Adding phone range with different num of digits (8-23)..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/NewRtmIsdnPhoneRange.xml')
c.ModifyXml("NET_PHONE","FIRST_PHONE_NUMBER","8")
c.ModifyXml("NET_PHONE","LAST_PHONE_NUMBER","23")
c.Send("Invalid Phone Number Range")
print "Phone range rejected (illegal) - test passed"


#-------------------------------------------------------------------------------
# -------- Add a range with firstNum larger then lastNum
#-------------------------------------------------------------------------------
print ""
print "Adding phone range with different num of digits (764-732)..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/NewRtmIsdnPhoneRange.xml')
c.ModifyXml("NET_PHONE","FIRST_PHONE_NUMBER","764")
c.ModifyXml("NET_PHONE","LAST_PHONE_NUMBER","732")
c.Send("Invalid Phone Number Range")
print "Phone range rejected (illegal) - test passed"


#-------------------------------------------------------------------------------
# -------- Restar - to check if the new range still exists
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


# -------- search for the new range - first phone
c.SendXmlFile("Scripts/RtmIsdnConfig/GetRtmIsdnServicesList.xml")
phoneRanges_list = c.xmlResponse.getElementsByTagName("NET_PHONE")
num_of_phoneRanges = len(phoneRanges_list)

newFirstPhoneExists = 0
for index in range(num_of_phoneRanges):
	fieldValue = phoneRanges_list[index].getElementsByTagName("FIRST_PHONE_NUMBER")[0].firstChild.data
	if fieldValue == "200":
		newFirstPhoneExists = newFirstPhoneExists+1

print ""
print "Searching for the new phone range (200-299)..."
if newFirstPhoneExists <> 1:
	print "Test Failed, first phone from new range (200) does not exist"
	print ""
	sys.exit(1)
else:
	print "1st phase passed: first phone from new range (200) exists"


# -------- search for the new range - last phone
newLastPhoneExists = 0
for index in range(num_of_phoneRanges):
	fieldValue = phoneRanges_list[index].getElementsByTagName("LAST_PHONE_NUMBER")[0].firstChild.data
	if fieldValue == "299":
		newLastPhoneExists = newLastPhoneExists+1

c.Disconnect()

if newLastPhoneExists <> 1:
	print "Test Failed, last phone from new range (299) does not exist"
	print ""
	sys.exit(1)
else:
	print "2nd phase passed: last phone from new range (299) exists"
	print "Test Passed"
	print ""
	sys.exit(0)
