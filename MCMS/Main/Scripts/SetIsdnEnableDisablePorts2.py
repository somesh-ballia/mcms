#!/mcms/python/bin/python
 
from McmsConnection import *
 
c = McmsConnection()
c.Connect()
print "Sending ISDN Enable Disable Ports (2) to GideonSim..."

c.SendXmlFile("Scripts/SimIsdnEnableDisablePorts2.xml") 

c.Disconnect()
