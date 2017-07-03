#!/mcms/python/bin/python

from SysCfgUtils import *
from UsersUtils import *
import os

os.environ["CLEAN_CFG"]="YES"
os.system("Scripts/StartupWithFederal.sh")
os.environ["CLEAN_CFG"]="NO"

sysCfgUtils = SyscfgUtilities()
sysCfgUtils.Sleep(5)

usersUtils = UsersUtilities()
usersUtils.ConnectSpecificUserFederalFirstLogin("POLYCOM", "POLYCOM", "Abst_45Agk$1234")

print "Adding new operator..."
usersUtils.AddNewUser("TESTER","Test_45Abc#12345","operator")
usersUtils.Disconnect()

print "\nTest First Login, and change password..."
usersUtils.ConnectSpecificUserFederalFirstLogin("TESTER","Test_45Abc#12345","123AbcD_#p12345")
print "-Password changed successfully."

usersUtils.Disconnect()
sysCfgUtils.Sleep(5)
expected_status = "Invalid Login name"
usersUtils.ConnectSpecificUser("TESTER","Test_45Abc12345#",expected_status)
print "User With Old Password could not login."

expected_status = "Status OK"
usersUtils.ConnectSpecificUser("TESTER","123AbcD_#p12345",expected_status)
print "User With New Password logged in successfully."

usersUtils.Disconnect()
print "Resetting ...\n"
os.system("Scripts/Destroy.sh")
os.system("Scripts/StartupWithFederal.sh")

sysCfgUtils.ConnectSpecificUser("TESTER","123AbcD_#p12345",expected_status)
print "\nUser logged without need to change password (not first login case).\n"
sysCfgUtils.Disconnect()

print "Setting system.cfg to regular mode."
jitc_mode="NO"
force_pwd_policy="NO"
sysCfgUtils.ConnectSpecificUser("POLYCOM", "Abst_45Agk$1234")
sysCfgUtils.SetFederalMode(jitc_mode, force_pwd_policy)       

print "\nTest was completed successfully !!! \n"
