#!/mcms/python/bin/python

from SNMPUtils import *




snmpGet = SNMPGetUtils()

print ""

ver = 1
val = snmpGet.GetDescription(ver)
print "Description version " + str(ver) + " : " + str(val)

ver = 2
val = snmpGet.GetObjectId(ver)
print "ObjectId    version " + str(ver) + " : " + str(val)

ver = 1
val = snmpGet.GetUpTime(ver)
print "Up Time     version " + str(ver) + " : " + str(val)

ver = 2
val = snmpGet.GetContact(ver)
print "Contact     version " + str(ver) + " : " + str(val)

ver = 1
val = snmpGet.GetName(ver)
print "Name        version " + str(ver) + " : " + str(val)

ver = 2
val = snmpGet.GetLocation(ver)
print "Location    version " + str(ver) + " : " + str(val)

ver = 1
val = snmpGet.GetServices(ver)
print "Services    version " + str(ver) + " : " + str(val)

ver = 2
val = snmpGet.GetGatekeeperAddress(ver)
print "GK IP       version " + str(ver) + " : " + str(val)

print ""


