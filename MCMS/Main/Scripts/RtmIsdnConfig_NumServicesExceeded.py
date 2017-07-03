#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script which test RtmIsdn service configuration - adding more services than allowed
# The expected behavior:
#    - adding more than two (in the future: one) service is rejected
# By: Haggai
#############################################################################


from McmsConnection import *


c = McmsConnection()
c.Connect()

#-------------------------------------------------------------------------------
# -------- Add second service
#-------------------------------------------------------------------------------
print ""
print "Adding new ISDN service - 2nd service in list..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/NewRtmIsdnService.xml')
c.ModifyXml("COMMON_SERVICE_PARAMS","NAME","ISDN2")
c.Send()
print "service accepted - test passed"


#-------------------------------------------------------------------------------
# -------- Add third service
#-------------------------------------------------------------------------------
print ""
print "Adding new ISDN service - 3rd service in list..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/NewRtmIsdnService.xml')
c.ModifyXml("COMMON_SERVICE_PARAMS","NAME","ISDN3")
c.Send("Maximum number of ISDN/PSTN Network Services reached")
print "service rejected - test passed"
print ""

c.Disconnect()
