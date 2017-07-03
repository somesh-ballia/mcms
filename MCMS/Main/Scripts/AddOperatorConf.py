#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

from McmsConnection import *

c = McmsConnection()
c.Connect()


profileName = "operator_conf_profile_01"
print "step 01: Adding profile with OPERATOR_CONF = YES, profile_name = " + profileName
c.LoadXmlFile('Scripts/CheckUnicodeNames/AddProfile.xml')
c.ModifyXml("RESERVATION","DISPLAY_NAME", profileName)
c.ModifyXml("RESERVATION","OPERATOR_CONF","true")
c.Send()
ProfId = c.GetTextUnder("RESERVATION","ID")
print "Profile, named: " + profileName + " ,ID = " + ProfId + ", is added"
sleep(10)

#create the target Conf and wait untill it connected

## connection.CreateConfFromProfile(confDisplayName, ProfId, 'Scripts/EncryConf/CreateNewConf.xml')
## confId  = connection.WaitConfCreated(confDisplayName,num_retries)
## sleep(2)
   
c.Disconnect()


