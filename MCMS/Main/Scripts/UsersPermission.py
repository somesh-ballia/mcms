#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6
import os
import sys
import shutil

from UsersUtils import *

r_in_list=0
r = UsersUtilities()
p = UsersUtilities()
r.Connect()

r.AddNewUser("USER1","USER1_pwd","operator")
p.ConnectSpecificUser("USER1","USER1_pwd")


# the following test is removed since update profile is ALLOWED for Operator (MCMS-Carmel_1.1.0.0193)
"""
#~~~~~~~~~~~~~~~~~~~~~Change profile definition ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
print "Test operator user permission to Change profile definition "
r.SendXmlFile("Scripts/Addprofile.xml")
p.SendXmlFile("Scripts/UpdateProfile.xml","You are not authorized to perform this operation")
"""

#~~~~~~~~~~~~~~~~~~~~~CDR ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
print "Test operator user permission to retrive CDR "

r.CreateConf("forCdr")
confid = r.WaitConfCreated("forCdr",1)
r.DeleteConf(confid)
sleep (2)

p.Disconnect()
p.ConnectSpecificUser("USER1","USER1_pwd")

p.LoadXmlFile('Scripts/RetriveFormatedCDR.xml') 
p.ModifyXml("TRANS_CDR_FULL","ID",confid)
p.Send("You are not authorized to perform this operation")
#r.SendXmlFile("Scripts/RetriveFormatedCDR.xml","You are not authorized to perform this operation")

#~~~~~~~~~~~~~~~~~~~~~Change System.cfg~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
print "Test operator user permission to Change System.cfg "

print "Send Set in section GATE_KEEPER: CM_KEEP_ALIVE_RECEIVE_PERIOD=3 (seconds) in system.cfg"
p.LoadXmlFile('Scripts/SetSystemCfg.xml') 

p.ModifyXml("SET_CFG","NAME","GATE_KEEPER")
p.ModifyXml("SET_CFG","KEY","CM_KEEP_ALIVE_RECEIVE_PERIOD")
p.ModifyXml("SET_CFG","DATA",3)
#p.Send("You are not authorized to perform this operation")
p.Send("The new flag cannot be added. Flag is not recognized by the system")

#~~~~~~~~~~~~~~~~~~~~~change System Time ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
print "Test operator user permission to Change System Time "

r.SendXmlFile("Scripts/ChangeTime.xml")

# the following test is removed since update profile is ALLOWED for Operator (MCMS-Carmel_1.1.0.0193)
"""
p.SendXmlFile("Scripts/UpdateProfile.xml","You are not authorized to perform this operation")
"""

#~~~~~~~~~~~~~~~~~~~~~Update_Ivr ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
print "Test operator user permission to Change Update_Ivr "

p.SendXmlFile("Scripts/Update_Ivr.xml","You are not authorized to perform this operation")

#~~~~~~~~~~~~~~~~~~~~~UpdateManagementService~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
print "Test operator user permission to Update Management Service "

p.SendXmlFile("Scripts/UpdateNetworkService.xml","You are not authorized to perform this operation")

#~~~~~~~~~~~~~~~~~~~~~UpdateIPService~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
print "Test operator user permission to Update IP Service "

r.SendXmlFile("Scripts/UpdateIPService.xml")
p.SendXmlFile("Scripts/UpdateIPService.xml","You are not authorized to perform this operation")

#~~~~~~~~~~~~~~~~~~~~~DelIPService~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
print "Test operator user permission to Delete IP Service "

r.SendXmlFile("Scripts/Del_IPService.xml")
p.SendXmlFile("Scripts/Del_IPService.xml","You are not authorized to perform this operation")

os.environ["USE_DEFAULT_IP_SERVICE"]="NO"
os.system("Scripts/Startup.sh")


#~~~~~~~~~~~~~~~~~~~~~AddIPService~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
print "Test operator user permission to Add IP Service "
r.Connect()
r.AddNewUser("USER1","USER1_pwd","operator")
p.ConnectSpecificUser("USER1","USER1_pwd")
p.SendXmlFile("Scripts/Add_ip_service.xml","You are not authorized to perform this operation")
#p.SendXmlFile("Scripts/Add_ip_service.xml","The new flag cannot be added. Flag is not recognized by the system")

p.Disconnect()
r.Disconnect()



