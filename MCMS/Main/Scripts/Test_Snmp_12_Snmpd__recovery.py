#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_11
from McmsTargetConnection import *
from McmsConnection import *
#from test_recovery_policy_Utilities import *
#-- SKIP_ASSERTS
#-LONG_SCRIPT_TYPE

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

import os, string
import sys
import shutil

from SNMPUtils import *

sleep(2)




c = McmsConnection()
c.Connect()
print "________1___SnmpGet_______________________"
c.SendXmlFile('Scripts/TransSnmpGet.xml')
#c.PrintLastResponse()
sleep(1)
print "________2___Enable_snmp___________________"
c.LoadXmlFile('Scripts/TransSnmpSet.xml')
c.ModifyXml("SNMP_DATA","LOCATION","location_test")
c.ModifyXml("SNMP_DATA","CONTACT_NAME","contact_name_test")
c.ModifyXml("SECURITY","ACCEPT_ALL_REQUESTS","true")
c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","public")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
c.Send()
sleep (20)
print "________3___killall snmpd___________________"
temp_str= str("killall snmpd")
print temp_str
os.system(temp_str)
sleep (20)
#SM_fail = 0
SNMPD_fail =0
c.SendXmlFile("Scripts/GetFaults.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
            fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
            print fault_desc
            if fault_desc =="SNMPD_FAILURE":
               SNMPD_fail = 1

if SNMPD_fail <> 1 :
     print SNMPD_fail
     print "Test failed,  SNMPD_FAILUER not found in Faults"
     sys.exit(1)
print "________4___SNMPD_FAILURE fault found________"
sleep (20)
print "________5___change community name ___________"
c.LoadXmlFile('Scripts/TransSnmpSet.xml')
c.ModifyXml("SNMP_DATA","LOCATION","location_test")
c.ModifyXml("SNMP_DATA","CONTACT_NAME","contact_name_test")
c.ModifyXml("SNMP_DATA","SYSTEM_NAME","system_name_test")
c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
c.ModifyXml("SECURITY","TRAP_VERSION","snmpv1")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_NAME","public")
c.ModifyXml("COMMUNITY_PERMISSION_LIST","COMMUNITY_PERMISSION_SPECIFIC","snmp_read_only")
c.ModifyXml("SNMP_DATA","SNMP_ENABLED","true")
c.Send()
sleep(2)
print ""
snmpGet = SNMPGetUtils()
version=1

val = snmpGet.GetDescription(version)
print "Description : " + str(val)

val = snmpGet.GetLocation(version)
print "Location    : " + str(val)
if ( str(val) <> "location_test"):
   print"oy oy oy ... wrong location value - test failed "
   exit (1)
else:
   print "________6___Test paseed snmpd work again ______"
exit (0)