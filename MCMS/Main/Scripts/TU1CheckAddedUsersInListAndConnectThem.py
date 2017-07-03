#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6
import os
import sys
import shutil

from UsersUtils import *

############################################
# 
##############################################
r_in_list=0
r = UsersUtilities()
j = UsersUtilities()
o = UsersUtilities()
p = UsersUtilities()

r.Connect()
r.AddNewUser("USER1","USER1_pwd","administrator")
r.AddNewUser("USER2","USER2_pwd","administrator")
r.AddNewUser("USER3","USER3_pwd","operator")
sleep (1)
r_in_list=r.CheckIfUserExistInTheUsersList("USER1","USER1_pwd","administrator")
if r_in_list==0:
    print "Test failed, The new user USER1 does not appear in the users list"
    sys.exit(1) 

r_in_list=0    
r_in_list=r.CheckIfUserExistInTheUsersList("USER2","USER2_pwd","administrator")
if r_in_list==0:
    print "Test failed, The new user USER2 does not appear in the users list"
    sys.exit(1) 

r_in_list=0   
r_in_list=r.CheckIfUserExistInTheUsersList("USER3","USER3_pwd","operator")
if r_in_list==0:
    print "Test failed, The new user USER3 does not appear in the users list"
    sys.exit(1) 
    
    
    
print "good start - The new users appear in the users list"
r_in_list=0   
j.ConnectSpecificUser("USER1","USER1_pwd")
o.ConnectSpecificUser("USER2","USER2_pwd")
p.ConnectSpecificUser("USER3","USER3_pwd")

r.Disconnect()
j.Disconnect()
o.Disconnect()
p.Disconnect()


