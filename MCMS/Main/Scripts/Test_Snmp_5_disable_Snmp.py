#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_11
from McmsConnection import *
from SNMPUtils import *

#snmpGet = SNMPGetUtils()
#val = snmpGet.GetDescription()
#print "Description : " + str(val)

c = McmsConnection()
c.Connect()
command = "netstat -u -l | grep 8090"
if os.system (command) == 0:
	print "snmp port is open right after startup- test failed"
	sys.exit(1)
print "________1___SnmpGet_______________________"
c.SendXmlFile('Scripts/TransSnmpGet.xml')
#c.PrintLastResponse()
sleep(1)
print "________2___Enable_snmp___________________"
c.LoadXmlFile('Scripts/TransSnmpSet.xml')
c.ModifyXml("SNMP_DATA","LOCATION","location_test")
c.ModifyXml("SNMP_DATA","CONTACT_NAME","contact_name_test")

c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","public")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
c.Send()
#c.PrintLastResponse()
c.SendXmlFile('Scripts/TransSnmpGet.xml')
#c.PrintLastResponse()

sleep(1)

print "________3___Test_enable_snmp______________"
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
if os.system (command) <> 0:
	print "snmp port not found after enabled snmp- test failed"
	sys.exit(1)

print "________4___ Disable snmp ________________"
c.LoadXmlFile('Scripts/TransSnmpSet.xml')
c.ModifyXml("SNMP_DATA","LOCATION","location_test")
c.ModifyXml("SNMP_DATA","CONTACT_NAME","contact_name_test")
c.ModifyXml("SNMP_DATA","SNMP_ENABLED","false")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","public")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
c.Send()
#c.PrintLastResponse()
c.SendXmlFile('Scripts/TransSnmpGet.xml')
#c.PrintLastResponse()


snmpGet = SNMPGetUtils()
val = snmpGet.GetDescription()
if ( str(val) <> snmpGet.GetErrorVal()):
#if ( str(val) <> "Connection refused"):
	print str(val)
	print"Test_failed "
	exit (1)
print "Description : " + str(val)

print "________Good !!! the  disable snmp succeed"
if os.system (command) == 0:
	print "snmp port is after disable snmp- test failed"
	sys.exit(1)

print "________5___Enable the snmpd again _______"
c.LoadXmlFile('Scripts/TransSnmpSet.xml')
c.ModifyXml("SNMP_DATA","LOCATION","new_location_test")
c.ModifyXml("SNMP_DATA","CONTACT_NAME","contact_name_test")
c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","public")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
c.Send()
#c.PrintLastResponse()
c.SendXmlFile('Scripts/TransSnmpGet.xml')
#c.PrintLastResponse()
sleep (5)
print "________6___Trap get Location_____________"

val = snmpGet.GetLocation()
print "Location    : " + str(val)
if ( str(val) <> "new_location_test"):
   print"Test_failed -Enabeld Snmpd failed"
   exit (1)
if os.system (command) <> 0:
	print "snmp port not found after enabled snmp- test failed"
	sys.exit(1)
print "________Good !!! the  Enable snmp succeed_"
c.Disconnect()
exit (0)





