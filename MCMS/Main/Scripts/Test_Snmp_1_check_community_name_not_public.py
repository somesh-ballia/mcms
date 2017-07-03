#!/usr/bin/python

from McmsConnection import *


c = McmsConnection()
c.Connect()
print "________1___SnmpGet_______________________"
c.SendXmlFile('Scripts/TransSnmpGet.xml')
c.PrintLastResponse()
sleep(1)
print "________2___change community name ________"
c.LoadXmlFile('Scripts/TransSnmpSet.xml')
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","Dora")
c.Send("STATUS_INCONSISTENT_PARAMETERS")
#c.PrintLastResponse()
print "________3___Test passed __________________"



