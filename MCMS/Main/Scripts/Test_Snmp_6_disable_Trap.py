#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_11
from McmsConnection import *
from SNMPUtils import *

#snmpGet = SNMPGetUtils()
#val = snmpGet.GetDescription()
#print "Description : " + str(val)

c = McmsConnection()
c.Connect()
print "________1___SnmpGet_______________________"
c.SendXmlFile('Scripts/TransSnmpGet.xml')
#c.PrintLastResponse()
sleep(1)
print "________2___Set enabled trap _____________"
c.LoadXmlFile('Scripts/TransSnmpSet.xml')
c.ModifyXml("SNMP_DATA","LOCATION","location_test")
c.ModifyXml("SNMP_DATA","CONTACT_NAME","contact_name_test")
c.ModifyXml("SECURITY","ACCEPT_ALL_REQUESTS","true")
c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","public")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
c.Send()
#c.PrintLastResponse()
c.SendXmlFile('Scripts/TransSnmpGet.xml')
#c.PrintLastResponse()
sleep(1)

print "________3   Test enabled trap_____________"

snmpGet = SNMPGetUtils()
val = snmpGet.GetDescription()
print "Description : " + str(val)

val = snmpGet.GetLocation()
print "Location    : " + str(val)
if ( str(val) <> "location_test"):
   print"oy oy oy ... wrong location value "
   exit (1)
val = snmpGet.GetContact()
print "Contact     : " + str(val)
if ( str(val) <> "contact_name_test"):
   print"oy oy oy ... wrong contact_name value "
   exit (1)
"""   
val = snmpGet.GetName()
print "Name        : " + str(val)
if ( str(val) <> "system_name_test"):
   print"oy oy oy ... wrong system_name value "
   print val
   exit (1)
"""
val = snmpGet.GetDescription()
print "Description : " + str(val)
if ( str(val) <> "RMX.0.0.0.0"):
   print"oy oy oy ... wrong description value "
   print val
   exit (1)

val = snmpGet.GetObjectId()
print "ObjectId    : " + str(val)
#if ( str(val) <> ".1.3.6.1.4.1.13885.9.1.10.2"):
if ( str(val) <> ".1.3.6.1.4.1.13885.9.1.10.1"):
   print"oy oy oy ... wrong ObjectId value "
   print val
   exit (1)

print "________4___Disable trap__________________"

c.LoadXmlFile('Scripts/TransSnmpSet.xml')
c.ModifyXml("SNMP_DATA","LOCATION","location_test")
c.ModifyXml("SNMP_DATA","CONTACT_NAME","contact_name_test")
#c.ModifyXml("SNMP_DATA","SNMP_ENABLED","false")
c.ModifyXml("SECURITY","ACCEPT_ALL_REQUESTS","false")
c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","public")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
c.Send()
#c.PrintLastResponse()
c.SendXmlFile('Scripts/TransSnmpGet.xml')
#c.PrintLastResponse()

print "________5___Test disabled trap____________"

snmpGet = SNMPGetUtils()
val = snmpGet.GetDescription()
#if ( str(val) <> "Connection refused"):
if ( str(val) <> snmpGet.GetErrorVal()):
   print"Test_failed "
   exit (1)
print "Description : " + str(val)

print "_Good !!! the  disable trap succeed_______"

print "________6___Enable the traps again _______"
c.LoadXmlFile('Scripts/TransSnmpSet.xml')
c.ModifyXml("SNMP_DATA","LOCATION","new_location_test")
c.ModifyXml("SNMP_DATA","CONTACT_NAME","contact_name_test")
#c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
c.ModifyXml("SECURITY","ACCEPT_ALL_REQUESTS","true")
c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","public")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
c.Send()
#c.PrintLastResponse()
c.SendXmlFile('Scripts/TransSnmpGet.xml')
#c.PrintLastResponse()
sleep (5)
print "________7___Test the enable trap__________"
val = snmpGet.GetLocation()
print "Location    : " + str(val)
if ( str(val) <> "new_location_test"):
   print"Test_failed -Enabeld Snmpd failed"
   exit (1)
print "_Good !!! the  Enable snmp test succeed!!!"


