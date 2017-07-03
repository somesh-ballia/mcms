#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6
import os
import sys
import shutil

from UsersUtils import *
r_in_list=0
r = UsersUtilities()
p = UsersUtilities()
r.Connect()

# by default there are three users : POLYCOM, SUPPORT and AUDIT; this script expects for one user only(POLYCOM)
r.DelUser("AUDIT")
r.DelUser("SUPPORT")

for i in range(99):
    r.AddNewUser("USER"+str(i),str(i),"operator")
r.AddNewUser("USER2","USER2_pwd","operator","Maximum number of user connections exceeded")
r.Disconnect()

