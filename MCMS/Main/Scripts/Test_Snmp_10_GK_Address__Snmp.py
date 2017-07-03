#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_11
from McmsConnection import *
from SNMPUtils import *

#snmpGet = SNMPGetUtils()
#val = snmpGet.GetDescription()
#print "Description : " + str(val)

c = McmsConnection()
c.Connect()

print "________1___Disabled GK Address _______________________"

c.SendXmlFile('Scripts/Update_IP_service-GK_as__NO.xml')
os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")
c.Connect()
sleep (4)
exit(0)
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

print "________3___Test_disabled_Gatekeeper Address_________"
sleep (4)
snmpGet = SNMPGetUtils()
val = snmpGet.GetGatekeeperAddress()
print "GatekeeperAddress : " + str(val)
if ( str(val) <> ""):
   print"oy oy oy ... wrong GatekeeperAddress "
   exit (1)
   
print "________4___Enabled GK Address (without reset)_______________________"
sleep(2)
c.Connect()
c.LoadXmlFile('Scripts/Update_IP_service-GK_as__NO.xml')
c.ModifyXml("IP_SERVICE","GATEKEEPER_TYPE","specify")
#c.ModifyXml("IP_SERVICE","GATEKEEPER_TYPE","external")
#c.ModifyXml("GATEKEEPER","EXTERNAL_GATEKEEPER_ADDRESS","172.22.188.152")
c.Send()

print "________5___Test_disabled_Gatekeeper Address_________"
sleep (4)
snmpGet = SNMPGetUtils()
val = snmpGet.GetGatekeeperAddress()
print "GatekeeperAddress : " + str(val)
if ( str(val) <> ""):
   print"oy oy oy ... wrong GatekeeperAddress "
   exit (1)
print "________6___reset- (Activate the Enabled GK Address)______________"   
os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")
c.Connect() 
sleep(8)


print "________7___Test_GK Address______________"
snmpGet = SNMPGetUtils()
val = snmpGet.GetGatekeeperAddress()
print "GatekeeperAddress : " + str(val)
if ( str(val) <> "172.22.188.152"):
   print"oy oy oy ... wrong GatekeeperAddress  - test failed"
   exit (1)

print "________Good !!! the  GatekeeperAddress snmp test succeed_"
c.Disconnect()
exit (0)





