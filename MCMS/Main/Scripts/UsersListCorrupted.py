#!/usr/bin/python
#-- EXPECTED_ASSERT(3)=error(1):

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script which test system's behavior when Users list (Operators list) is corrupted.
# The expected behavior:
#    - 'POLYCOM' user is added as default
#    - Alert: USERS_LIST_CORRUPTED
# By: Haggai
#############################################################################




import os
import sys


# -------- copy corrupted file to Cfg folder
os.system("cp Scripts/UsersListCorrupted/UsersListCorrupted.xml Cfg/OperatorDB.xml")
os.environ["CLEAN_CFG"]="NO"

# start (using the corrupted file)
os.system("Scripts/Startup.sh")

from McmsConnection import *
c = McmsConnection()
c.Connect()

# -------- for next startups
os.environ["CLEAN_CFG"]="YES"


#-------------------------------------------------------------------------------
# -------- 1. check Users - ensure 'POLYCOM' is there (as default)
#-------------------------------------------------------------------------------
print ""
print "Monitor users list..."      

c.SendXmlFile("Scripts/GetUserListReq.xml")
users_list = c.xmlResponse.getElementsByTagName("OPERATOR")

num_of_users = len(users_list)
defaultUserFound = 0
for index in range(num_of_users):
    userName_from_list = users_list[index].getElementsByTagName("USER_NAME")[0].firstChild.data
    print userName_from_list
    if "POLYCOM" == userName_from_list :
        defaultUserFound = defaultUserFound+1

# -------- test failed
if defaultUserFound <> 1:
    print "Test failed, POLYCOM is not found in Users list"
    sys.exit(1)


#-------------------------------------------------------------------------------
# -------- 2. check Alerts - ensure 'USERS_LIST_CORRUPTED' is displayed
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
    if "USERS_LIST_CORRUPTED" == Alert_desc :
        userListCorrupted = userListCorrupted+1


# -------- test failed
if userListCorrupted <> 1:
    print "Test failed, USERS_LIST_CORRUPTED is not found in System Alerts"
    sys.exit(1)


# -------- test succeeded
print ""
print "Test succeeded: 'POLYCOM' was added at startup (as default), and USERS_LIST_CORRUPTED Alert is displayed"
sys.exit(0)

