#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_11
from McmsConnection import *
#////
from SNMPUtils import *




#snmpGet = SNMPGetUtils()

print ""

#val = snmpGet.GetDescription()
#print "Description : " + str(val)

#///

c = McmsConnection()
c.Connect()
print "________1___SnmpGet_______________________"
c.SendXmlFile('Scripts/TransSnmpGet.xml')
c.PrintLastResponse()
sleep(1)
print "________2___change community name ________"
c.LoadXmlFile('Scripts/TransSnmpSet.xml')
c.ModifyXml("SNMP_DATA","LOCATION","location_test")
c.ModifyXml("SNMP_DATA","CONTACT_NAME","contact_name_test")
c.ModifyXml("SNMP_DATA","SYSTEM_NAME","system_name_test")
c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
c.ModifyXml("SECURITY","TRAP_VERSION","snmpv1")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","public")
#c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","majic")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
#c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
#c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
c.Send()
c.PrintLastResponse()
c.SendXmlFile('Scripts/TransSnmpGet.xml')
c.PrintLastResponse()

print "Sleep 5 - it takes time to update data"
sleep(5)

print ""
snmpGet = SNMPGetUtils()
version=1

val = snmpGet.GetDescription(version)
print "Description : " + str(val)

val = snmpGet.GetLocation(version)
print "Location    : " + str(val)
if ( str(val) <> "location_test"):
   print"oy oy oy ... wrong location value "
   exit (1)
val = snmpGet.GetContact(version)
print "Contact     : " + str(val)
if ( str(val) <> "contact_name_test"):
   print"oy oy oy ... wrong contact_name value "
   exit (1)
val = snmpGet.GetName(version)
print "Name        : " + str(val)
if ( str(val) <> "system_name_test"):
   print"oy oy oy ... wrong system_name value "
   exit (1)
val = snmpGet.GetDescription(version)
print "Description : " + str(val)
if ( str(val) <> "RMX.0.0.0.0"):
   print"oy oy oy ... wrong description value "
   exit (1)

val = snmpGet.GetObjectId(version)
print "ObjectId    : " + str(val)
#if ( str(val) <> ".1.3.6.1.4.1.13885.9.1.10.2"):
if ( str(val) <> ".1.3.6.1.4.1.13885.9.1.10.1"):
   print"oy oy oy ... wrong ObjectId value "
   exit (1)

version=2

val = snmpGet.GetDescription(version)
print "Description : " + str(val)

val = snmpGet.GetLocation(version)
print "Location    : " + str(val)
if ( str(val) <> "location_test"):
   print"oy oy oy ... wrong location value "
   exit (1)
val = snmpGet.GetContact(version)
print "Contact     : " + str(val)
if ( str(val) <> "contact_name_test"):
   print"oy oy oy ... wrong contact_name value "
   exit (1)
val = snmpGet.GetName(version)
print "Name        : " + str(val)
if ( str(val) <> "system_name_test"):
   print"oy oy oy ... wrong system_name value "
   exit (1)

val = snmpGet.GetDescription(version)
print "Description : " + str(val)
if ( str(val) <> "RMX.0.0.0.0"):
   print"oy oy oy ... wrong description value "
   exit (1)

val = snmpGet.GetObjectId(version)
print "ObjectId    : " + str(val)
#if ( str(val) <> ".1.3.6.1.4.1.13885.9.1.10.2"):
if ( str(val) <> ".1.3.6.1.4.1.13885.9.1.10.1"):
   print"oy oy oy ... wrong ObjectId value "
   exit (1)

#val = snmpGet.GetNumber_OfInterfaces()
#print "IF number   : " + str(val)
#if ( str(val) <> "4"):
#   print"oy oy oy ... wrong IF number "
#   exit (1)
#   print "val: "++str(val)

#c.PrintLastResponse()
print "________3___Test passed __________________"
exit (0)


