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

p.DelUser("USER2","You are not authorized to perform this operation") 

r_in_list=0    
r_in_list=r.CheckIfUserExistInTheUsersList("USER2","USER2_pwd","operator")
if r_in_list<>1:
    print "Test failed, permission conflict!!!  A new user USER2 was deletd by USER1 -an ordinary user."
#    sys.exit(1) 
p.Disconnect()
r.Disconnect()



