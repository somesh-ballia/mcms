#!/mcms/python/bin/python

from SysCfgUtils import *
from UsersUtils import *
import os

os.system("rm Cfg/EncOperatorDB.xml")
os.system("rm Cfg/SystemCfgUser.xml")
os.system("rm Cfg/SystemCfgUserTmp.xml")
os.system("echo YES > /mcms/JITC_MODE.txt");
os.system("echo YES > /tmp/OLD_JITC_MODE.txt");
os.system("Scripts/Destroy.sh")
os.system("Scripts/Startup.sh")

sysCfgUtils = SyscfgUtilities()
sysCfgUtils.ConnectSpecificUser("POLYCOM", "POLYCOM")

print "Setting system.cfg to Federal mode."
sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "MIN_PASSWORD_LENGTH", "15", "user")
jitc_mode="YES"
force_pwd_policy="YES"
sysCfgUtils.SetFederalMode(jitc_mode, force_pwd_policy, False)

#sysCfgUtils.Sleep(60)

print "Adding new operator..."
usersUtils = UsersUtilities()
usersUtils.ConnectSpecificUserFederalFirstLogin("POLYCOM", "POLYCOM", "Abst_45Agk$1234")
usersUtils.AddNewUser("TESTER","Test_45Abc#12345","operator")
usersUtils.Disconnect()

print "\nTest First Login, and change password..."
usersUtils.ConnectSpecificUserFederalFirstLogin("TESTER","Test_45Abc#12345","123AbcD_#p12345")
print "-Password changed successfully."

usersUtils.Disconnect()
usersUtils.Sleep(5)

print "-------------------------------------------------------------\n"
print "    Testing login history \n"
print "-------------------------------------------------------------\n"

num = 10
for i in range(num+1):
    expected_status = "Invalid Login"
    usersUtils.ConnectSpecificUser("TESTER","Test_45Abc#12345#",expected_status)
    print "User with wrong password could not login " + str(i) + "..."
    usersUtils.Sleep(5)

''' Check of history reset after successful login
expected_status = "Status OK"
usersUtils.ConnectSpecificUser("TESTER","123AbcD_#p12345",expected_status)
print "User with good password has been login..."
usersUtils.Disconnect()
'''

usersUtils.Sleep(5)
print "Setting system.cfg to regular mode."
jitc_mode="NO"
force_pwd_policy="NO"
sysCfgUtils.ConnectSpecificUser("POLYCOM", "Abst_45Agk$1234")
sysCfgUtils.SetFederalMode(jitc_mode, force_pwd_policy)       
 

print "\nTest was completed successfully !!! Please open the 'Cfg/EncOperatorDB.xml' file in order to check the failures history.\n" 
 
