#!/mcms/python/bin/python

from SysCfgUtils import *
from UsersUtils import *
import os

os.environ["CLEAN_CFG"]="YES"
os.system("Scripts/StartupWithFederal.sh")
os.environ["CLEAN_CFG"]="NO"

sysCfgUtils = SyscfgUtilities()
sysCfgUtils.Sleep(5)

print "Rename POLYCOM operator to TESTER..."
usersUtils = UsersUtilities()
usersUtils.Sleep(5)
usersUtils.ConnectSpecificUserFederalFirstLogin("POLYCOM", "POLYCOM", "Abst_45Agk$1234")

usersUtils.LoadXmlFile("Scripts/RenameNewOperator.xml")
usersUtils.ModifyXml("OPERATOR", "OLD_USER_NAME", "POLYCOM")
usersUtils.ModifyXml("OPERATOR", "NEW_USER_NAME", "TESTER")
usersUtils.Send()

#usersUtils.AddNewUser("TESTER","Test_45Abc$1234","administrator")
#usersUtils.DelUser("POLYCOM")
#usersUtils.DelUser("SUPPORT")
#usersUtils.DelUser("AUDIT")
usersUtils.Disconnect()
sysCfgUtils.Sleep(5)

print "\nTest First Login, and change password..."
usersUtils.ConnectSpecificUserFederalFirstLogin("TESTER","Abst_45Agk$1234","Cdst_56Agk$1234")
print "-Password changed successfully."

usersUtils.Disconnect()
os.system("Scripts/Destroy.sh")
os.system("Scripts/StartupWithFederal.sh")
usersUtils.ConnectSpecificUser("TESTER", "Cdst_56Agk$1234")

print "\n-------------------------------------------------------------\n"
print "    Testing password change freq. limitation\n"
print "    Current setting: password can be changed once a day\n"
print "-------------------------------------------------------------\n"
sysCfgUtils.Sleep(5)

print "Adding new operator..."
usersUtils.AddNewUser("USER","Just_45Abc$1234","operator")
usersUtils.Disconnect()
usersUtils.ConnectSpecificUserFederalFirstLogin("USER","Just_45Abc$1234","Cdst_56Agk$1234")

expected_status = "Password change is not allowed before defined min time has passed"
usersUtils.UpdatePwd("USER","Cdst_56Agk$1234","KOko_@34tra1234",expected_status)
print "User Can not change password faster than defined."

usersUtils.Disconnect()
usersUtils.ConnectSpecificUser("TESTER", "Cdst_56Agk$1234")

print "-------------------------------------------------------------\n"
print "    Testing strong password feature\n"
print "-------------------------------------------------------------\n"

expected_status = "Password characteristics do not comply with JITC requirements"
usersUtils.UpdatePwd("TESTER","Cdst_56Agk$1234","",expected_status)
print "Test empty password...OK"

expected_status = "Password is too short."
usersUtils.UpdatePwd("TESTER","Cdst_56Agk$1234","ABcd12#$1234",expected_status)
print "Test min length...OK"

expected_status = "New password must differ in at least 4 chars from the old password."
usersUtils.UpdatePwd("TESTER","Cdst_56Agk$1234","Cdst_67Bgk$1234",expected_status)
print "Test at least 4 characters were changed...OK"

expected_status = "Password characteristics do not comply with JITC requirements"
usersUtils.UpdatePwd("TESTER","Cdst_56Agk$1234","abcd123_@pwd1234",expected_status)
print "Test at least one 1 characters alphabetic, upper case...OK"

usersUtils.UpdatePwd("TESTER","Cdst_56Agk$1234","ABCD123_@PWD1234",expected_status)
print "Test at least one 1 characters alphabetic, lower case...OK"

usersUtils.UpdatePwd("TESTER","Cdst_56Agk$1234","ABCDefg_@PWDabcd",expected_status)
print "Test at least one numeric character...OK"

usersUtils.UpdatePwd("TESTER","Cdst_56Agk$1234","ABCDefg12PWD1234",expected_status)
print "Test at least one special character...OK"

usersUtils.UpdatePwd("TESTER","Cdst_56Agk$1234","TESTer_@12PWD1234",expected_status)
print "Test Passwords does not contain the associated login...OK"

usersUtils.UpdatePwd("TESTER","Cdst_56Agk$1234","ABCDer_@12PW56789",expected_status)
print "Test Passwords does not contain 4 successive digits...OK"

usersUtils.UpdatePwd("USER","Cdst_56Agk$1234","Just_45Abc$1235")
print "Test Admin changes User Password, less than 4 chars are changed ... OK"

usersUtils.DelUser("USER")


print "-------------------------------------------------------------\n"
print "    Testing password history feature\n"
print "-------------------------------------------------------------\n"

expected_status = "New password was used recently."
usersUtils.UpdatePwd("TESTER","Cdst_56Agk$1234","Cdst_56Agk$1234",expected_status)
print "Test used password...OK"
usersUtils.Sleep(5)

usersUtils.Disconnect()
usersUtils.UpdateUserLastPwdChangedInOperatorDB("TESTER", 1, 3, 40)
usersUtils.Sleep(5)

sysCfgUtils.ConnectSpecificUser("TESTER", "Cdst_56Agk$1234")
print "Setting system.cfg to force strong password mode."
jitc_mode="YES"
force_pwd_policy="YES"
sysCfgUtils.SetFederalMode(jitc_mode, force_pwd_policy) 
usersUtils.ConnectSpecificUser("TESTER", "Cdst_56Agk$1234")

print "\n"
usersUtils.UpdatePwd("TESTER","Cdst_56Agk$1234","Abs005Agk$120_0")


for i in range(15):
    usersUtils.Disconnect()
    usersUtils.UpdateUserLastPwdChangedInOperatorDB("TESTER", i+1, 3, 40)
    
    if(i<9):
        old_pwd = "Abs" + str(i) + str(i) + "5Agk$12" + str(i) + "_" + str(i)
        new_pwd = "Abs" + str(i+1) + str(i+1) + "5Agk$12" + str(i+1) + "_" + str(i+1)
    elif(i==9):
        j=1
        old_pwd = "Abs" + str(i) + str(i) + "5Agk$12" + str(i) + "_" + str(i)
        new_pwd = "Bbs" + str(j) + str(j) + "5Agk$12" + str(j) + "_" + str(j)
    else:
        j=i-9
        old_pwd = "Bbs" + str(j) + str(j) + "5Agk$12" + str(j) + "_" + str(j)
        new_pwd = "Bbs" + str(j+1) + str(j+1) + "5Agk$12" + str(j+1) + "_" + str(j+1)
    
    usersUtils.ConnectSpecificUser("TESTER", old_pwd)
    print "\n-------\n   " + str(i) + "\n-------\n"
    print "old_pwd = " + old_pwd
    print "new_pwd = " + new_pwd
    usersUtils.UpdatePwd("TESTER", old_pwd, new_pwd)
    

usersUtils.Disconnect()
usersUtils.UpdateUserLastPwdChangedInOperatorDB("TESTER", 11, 3, 40)
usersUtils.ConnectSpecificUser("TESTER", "Bbs665Agk$126_6")

expected_status = "New password was used recently."
old_pwd = "Bbs665Agk$126_6"
for i in range(15):
    
    if(i<9):
        new_pwd = "Abs" + str(i+1) + str(i+1) + "5Agk$12" + str(i+1) + "_" + str(i+1)
    else:
        j=i-8   
        new_pwd = "Bbs" + str(j) + str(j) + "5Agk$12" + str(j) + "_" + str(j)
        
    print "\n-------\n   " + str(i) + "\n-------\n"
    print "new_pwd = " + new_pwd
    usersUtils.UpdatePwd("TESTER", old_pwd, new_pwd, expected_status)
    print "Test password was used (log " + str(i) + ") ... OK"
    
print "\n"

usersUtils.AddNewUser("TESTER2","Test_45Abc$1234","operator")

print "-------------------------------------------------------------\n"
print "    Testing other user's password change by Admin\n"
print "-------------------------------------------------------------\n"

expected_status = "Admin password is incorrect"
usersUtils.UpdatePwd("TESTER2","Abs995Agk$129_8","Abs995Agk$129_0", expected_status)
print "Test - wrong admin password - OK"

usersUtils.UpdatePwd("TESTER2","Bbs665Agk$126_6","Abs995Agk$129_0")
print "Test - admin password matches, password changes - OK"

usersUtils.UpdatePwd("TESTER2","Bbs665Agk$126_6","Abs995Agk$129_1")
print "Test - no minimal wait between password changes - OK"

usersUtils.UpdatePwd("TESTER2","Bbs665Agk$126_6","Abs995Agk$129_0")
print "Test - history is not checked password tests - OK"

expected_status = "Password characteristics do not comply with JITC requirements"
usersUtils.UpdatePwd("TESTER2","Bbs665Agk$126_6","",expected_status)
print "Test - strong password - OK"


print "-------------------------------------------------------------\n"
print "    Testing account locked after 3 password change failures \n"
print "-------------------------------------------------------------\n"

usersUtils.Disconnect()

usersUtils.UpdateUserLastPwdChangedInOperatorDB("TESTER2", 1, 3, 40)

usersUtils.ConnectSpecificUserFederalFirstLogin("TESTER2", "Abs995Agk$129_0", "Des997Agk$129_1")
usersUtils.Disconnect()
usersUtils.UpdateUserLastPwdChangedInOperatorDB("TESTER2", 2, 3, 40)
usersUtils.ConnectSpecificUser("TESTER2", "Des997Agk$129_1")

expected_status = "Old password is incorrect"
usersUtils.UpdatePwd("TESTER2","Abs995Agk$129_3","Abs995Agk$129_4",expected_status)
usersUtils.UpdatePwd("TESTER2","Abs995Agk$129_3","Abs995Agk$129_4",expected_status)
usersUtils.UpdatePwd("TESTER2","Des997Agk$129_1","Abs995Agk$129_4")

usersUtils.Disconnect()
usersUtils.ConnectSpecificUser("TESTER2", "Abs995Agk$129_4")
print "Test account is not disabled after 2 failures to change password...OK"

expected_status = "Old password is incorrect"
usersUtils.UpdatePwd("TESTER2","Abs995Agk$129_2","Abs995Agk$129",expected_status)
usersUtils.UpdatePwd("TESTER2","Abs995Agk$129_2","Abs995Agk$129",expected_status)
usersUtils.UpdatePwd("TESTER2","Abs995Agk$129_2","Abs995Agk$129",expected_status)

expected_status = "Account is locked"
usersUtils.UpdatePwd("TESTER2","Abs995Agk$129_4","Des997Agk$129_8", expected_status)
usersUtils.Disconnect()

expected_status = "Account is locked"
usersUtils.ConnectSpecificUser("TESTER2", "Abs995Agk$129_4", expected_status)
print "Test account is disabled after 3 failures to change password ... OK"

usersUtils.UpdateAccountLockedTimeInOperatorDB("TESTER2", 0, 3, 40)

usersUtils.ConnectSpecificUser("TESTER2", "Abs995Agk$129_4")
print "Test account is not locked after lock time is over... OK"
usersUtils.Disconnect()


print "-------------------------------------------------------------\n"
print "    Testing account locked after 3 login failures \n"
print "-------------------------------------------------------------\n"

usersUtils.ConnectSpecificUser("TESTER", "Bbs665Agk$126_6")
usersUtils.Sleep(5)

usersUtils.DelUser("TESTER2")
usersUtils.AddNewUser("TESTER2","Test_45Abc$1234","operator")
usersUtils.Disconnect()

expected_status = ""
usersUtils.ConnectSpecificUser("TESTER2", "Abs995Agk$129_9", expected_status)
usersUtils.ConnectSpecificUser("TESTER2", "Abs995Agk$129_9", expected_status)
usersUtils.ConnectSpecificUser("TESTER2", "Abs995Agk$129_9", expected_status)
usersUtils.Sleep(5)
expected_status = "Account is locked"
usersUtils.ConnectSpecificUser("TESTER2","Cds996Agk$129_2", expected_status)
print "Test account is locked after 3 failures to change password ... OK"

print "-------------------------------------------------------------------------\n"
print "    Testing Admin account locked for one minute after 3 login failures \n"
print "-------------------------------------------------------------------------\n"

expected_status = ""
usersUtils.ConnectSpecificUser("TESTER", "Abs995Agk$129_8", expected_status)
usersUtils.ConnectSpecificUser("TESTER", "Abs995Agk$129_8", expected_status)
usersUtils.ConnectSpecificUser("TESTER", "Abs995Agk$129_8", expected_status)

expected_status = "Account is locked"
usersUtils.ConnectSpecificUser("TESTER","Bbs665Agk$126_6", expected_status)
print "Test Admin account is locked after 3 failures ... OK"

usersUtils.Sleep(5)
usersUtils.ConnectSpecificUser("TESTER","Bbs665Agk$126_6")
print "Test Admin account is enabled after 1 minute ... OK"

usersUtils.Disconnect()
os.system("Scripts/Destroy.sh")
os.system("Scripts/StartupWithFederal.sh")
usersUtils.Sleep(5)
sysCfgUtils.ConnectSpecificUser("TESTER","Bbs665Agk$126_6")
print "Test Admin account is NOT disabled after reset (Testing LoadDB during system init) ... OK"


print "--------------------------------------------------------------------------------------------------\n"
print "    Testing account is not disabled if 3 login failures happen with 1 hour delay between failures \n"
print "--------------------------------------------------------------------------------------------------\n"

usersUtils.ConnectSpecificUser("TESTER","Bbs665Agk$126_6")
usersUtils.DelUser("TESTER2")
usersUtils.AddNewUser("TESTER2","Test_45Abc$1234","operator")
usersUtils.Disconnect()

usersUtils.ConnectSpecificUserFederalFirstLogin("TESTER2", "Test_45Abc$1234", "Abs995Agk$129_9")
usersUtils.Sleep(5)
usersUtils.Disconnect()

print "Two consecutive failures:"
expected_status = ""
usersUtils.ConnectSpecificUser("TESTER2", "Abs995Agk$129_8", expected_status)
usersUtils.ConnectSpecificUser("TESTER2", "Abs995Agk$129_8", expected_status)

print "1 hour delay for the last failure:"
usersUtils.UpdateUserFirstFailureTimeInOperatorDB("TESTER2", 0, 1, 1)

usersUtils.ConnectSpecificUser("TESTER2", "Abs995Agk$129_8", expected_status)

usersUtils.ConnectSpecificUser("TESTER2","Abs995Agk$129_9")
print "Test account is NOT locked after 3 failures (with 1 hour between) ... OK"

usersUtils.Disconnect()

print "-------------------\n"
print "    Tests done.    \n"
print "-------------------\n"

sysCfgUtilsDebug = SyscfgUtilities()
sysCfgUtils.ConnectSpecificUser("TESTER", "Bbs665Agk$126_6")    

print "Setting system.cfg to regular mode."
jitc_mode="NO"
force_pwd_policy="YES"
sysCfgUtils.SetFederalMode(jitc_mode, force_pwd_policy)    

sysCfgUtils.ConnectSpecificUser("POLYCOM", "POLYCOM")

jitc_mode="NO"
force_pwd_policy="NO"
sysCfgUtils.SetFederalMode(jitc_mode, force_pwd_policy)  

print "\nTest was completed successfully !!! \n" 


