#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script 
# By: Hillel
#############################################################################


from McmsConnection import *


c = McmsConnection()
c.Connect()

c.LoadXmlFile('Scripts/SysConfig/SysConfigSetWrongEnum.xml')
 # should get YES-NO
c.Send("Invalid ENUM value")

c.Disconnect()

