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
r.AddNewUser("USER2","USER2_pwd","operator")

p.ConnectSpecificUser("USER1","USER1_pwd")


p.UpdatePwd("USER2","USER2_pwd","USER2_new_pwd","You are not authorized to perform this operation")
p.ConnectSpecificUser("USER2","USER2_pwd")


r.Disconnect()



