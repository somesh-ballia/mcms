#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6
import os
import sys
import shutil

from UsersUtils import *
# _1_TO_OPERATOR_NAME_LENGTH 18
############################################
# 
##############################################
r_in_list=0
r = UsersUtilities()
j = UsersUtilities()
o = UsersUtilities()
p = UsersUtilities()
"""
r.Connect()
r.AddNewUser("123456789012345678901234567890","123456789012345678901234567890","administrator")
r.AddNewUser("USER2<<<<_>>>U","USER2_pwd","administrator")
r.AddNewUser("USER3~``:;?()*&U","USER3_pwd","operator")
sleep (1)
#r_in_list=r.CheckIfUserExistInTheUsersList("123456789012345678","USER1_pwd","administrator")
r_in_list=r.CheckIfUserExistInTheUsersList("12345678901234567890","USER1_pwd","administrator")
if r_in_list==0:
    print "Test failed, The new user 123456789012345678 does not appear in the users list"
    sys.exit(1) 

r_in_list=0    
r_in_list=r.CheckIfUserExistInTheUsersList("USER2<<<<_>>>U","USER2_pwd","administrator")
if r_in_list==0:
    print "Test failed, The new user USER2 does not appear in the users list"
    sys.exit(1) 

r_in_list=0   
r_in_list=r.CheckIfUserExistInTheUsersList("USER3~``:;?()*&U","USER3_pwd","operator")
if r_in_list==0:
    print "Test failed, The new user USER3 does not appear in the users list"
    sys.exit(1) 
    
    
    
print "good start - The new users appear in the users list"
r_in_list=0   
#j.ConnectSpecificUser("12345678901234567890","123456789012345678901234567890")
j.ConnectSpecificUser("12345678901234567890","12345678901234567890")
o.ConnectSpecificUser("USER2<<<<_>>>U","USER2_pwd")
p.ConnectSpecificUser("USER3~``:;?()*&U","USER3_pwd")

r.Disconnect()
j.Disconnect()
o.Disconnect()
p.Disconnect()
"""

