#!/usr/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

from McmsConnection import *
from string import *
from ConfPkgUtil import *



c = McmsConnection()
c.Connect()

print " -> Add new Subscriber"
confname = "conf1"
userName = "user1"
confFile = "Scripts/ConfPkgEvents/AddVideoCpConf.xml"
numRetries = 3
numOfParties = 0

CreateConfWithInfo(c,confname, confFile)
#c.CreateConf(confname, confFile)
confId = c.WaitConfCreated(confname,numRetries)
sleep(1)
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
print notify.toprettyxml()
CheckNotifyValidity(notify, confname+"@plcm.com", "full", numOfParties, version)
CheckConfContactInfo(notify,confname+"@plcm.com")
#*************************************************************
    
print " -> Add a 323 user"
partyname = "Party1" 
partyip =  "1.2.3.1"
partyFile = "Scripts/ConfPkgEvents/AddDialOutH323Party.xml"

AddPartyWithInfo(c,confId, partyname, partyip, partyFile)
#c.AddParty(confId, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutH323Party.xml")
numOfParties = 1
sleep(5)

print " -> Get notify"
version+=1
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER",userName+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)
CheckPartyType(notify, 0, 'h323', 'video', 'full', partyname+"@"+partyip, 'connected')
CheckActionsValidity(notify ,0 , "connection-changed", 0)
CheckActionsValidity(notify ,0, "added", 1)

#*************************************************************
print " -> Change Audio Speaker : party1"
partyId = int(c.GetPartyId(confId, "Party1"))
c.ChangeDialOutAudioSpeaker(confId, partyId)  
numOfParties = 0
sleep(5)

print " -> Get notify"
version+=1
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER",userName+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)
CheckActiveSpeaker(notify, "sip:Party1@1.2.3.1;user=h323", partyname+"@"+partyip)

#*************************************************************

print " -> Add new Subscriber"
confname2 = "conf2"
userName2 = "user2"
confFile = "Scripts/ConfPkgEvents/AddVideoCpConf.xml"

numRetries = 3
numOfParties = 0
CreateConfWithInfo(c,confname2, confFile)
#c.CreateConf(confname2, confFile)
confId2 = c.WaitConfCreated(confname2,numRetries)
sleep(1)
print " -> Add subscriber"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME",confname2+"@plcm.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER","user2"+"@domain.com")
c.Send()

sleep(3)

print " -> Get notify"
version2 = 1
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname2+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER","user2"+"@domain.com")
c.Send()

notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
CheckNotifyValidity(notify, confname2+"@plcm.com", "full", numOfParties, version2)
CheckConfContactInfo(notify,confname2+"@plcm.com")
#*************************************************************
print " -> Mute party1"
mutedParty = 0;
print
print "Mute Audio by Direction (receive) in Conf ..."
MutePartyVideoAudioTest(c,confId, mutedParty)
sleep(4)
numOfParties = 0
print " -> Get notify"
version+=1
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER",userName+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)
CheckMuteViaFocus(notify, "sip:Party1@1.2.3.1;user=h323")
CheckActionsValidity(notify ,0, "mute-changed", 0)
#*************************************************************
    
print " -> Add a 323 user"
partyname = "Party2" 
partyip =  "1.2.3.2"
partyFile = "Scripts/ConfPkgEvents/AddDialOutH323Party.xml"

AddPartyWithInfo(c,confId2, partyname, partyip, partyFile)
#c.AddParty(confId2, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutH323Party.xml")
numOfParties = 0
sleep(5)

print " -> Get notify"
version2+=2
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname2+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER","user2"+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
CheckNotifyValidity(notify, confname2+"@plcm.com", "partial", numOfParties, version2)
CheckPartyType(notify, 0, 'h323', 'video', 'partial', partyname+"@"+partyip,'connected')
CheckActionsValidity(notify ,0 , "connection-changed", 0)
CheckActionsValidity(notify ,0, "added", 1)

#*************************************************************
print " -> Add new Subscriber"
#confname = "conf1"
userName = "user3"
confFile = "Scripts/ConfPkgEvents/AddVideoCpConf.xml"
numRetries = 3
numOfParties = 0

#c.CreateConf(confname, confFile)
#confId = c.WaitConfCreated(confname,numRetries)
sleep(1)
print " -> Add subscriber"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER","user3"+"@domain.com")
c.Send()

sleep(5)

print " -> Get notify"
#version += 1
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER","user3"+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
CheckNotifyValidity(notify, confname+"@plcm.com", "full", numOfParties, version)
CheckConfContactInfo(notify,confname +"@plcm.com")
sleep(3)
#*************************************************************
    
print " -> Add a 323 user"
partyname = "Party3" 
partyip =  "1.2.3.3"
partyFile = "Scripts/ConfPkgEvents/AddDialOutH323Party.xml"

AddPartyWithInfo(c,confId, partyname, partyip, partyFile)

#c.AddParty(confId, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutH323Party.xml")
numOfParties = 1
sleep(5)

print " -> Get notify for user1"
version+=2
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER","user1"+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
#CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)
CheckPartyType(notify, 0, 'h323', 'video', 'partial', partyname+"@"+partyip,'connected')
CheckActionsValidity(notify ,0 , "connection-changed", 0)
#CheckActionsValidity(notify ,0, "added", 1)

print " -> Get notify for user3"
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER","user3"+"@domain.com")
c.Send()

notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
#CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)

CheckPartyType(notify, 0, 'h323', 'video', 'partial',partyname+"@"+partyip, 'connected')
CheckActionsValidity(notify ,0 , "connection-changed", 0)

#CheckActionsValidity(notify ,0, "added", 1)

#*************************************************************
'''
print " -> Mute party2"
mutedParty = 1;
print
print "Mute Audio by Direction (receive) in Conf ..."
MutePartyVideoAudioTest(c,confId, mutedParty)
'''
#*************************************************************

print " ->UNMute party1"
#unmutedParty = 1;
#WaitAllButSelfSeesPartyInLayout(c, confid, mutedParty)
print
print "UnMute Audio by Direction (receive) in Conf ..."
UnMutePartyVideoAudioTest(c,confId, mutedParty)
sleep(5)

print " -> Get notify for user1"
version+=1
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER","user1"+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)
CheckUnMuteViaFocus(notify, "sip:Party1@1.2.3.1;user=h323")
CheckActionsValidity(notify ,0, "mute-changed", 0)

print " -> Get notify for user3"
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER","user3"+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)
CheckUnMuteViaFocus(notify, "sip:Party1@1.2.3.1;user=h323")
CheckActionsValidity(notify ,0, "mute-changed", 0)

#*************************************************************
  


sleep(1)
print " -> Remove subscriber 1"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER","user3"+"@domain.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","EXPIRES","0")
c.Send()
sleep(3)

sleep(1)
print " -> Remove subscriber 2"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER","user1"+"@domain.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","EXPIRES","0")
c.Send()
sleep(3)


c.DeleteConf(confId)

c.Disconnect()
