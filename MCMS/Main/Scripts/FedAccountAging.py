#!/mcms/python/bin/python

from SysCfgUtils import *
from UsersUtils import *
from time import *
from datetime import *
import os

os.environ["CLEAN_CFG"]="YES"
os.system("Scripts/StartupWithFederal.sh")
os.environ["CLEAN_CFG"]="NO"

sysCfgUtils = SyscfgUtilities()
sysCfgUtils.Sleep(5)

usersUtils = UsersUtilities()
usersUtils.ConnectSpecificUserFederalFirstLogin("POLYCOM", "POLYCOM", "Kolm_45Agk$1234")

usersUtils.AddNewUser("TESTER2","Tess_45Abc$1234","administrator")
usersUtils.Disconnect()
sysCfgUtils.Sleep(5)
usersUtils.ConnectSpecificUserFederalFirstLogin("TESTER2", "Tess_45Abc$1234", "Kolm_45Agk$1234")

sysCfgUtils.Sleep(5)

print "----------------------------------------------------------------------\n"
print "    Testing Inactive user account:\n"
print "    Current setting: User account expires after 30 days of inactivity.\n"
print "----------------------------------------------------------------------\n"

print "Modifying last login time ..."
usersUtils.UpdateUserLastLoginTimeInOperatorDB("POLYCOM", 92, 3, 30)
print "Modifying last login time ..."
usersUtils.UpdateUserLastLoginTimeInOperatorDB("TESTER2", 95, 3, 30)
print "Modifying last login time ..."
usersUtils.UpdateUserLastLoginTimeInOperatorDB("SUPPORT", 97, 3, 30)

expected_status = "Account is disabled"
usersUtils.ConnectSpecificUser("TESTER2", "Kolm_45Agk$1234", expected_status)
print "\nTest user account is disabled...OK\n"

#os.system("Scripts/Flush.sh")

usersUtils.Sleep(10)

print "\nTest was completed successfully !!!! \n" 


