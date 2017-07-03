#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

import os
import sys

from UsersUtils import *


#----------------------------------------------------------------------
# start test
#----------------------------------------------------------------------



badUserName = "@"
badPassword = "sdsddddddddd s"

goodUserName = "abcdzABCDZ0129.-_5"
goodPassword = "abcdABCD1!"

userUtils = UsersUtilities()
userUtils.Connect()
userUtils.AddNewUser(badUserName, goodPassword, "operator", 'Login name contains invalid characters')


userUtils.AddNewUser(goodUserName, badPassword, "operator", 'Password contains invalid characters')

userUtils.AddNewUser(goodUserName, goodPassword, "operator", 'Status OK')
 
userUtils.Disconnect()
