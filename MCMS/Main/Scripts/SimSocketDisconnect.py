#!/mcms/python/bin/python
 
from McmsConnection import *
 
c = McmsConnection()
c.Connect()
print "Send socket disconnect to gideon sim..."

c.SendXmlFile("Scripts/SimSocketDisconnectMfa.xml")

#print "Waiting 2 seconds to delete Operator..."
#sleep (2)
#print "Deleting new Operator..."
#c.SendXmlFile("Scripts/RemoveNewOperator.xml")
c.Disconnect()
