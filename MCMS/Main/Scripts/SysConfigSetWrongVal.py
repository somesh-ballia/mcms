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
# c.SendXmlFile("Scripts/SysConfig/SysConfigGet.xml")
# pair_list = c.xmlResponse.getElementsByTagName("CFG_PAIR")
# num_of_pairs = len(pair_list)

# print "num_of_pairs = " + str(num_of_pairs)
# for index in range(num_of_services):
#	fieldKey = services_list[index].getElementsByTagName("KEY")[0].firstChild.data
#	if fieldKey == "ENABLE_EXTERNAL_DB_ACCESS":
#		fieldData = services_list[index].getElementsByTagName("DATA")[0].firstChild.data

data=1
c.LoadXmlFile('Scripts/SysConfig/SysConfigSetWrongBool.xml')
 # should get YES-NO
c.Send("Invalid ENUM value")

c.Disconnect()

