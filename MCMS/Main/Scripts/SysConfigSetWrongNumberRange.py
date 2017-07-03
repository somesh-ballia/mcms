#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script  
# By: Hillel
#############################################################################


from McmsConnection import *


c = McmsConnection()
c.Connect()

c.LoadXmlFile('Scripts/SysConfig/SysConfigSetWrongRangeNumber.xml')
 # should get YES-NO
c.Send("value is out of range")

c.Disconnect()

