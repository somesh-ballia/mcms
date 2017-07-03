#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script 
# By: Hillel
#############################################################################


from McmsConnection import *


c = McmsConnection()
c.Connect()

c.LoadXmlFile('Scripts/SysConfig/SysConfigSetTooLongString.xml')
 # should get YES-NO
c.Send()

c.Disconnect()

