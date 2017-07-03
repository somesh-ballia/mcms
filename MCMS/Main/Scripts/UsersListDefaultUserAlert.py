#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script which test system's behavior when default user (login: 'POLYCOM') exists in Users list (Operators list).
# The expected behavior:
#    - 'DEFAULT_USER_EXIST' Alert is produced
# By: Haggai
#############################################################################




import os
import sys


# start
os.system("Scripts/Startup.sh")

from McmsConnection import *
c = McmsConnection()
c.Connect()



#-------------------------------------------------------------------------------
# ------ 1. check Alerts - ensure 'DEFAULT_USER_EXISTS' is displayed
#-------------------------------------------------------------------------------
print ""
print "Monitor active alarms..."

c.SendXmlFile("Scripts/GetActiveAlarms.xml")
sysAlerts_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_Alerts = len(sysAlerts_list)

userListCorrupted = 0
for index in range(num_of_Alerts):
    Alert_desc = sysAlerts_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print Alert_desc
    if "DEFAULT_USER_EXISTS" == Alert_desc :
        userListCorrupted = userListCorrupted+1


# -------- test failed
if userListCorrupted <> 1:
	print "Test failed, DEFAULT_USER_EXISTS is not found in System Alerts"
	sys.exit(1)
else:
	print "----- Phase 1 succeeded: DEFAULT_USER_EXISTS Alert was produced at startup"




#-------------------------------------------------------------------------------
# ------ 2. delete default user - ensure 'DEFAULT_USER_EXISTS' Alert is removed
#-------------------------------------------------------------------------------
from UsersUtils import *

print ""
print "Delete Default user..."
r = UsersUtilities()
r.ConnectSpecificUser("POLYCOM","POLYCOM")
r.DelUser("POLYCOM","Status OK") 

sleep(1)

print ""
print "Monitor active alarms..."

c.SendXmlFile("Scripts/GetActiveAlarms.xml")
sysAlerts_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_Alerts = len(sysAlerts_list)

defUserAlert = 0
for index in range(num_of_Alerts):
    Alert_desc = sysAlerts_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print Alert_desc
    if "DEFAULT_USER_EXISTS" == Alert_desc :
        defUserAlert = defUserAlert+1

# -------- test failed
if defUserAlert <> 0:
	print "Test failed, DEFAULT_USER_EXISTS is found in System Alerts although the user was deleted"
	r.Disconnect()
	sys.exit(1)
else:
	print "----- Phase 2 succeeded: DEFAULT_USER_EXISTS Alert was removed after default user was deleted"




#-------------------------------------------------------------------------------
# ------ 3. add default user - ensure 'DEFAULT_USER_EXISTS' Alert is produced
#-------------------------------------------------------------------------------
print ""
print "Add Default user..."
r.AddNewUser("POLYCOM","POLYCOM","administrator")

sleep(1)

print ""
print "Monitor active alarms..."

c.SendXmlFile("Scripts/GetActiveAlarms.xml")
sysAlerts_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_Alerts = len(sysAlerts_list)

defUserAlert = 0
for index in range(num_of_Alerts):
    Alert_desc = sysAlerts_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print Alert_desc
    if "DEFAULT_USER_EXISTS" == Alert_desc :
        defUserAlert = defUserAlert+1

# -------- test failed
if defUserAlert <> 1:
	print "Test failed, DEFAULT_USER_EXISTS is not found in System Alerts although the user was added"
	r.Disconnect()
	sys.exit(1)
else:
	print "----- Phase 3 succeeded: DEFAULT_USER_EXISTS Alert was produced as default user was added"




# -------- test succeeded
r.Disconnect()
print ""
print "Test succeeded"
sys.exit(0)

