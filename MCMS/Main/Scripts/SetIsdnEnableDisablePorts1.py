#!/mcms/python/bin/python
 
from McmsConnection import *
 
c = McmsConnection()
c.Connect()
print "Sending ISDN Enable Disable Ports (1) to GideonSim..."

c.SendXmlFile("Scripts/SimIsdnEnableDisablePorts1.xml") 

c.Disconnect()
