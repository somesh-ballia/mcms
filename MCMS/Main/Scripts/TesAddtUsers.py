#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6
import os
import sys
import shutil

from UsersUtils import *

############################################
# 	Scripts for test Carmel's behavior related to Add ,remove and update 
#   (Change passwords and permissions) user in the Users List:
#6.1. Add an administrator user C and Connect C. 
#                  Expectation: get the Users list and see the new user C .
#6.2. From C add an operator user A.
#       Expectation: get the Users list and see the new users A and C.
#6.3. Connect user A.
#                  Expectation: The Login should succeed.
#6.2	.Change the A password  From  C 
#       (a) The change password activity should succeed.
#6.3	Disconnect  A and reconnect (with the old password)       
#        Expectation: (a) The reconnect should be rejected.
#6.4	Reconnect A with the new password.
#       Expectation: (a) The reconnect should  succeed.
#6.5	.Change the C password  From  A 
#       Expectation: (a) The change password activity should reject.

##############################################
r_in_list=0
r = UsersUtilities()
j = UsersUtilities()
o = UsersUtilities()
p = UsersUtilities()

###################        step #1        ###########################
r.Connect()
r.AddNewUser("adminC","PadminC","administrator")
sleep (1)
j.ConnectSpecificUser("adminC","PadminC")
#sleep (1)
r_in_list=r.CheckIfUserExistInTheConnectionList("adminC","PadminC","administrator")
#r_in_list=r.CheckIfUserExistInTheUsersList("adminC","PadminC","administrator")
if r_in_list==0:
    print "Test failed, The new user adminC does not appear in the connection list"
    sys.exit(1) 
print "step1: OK - The new user adminC appear in the users list after the connect "
###################        step #2        ###########################
r_in_list=0
j.AddNewUser("operA","PoperA","operator")
sleep (1)
r_in_list=r.CheckIfUserExistInTheUsersList("operA","PoperA","operator")
if r_in_list==0:
    print "Test failed, The new user operA does not appear in the users list"
    sys.exit(1) 
print "step1: OK - The new user operA appear in the users list "
###################        step #3        ###########################

o.ConnectSpecificUser("operA","PoperA")


#r.AddNewUser("Shlomit","pezem","administrator")
#r.AddNewUser("Libi","Loblob","operator")
#r.AddNewUser("Maor","Somsom","operator")
#sleep (1)
#r_in_list=r.CheckIfUserExistInTheUsersList("Shlomit","Libi","operator")
#if r_in_list==0:
#    print "Test failed, The new user does not appear in the users list"
#    sys.exit(1) 
#print "good start - The new user appear in the users list"
#r_in_list=0   
#j.ConnectSpecificUser("Miko","kesem")
#r_in_list=r.CheckIfUserExistInTheConnectionList("Miko","kesem","administrator")
#if r_in_list==0:
#    print "Test failed, The new user does not appear in the connection list after the connect action"
#    sys.exit(1) 
#print "good - The new user appear in the connection list after the connect actionThe new user appear in the users list"

#sleep (5)
#r.DelUser("Miko")
#r.DelUser("Shlomit")
#r.DelUser("Libi")
#r.DelUser("Maor ")

#r.CheckIfUserExistInTheUsersList("Shlomit","Libi","operator")
#j.ConnectSpecificUser("Miko","kesem")
#r.CheckIfUserExistInTheConnectionList("Miko","kesem","administrator")
#os.system("export CLEAN_CFG=NO")
#os.system("Scripts/Startup.sh")
#os.system("Scripts/SocketDisconnectConnect.py")

r.Disconnect()

#r = UsersUtilities()
#j= UsersUtilities()
#r.Connect()
#r.AddNewUser("Miko","kesem","administrator")
#r.AddNewUser("Shlomit","pezem","administrator")
#r.AddNewUser("Libi","Loblob","operator")
#r.AddNewUser("Maor","Somsom","operator")
#sleep (5)
#r.CheckIfUserExistInTheUsersList("Shlomit","Libi","operator")
#sleep (5)
#r.DelUser("Shlomit")
#r.CheckIfUserExistInTheUsersList("Shlomit","Libi","operator")
#j.ConnectSpecificUser("Miko","kesem")
#os.system("export CLEAN_CFG=NO")
#os.system("Scripts/Startup.sh")
#os.system("Scripts/SocketDisconnectConnect.py")

r.Disconnect()
