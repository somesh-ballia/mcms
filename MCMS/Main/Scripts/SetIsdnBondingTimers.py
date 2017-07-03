#!/mcms/python/bin/python
 
from McmsConnection import *
 
c = McmsConnection()
c.Connect()
print "Sending ISDN Bonding timers (alignment) to GideonSim..."

c.SendXmlFile("Scripts/SimIsdnBondingTimer.xml") 

#print "Waiting 2 seconds to delete Operator..."
#sleep (2)
#print "Deleting new Operator..."
#c.SendXmlFile("Scripts/RemoveNewOperator.xml")
c.Disconnect()
