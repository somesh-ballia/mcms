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

print "Rename POLYCOM operator to TESTER..."
usersUtils.LoadXmlFile("Scripts/RenameNewOperator.xml")
usersUtils.ModifyXml("OPERATOR", "OLD_USER_NAME", "POLYCOM")
usersUtils.ModifyXml("OPERATOR", "NEW_USER_NAME", "TESTER")
usersUtils.Send()

usersUtils.DelUser("SUPPORT")
usersUtils.DelUser("AUDIT")
usersUtils.AddNewUser("TESTER2","Tess_45Abc$1234","administrator")
usersUtils.Disconnect()
sysCfgUtils.Sleep(5)

print "\nTest First Login, and change password..."
usersUtils.ConnectSpecificUserFederalFirstLogin("TESTER","Kolm_45Agk$1234","Abst_45Agk$1234")
print "-Password changed successfully."

print "-------------------------------------------------------------\n"
print "    Testing password change freq. limitation\n"
print "    Current setting: password can be changed once a day\n"
print "-------------------------------------------------------------\n"

print "Adding new operator..."
usersUtils.AddNewUser("USER","Just_45Abc$1234","operator",)
usersUtils.Disconnect()
usersUtils.ConnectSpecificUserFederalFirstLogin("USER","Just_45Abc$1234","Abst_45Agk$1234")
                                                
sysCfgUtils.Sleep(5)
expected_status = "Password change is not allowed before defined min time has passed"
usersUtils.UpdatePwd("USER","Abst_45Agk$1234","KOko_@34tra1234",expected_status)
print "\nUser Can not change password faster than defined....OK"
usersUtils.Disconnect()

print "Modifying password change time ...\n"
usersUtils.UpdateUserLastPwdChangedInOperatorDB("TESTER", 1, 3, 30)

#sysCfgUtils.Sleep(80) 

usersUtils.ConnectSpecificUser("TESTER", "Abst_45Agk$1234")
pwdExpirationWarningDays = usersUtils.GetTextUnder("LOGIN","PASSWORD_EXPIRATION_WARNING_DAYS")
pwdExpirationDaysLeft = usersUtils.GetTextUnder("LOGIN","PASSWORD_EXPIRATION_DAYS_LEFT")
print "pwdExpirationDaysLeft:" + pwdExpirationDaysLeft

if pwdExpirationWarningDays != "7":
    usersUtils.Disconnect()
    ScriptAbort("Wrong Expiration Warning Days result: " + pwdExpirationWarningDays)
    
if pwdExpirationDaysLeft != "59":
    usersUtils.Disconnect()
    ScriptAbort("Wrong Expiration Days Left result")

usersUtils.DelUser("USER")

expected_status = "Status OK"
usersUtils.UpdatePwd("TESTER","Abst_45Agk$1234","KOko_@34tra1234",expected_status)
print "User password changed successfully."

print "-------------------------------------------------------------\n"
print "    Testing password aging:\n"
print "    Current setting: Password expires after 60 days.\n"
print "-------------------------------------------------------------\n"

usersUtils.Disconnect()
print "Modifying password change time ..."
usersUtils.UpdateUserLastPwdChangedInOperatorDB("TESTER", 90, 3, 30)

expected_status = "User must change password"
usersUtils.ConnectSpecificUser("TESTER", "KOko_@34tra1234", expected_status)
print "\nTest user account is expired...OK"

expected_status = "Status OK"
usersUtils.ConnectSpecificUserFederalFirstLogin("TESTER","KOko_@34tra1234","7890_lAMA_$ab45")
print "\nUser changed password and logged in ...OK"
usersUtils.Disconnect()

usersUtils.ConnectSpecificUserFederalFirstLogin("TESTER2","Tess_45Abc$1234","7890_lAMA_$ab45")
usersUtils.Disconnect()
print "Modifying password change time ..."
usersUtils.UpdateUserLastPwdChangedInOperatorDB("TESTER2", 90, 3, 30)
sleep(5)

expected_status = "User must change password"
usersUtils.ConnectSpecificUser("TESTER2", "7890_lAMA_$ab45", expected_status)
usersUtils.ConnectSpecificUser("TESTER2", "7890_lAMA_$ab45", expected_status)
usersUtils.ConnectSpecificUser("TESTER2", "7890_lAMA_$ab45", expected_status)

expected_status = "Account is disabled"
usersUtils.ConnectSpecificUser("TESTER2", "7890_lAMA_$ab45", expected_status)
print "\nTest an expired user account is disabled after 3 login attempts without changing the old password to a new one...OK\n"

print "----------------------------------------------------------------------\n"
print "    Testing Inactive user account:\n"
print "    Current setting: User account expires after 30 days of inactivity.\n"
print "----------------------------------------------------------------------\n"

print "Modifying last login time ..."
usersUtils.UpdateUserLastLoginTimeInOperatorDB("TESTER", 95, 3, 30)

usersUtils.ConnectSpecificUser("TESTER", "7890_lAMA_$ab45")
print "\nTest last admin account is never disabled...OK\n"

print "Adding new operator..."
usersUtils.AddNewUser("USER","Usrr_45Abc$1234","operator")
usersUtils.Disconnect()

usersUtils.ConnectSpecificUserFederalFirstLogin("USER","Usrr_45Abc$1234","7890_lAMA_$ab45")
usersUtils.Disconnect()

usersUtils.UpdateUserLastLoginTimeInOperatorDB("USER", 90, 3, 30)
usersUtils.UpdateUserLastPwdChangedInOperatorDB("USER", 90, 3, 30)

expected_status = "Account is disabled"
usersUtils.ConnectSpecificUser("USER", "7890_lAMA_$ab45", expected_status)
print "\nTest user account is disabled...OK\n"

usersUtils.ConnectSpecificUser("TESTER", "7890_lAMA_$ab45")

print "Enabling USER ..."
usersUtils.SendXmlFile("Scripts/EnableUser.xml")
sleep (2)
usersUtils.Disconnect()

expected_status = "User must change password"
usersUtils.ConnectSpecificUser("USER", "7890_lAMA_$ab45", expected_status)

sysCfgUtilsDebug = SyscfgUtilities()
sysCfgUtils.ConnectSpecificUser("TESTER", "7890_lAMA_$ab45")

print "Setting system.cfg to regular mode."
jitc_mode="NO"
force_pwd_policy="YES"
sysCfgUtils.SetFederalMode(jitc_mode, force_pwd_policy)    

sysCfgUtils.ConnectSpecificUser("POLYCOM", "POLYCOM")

jitc_mode="NO"
force_pwd_policy="NO"
sysCfgUtils.SetFederalMode(jitc_mode, force_pwd_policy)  

print "\nTest was completed successfully !!! \n" 


