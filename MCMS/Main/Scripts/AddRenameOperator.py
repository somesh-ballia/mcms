#!/mcms/python/bin/python

# this is an example for list of processes that will be running under valgrind
#*PROCESSES_FOR_VALGRIND='Authentication McuMngr MplApi GideonSim QAAPI Faults Logger Configurator'
# this is an example for profile of processes that will be running under valgrind
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_2

from McmsConnection import *
from SysCfgUtils import *
from UsersUtils import *
import os

os.environ["CLEAN_CFG"]="YES"
os.system("Scripts/StartupWithFederal.sh")
os.environ["CLEAN_CFG"]="NO"

usersUtils = UsersUtilities()
usersUtils.Sleep(5)
usersUtils.ConnectSpecificUserFederalFirstLogin("POLYCOM", "POLYCOM", "Abst_45Agk$1234")

print "Adding new Operator... operator group"
usersUtils.LoadXmlFile("Scripts/AddNewOperator.xml")
usersUtils.ModifyXml("OPERATOR", "PASSWORD", "Test_45Abc#12345")
usersUtils.ModifyXml("OPERATOR", "AUTHORIZATION_GROUP", "operator")
usersUtils.Send()

print "Change operator password"
usersUtils.UpdatePwd("NewOperator","Test_45Abc#12345","123AbcD_#p12345")


print "Rename the operator to SUPPORT user..."
usersUtils.LoadXmlFile("Scripts/RenameNewOperator.xml")
usersUtils.ModifyXml("OPERATOR", "NEW_USER_NAME", "SUPPORT")
usersUtils.Send("STATUS_USER_NAME_IS_NOT_ALLOWED")

print "Rename the operator to POLYCOM user that already exist..."
usersUtils.LoadXmlFile("Scripts/RenameNewOperator.xml")
usersUtils.ModifyXml("OPERATOR", "NEW_USER_NAME", "POLYCOM")
usersUtils.Send("A user with this login name already exists")

print "Rename the operator to valid name..."
usersUtils.SendXmlFile("Scripts/RenameNewOperator.xml")

print "Try to rename user that does not exist..."
usersUtils.SendXmlFile("Scripts/RenameNewOperator.xml", "Login name does not exist")

print "Try to login with the new operator login name - to get new account response..."
usersUtils.LoadXmlFile("Scripts/login.xml")
usersUtils.ModifyXml("LOGIN", "USER_NAME", "Judith")
usersUtils.ModifyXml("LOGIN", "PASSWORD", "123AbcD_#p12345")
usersUtils.Send("User must change password")

print "Change operator password to a good password"
usersUtils.LoadXmlFile("Scripts/FederalLogin.xml")
usersUtils.ModifyXml("LOGIN", "USER_NAME", "Judith")
usersUtils.ModifyXml("LOGIN", "PASSWORD", "123AbcD_#p12345")
usersUtils.ModifyXml("LOGIN", "NEW_PASSWORD", "Abst_45Agk$1234")
usersUtils.Send()

print "Change operator password to the original one - that was used recently"
usersUtils.ConnectSpecificUser("Judith", "Abst_45Agk$1234")
usersUtils.UpdatePwd("Judith","Abst_45Agk$1234","Test_45Abc#12345","New password was used recently.")
usersUtils.Disconnect()
