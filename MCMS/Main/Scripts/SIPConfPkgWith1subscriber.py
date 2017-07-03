#!/usr/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

from McmsConnection import *

c = McmsConnection()
c.Connect()


print " -> Start"
print " -> Sleep 10"
sleep(10)

print " -> Add subsc AAA+Vasily (good)"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME","ron1")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER","Vasily")
c.Send()

print " -> Sleep 10"
sleep(10)

print " -> Add subsc AAA+Ori (good)"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME","ron1")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER","Ori")
c.Send()

print " -> Sleep 10"
sleep(10)

print " -> Add subsc AAA+Ori (bad)"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME","ron1")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER","Ori")
c.Send()

print " -> Sleep 10"
sleep(10)

print " -> Remove subsc AAA+Vasily (good)"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME","ron1")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER","Vasily")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","EXPIRES","0")
c.Send()

print " -> Sleep 10"
sleep(10)

print " -> Get notif AAA+Ori (good)"
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME","ron1")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER","Ori")
c.Send()

print " -> Sleep 10"
sleep(10)

print " -> Get notif AAA+Amir (bad)"
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME","ron1")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER","Amir")
c.Send()

c.Disconnect()

