#!/usr/bin/python

from McmsConnection import *


c = McmsConnection()
c.Connect()
print "________1___Snmp Set_______________________"
c.SendXmlFile('Scripts/TransSnmpSet.xml')
c.PrintLastResponse()
sleep(1)

print "________2___Snmp Get_______________________"
c.SendXmlFile('Scripts/TransSnmpGet.xml')
c.PrintLastResponse()
sleep(1)

print "________3___Snmp Set Empty Community Name__"
c.SendXmlFile('Scripts/TransSnmpEmtptyCommunityNameSet.xml',"STATUS_INCONSISTENT_PARAMETERS")
c.PrintLastResponse()
sleep(1)

print "________4___SnmpGet________________________"
c.SendXmlFile('Scripts/TransSnmpGet.xml')
c.PrintLastResponse()
c.Disconnect()

print "________5___Test passed____________________"
