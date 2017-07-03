#!/usr/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

from McmsConnection import *
from string import *
from ConfPkgUtil import *



c = McmsConnection()
c.Connect()

confname = "ron1"
userName = "user1"
confFile = "Scripts/ConfPkgEvents/AddVideoCpConf.xml"
#confFile = "Scripts/AddVideoCpConf.xml"
numRetries = 20
numOfParties = 32

CreateConfWithInfo(c,confname, confFile)
#c.CreateConf(confname, confFile)
confId = c.WaitConfCreated(confname,numRetries)
sleep(1)

#*************************************************************
partyFile = "Scripts/ConfPkgEvents/AddDialOutH323Party.xml"
print " Add 32 Parties"
for x in range(numOfParties):
    partyname = "Party"+str(x+1)
    partyip =  "1.2.3." + str(x+1)
    AddPartyWithInfo(c,confId, partyname, partyip, partyFile)

c.WaitAllPartiesWereAdded(confId,numOfParties,numRetries)

sleep(400)
print " -> Add subscriber"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER",userName+"@domain.com")
c.Send()

sleep(3)

print " -> Get notify"
version = 1
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER",userName+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)

print " -> Remove subscriber"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER",userName+"@domain.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","EXPIRES","0")
c.Send()
sleep(4)

c.DeleteConf(confId)

c.Disconnect()


  