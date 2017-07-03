#!/usr/bin/python

# For list of profiles look at RunTest.sh

#############################################################################
# Test Script which test system's behavior when Users list (Operators list) file does not exist.
# The expected behavior:
#    - 'POLYCOM' user is added as default
# By: Haggai
#############################################################################




import os
import sys


# -------- remove file to Cfg folder
os.system("rm Cfg/OperatorDB.xml")
os.environ["CLEAN_CFG"]="NO"

# start (with no OperatorDB file)
os.system("Scripts/Startup.sh")

from McmsConnection import *
c = McmsConnection()
c.Connect()

# -------- for next startups
os.environ["CLEAN_CFG"]="YES"


#-------------------------------------------------------------------------------
# -------- check Users - ensure 'POLYCOM' is there (as default)
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


# -------- test succeeded
print ""
print "Test succeeded: 'POLYCOM' was added at startup (as default)"
sys.exit(0)

