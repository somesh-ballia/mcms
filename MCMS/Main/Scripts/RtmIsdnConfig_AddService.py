#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script which test RtmIsdn service configuration - adding service
# The expected behavior:
#    - adding a service with the same name is rejected
# By: Haggai
#############################################################################


from McmsConnection import *


c = McmsConnection()
c.Connect()


#-------------------------------------------------------------------------------
# -------- Add service
#-------------------------------------------------------------------------------
print ""
print "Adding new ISDN service..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/NewRtmIsdnService.xml')
c.Send()
print "service accepted - test passed"
c.Disconnect()



# -------- restart (for starting over)
print ""
print "Restart (for starting over)..."
print ""

import os
os.system("Scripts/Startup.sh")

c = McmsConnection()
c.Connect()

#-------------------------------------------------------------------------------
# -------- Add service with same name as already exists
#-------------------------------------------------------------------------------
print ""
print "Adding new ISDN service with the same name as already existed ('ISDN')..."
c.LoadXmlFile('Scripts/RtmIsdnConfig/NewRtmIsdnService.xml')
c.ModifyXml("COMMON_SERVICE_PARAMS","NAME","ISDN")
c.Send("Service provider name already exists")
print "service rejected - test passed"
print ""

c.Disconnect()
