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

r.AddNewUser("USER1","USER1_pwd","operator")
p.ConnectSpecificUser("USER1","USER2_pwd","Invalid Login name")


r.Disconnect()



