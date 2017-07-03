#!/usr/bin/python

from McmsConnection import *


c = McmsConnection()
c.Connect()
print "________1___SnmpGet_______________________"
c.SendXmlFile('Scripts/TransSnmpGet.xml')
c.PrintLastResponse()
sleep(1)

print "________2___SnmpSet_______________________"
c.SendXmlFile('Scripts/TransSnmpSet.xml')
c.PrintLastResponse()
sleep(1)
print "________3__SnmpGet________________________"
c.SendXmlFile('Scripts/TransSnmpGet.xml')
c.PrintLastResponse()
c.Disconnect()
sys.exit(1)

