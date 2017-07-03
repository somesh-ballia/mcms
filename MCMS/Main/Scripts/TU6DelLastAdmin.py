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

# by default there are two users : POLYCOM, SUPPORT, this script expects for one user only(POLYCOM)
r.DelUser("SUPPORT")

r.AddNewUser("USER1","USER1_pwd","operator")
r.AddNewUser("USER2","USER2_pwd","administrator")

p.ConnectSpecificUser("USER1","USER1_pwd")

r.DelUser("USER2")
#r.DelUser("POLYCOM") 
r.DelUser("POLYCOM","At least one Administrator must be defined in the MCU") 

r_in_list=0    
r_in_list=r.CheckIfUserExistInTheUsersList("POLYCOM","POLYCOM","administrator")
if r_in_list==0:
    print "Test failed, permission conflict!!!  The last administrator user was deleted"
    sys.exit(1) 
r.Disconnect()



