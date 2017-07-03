#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

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

print "Try to open ssh"
usersUtils.SendXmlFile("Scripts/TurnSsh.xml", "You are not authorized to perform this operation")

print "Try to restore defaults"
usersUtils.SendXmlFile("Scripts/BackToDefault.xml", "You are not authorized to perform this operation")

print "remove SUPPORT user..."
usersUtils.LoadXmlFile("Scripts/RemoveNewOperator.xml")
usersUtils.ModifyXml("DELETE_OPERATOR", "USER_NAME", "SUPPORT")
usersUtils.Send()

print "Adding new SUPPORT Operator..."

usersUtils.LoadXmlFile("Scripts/AddNewOperator.xml")
usersUtils.ModifyXml("OPERATOR", "USER_NAME", "SUPPORT")
usersUtils.ModifyXml("OPERATOR", "AUTHORIZATION_GROUP", "operator")
usersUtils.Send("STATUS_USER_NAME_IS_NOT_ALLOWED")
