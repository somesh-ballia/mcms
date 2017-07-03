#!/mcms/python/bin/python

from McmsConnection import *
from SNMPUtils import *



c = McmsConnection()
c.Connect()

print "________2___Enable_snmp___________________"
c.LoadXmlFile('Scripts/TransSnmpSet.xml')
c.ModifyXml("SNMP_DATA","LOCATION","location_test")
c.ModifyXml("SNMP_DATA","CONTACT_NAME","contact_name_test")

c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","public")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
c.Send()

print "sleep (10)"
sleep (10)

print "________3___Test_enable_snmp______________"
snmpGet = SNMPGetUtils()
val = snmpGet.GetDescription()
print "Description : " + str(val)
if ( str(val) <> "RMX.0.0.0.0"):
   print"oy oy oy ... wrong system_name value "
   exit (1)

status = False
numOfIter = 10
for i in range(1, numOfIter):
   val = snmpGet.GetNumber_OfInterfaces()
   print "IF number   : " + str(val)
   if ( str(val) <> "5"):
      print"oy oy oy ... wrong IF number, try again " + str(i) + ":" + str(numOfIter)
      print "val: "+ str(val)
      sleep(1)
   else:
      status = True
      break

if status == False:
   print "Failed to get number of interfaces"
   exit(1)

print "________4___Test_paseed______________"  
exit (0)





