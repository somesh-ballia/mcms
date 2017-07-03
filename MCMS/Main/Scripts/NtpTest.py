#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6
import os
import sys
import shutil
#*CLEAN_CFG = NO

from SysCfgUtils import *
from UsersUtils import *


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Direct the MCU to work with external DB (system.cfg)~~~~~~~~~
r = SyscfgUtilities()
j = UsersUtilities()
p = UsersUtilities()
o = UsersUtilities()
q = UsersUtilities()
s = UsersUtilities()


r.Connect()
#r.UpdateSyscfgFlag("MCMS_PARAMETERS_USER","ENABLE_EXTERNAL_DB_ACCESS","YES","user")
                                           
sleep (1)
r.UpdateSyscfgFlag("MCMS_PARAMETERS_USER","EXTERNAL_DB_IP","172.22.188.157","user")
sleep (1)
r.UpdateSyscfgFlag("MCMS_PARAMETERS_USER","EXTERNAL_DB_PORT","5005","user")
sleep (1)
r.UpdateSyscfgFlag("MCMS_PARAMETERS_USER","EXTERNAL_DB_LOGIN","POLYCOM","user")
sleep (1)
r.UpdateSyscfgFlag("MCMS_PARAMETERS_USER","EXTERNAL_DB_PASSWORD","POLYCOM","user")
sleep (1)
r.UpdateSyscfgFlag("MCMS_PARAMETERS_USER","EXTERNAL_DB_DIRECTORY","A","user")
sleep (1)
r.UpdateSyscfgFlag("MCMS_PARAMETERS_USER","ENABLE_EXTERNAL_DB_ACCESS","YES","user")

##	EXTERNAL_DB_IP=172.22.188.157
#	EXTERNAL_DB_PORT=5005
#	EXTERNAL_DB_LOGIN=POLYCOM
#	EXTERNAL_DB_PASSWORD=POLYCOM
#	EXTERNAL_DB_DIRECTORY=A

sleep (10)
os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")


sleep (10)
o.Connect()
#~~~~~~~~~~~~~~~~~~~~~Connect operator user  located in the External DB~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

print "Connect the operator user Night_test_user located in the External DB"
j.ConnectSpecificUser("Night_test_user","Night_test_user")
sleep (1)
r_in_list=o.CheckIfUserExistInTheConnectionList("Night_test_user","Night_test_user","aoperator")
#if r_in_list==0:
#    print "Test failed, The new user Night_test_user does not appear in the users list"
#    sys.exit(1) 


#~~~~~~~~~~~~~~~~~~~~~Connect administrator user  located in the External DB~~~~~~~~~~~~~~~~~~~~~~~~~~~~
print "Connect the administrator user Night_test_admin located in the External DB"
p.ConnectSpecificUser("Night_test_admin","Night_test_admin")
r_in_list=o.CheckIfUserExistInTheConnectionList("Night_test_admin","Night_test_admin","moderator")
if r_in_list==0:
    print "Test failed, The new user Night_test_user does not appear in the users list"
#    sys.exit(1) 
#~~~~~~~~~~~~~~~~~~~~~Connect the user that located in the internal (Add a new user same as in the external db 
#                                                                    but with diffrent autorizeation group )
# we expect the user to connect as an oprator 

print "Add the new user Night_test_prem (operator) same  name as in the external db but with diffrent autorizeation (admin)in the External DB"
print "Connect the user Night_test_prem we expect the user to connect as an oprator (as in the internal DB)"
print "The operator user sapose to fail in add user action"

o.AddNewUser("Night_test_prem","Night_test_prem","operator")
q.ConnectSpecificUser("Night_test_prem","Night_test_prem")
r_in_list=o.CheckIfUserExistInTheConnectionList("Night_test_prem","Night_test_prem","aoperator")
if r_in_list==0:
    print "Test failed, The new user Night_test_user does not appear in the users list"
    sys.exit(1) 
q.AddNewUser("Night_test_no_prem","Night_test_no_prem","operator","You are not authorized to perform this operation")


#~~~~~~~~~~~~~~~~~~~~~Connect user located in the External DB with worng password ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

s.ConnectSpecificUser("Night_test_prem","Night_test_prem1","Invalid Login name")

