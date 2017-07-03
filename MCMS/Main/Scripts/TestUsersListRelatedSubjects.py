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
r.AddNewUser("USER4","USER4_pwd","operator")
sleep (5)
r_in_list=r.CheckIfUserExistInTheUsersList("USER1","USER1","administrator")
if r_in_list==0:
    print "Test failed, The new user does not appear in the users list"
    sys.exit(1) 
print "good start - The new user appear in the users list"
r_in_list=0   
j.ConnectSpecificUser("USER1","USER1_pwd")
r_in_list=r.CheckIfUserExistInTheConnectionList("USER1","USER1_pwd","administrator")
if r_in_list==0:
    print "Test failed, The new user does not appear in the connection list after the connect action"
    sys.exit(1) 
print "good - The new user appear in the connection list after the connect actionThe new user appear in the users list"

#sleep (5)
r.DelUser("USER1")
r.DelUser("USER2")
r.DelUser("USER3")
r.DelUser("USER4 ")

#r.CheckIfUserExistInTheUsersList("USER2","USER3","operator")
#j.ConnectSpecificUser("USER1","USER1_pwd")
#r.CheckIfUserExistInTheConnectionList("USER1","USER1_pwd","administrator")
#os.system("export CLEAN_CFG=NO")
#os.system("Scripts/Startup.sh")
#os.system("Scripts/SocketDisconnectConnect.py")

r.Disconnect()

#r = UsersUtilities()
#j= UsersUtilities()
#r.Connect()
#r.AddNewUser("USER1","USER1_pwd","administrator")
#r.AddNewUser("USER2","USER2_pwd","administrator")
#r.AddNewUser("USER3","USER3_pwd","operator")
#r.AddNewUser("USER4","USER4_pwd","operator")
#sleep (5)
#r.CheckIfUserExistInTheUsersList("USER2","USER3","operator")
#sleep (5)
#r.DelUser("USER2")
#r.CheckIfUserExistInTheUsersList("USER2","USER3","operator")
#j.ConnectSpecificUser("USER1","USER1_pwd")
#os.system("export CLEAN_CFG=NO")
#os.system("Scripts/Startup.sh")
#os.system("Scripts/SocketDisconnectConnect.py")

r.Disconnect()
