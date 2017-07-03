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

j.ConnectSpecificUser("USER1","USER1_pwd")
o.ConnectSpecificUser("USER2","USER2_pwd")
p.ConnectSpecificUser("USER3","USER3_pwd")

j.Disconnect()
o.Disconnect()
p.Disconnect()

r.DelUser("USER1") 
r.DelUser("USER2") 
r.DelUser("USER3")   
    
j.ConnectSpecificUser("USER1","USER1_pwd","Invalid Login name")
o.ConnectSpecificUser("USER2","USER2_pwd","Invalid Login name")
p.ConnectSpecificUser("USER3","USER3_pwd","Invalid Login name")

r.Disconnect()


