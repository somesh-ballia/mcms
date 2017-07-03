#!/usr/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

from McmsConnection import *
from string import *
from ConfPkgUtil import *



c = McmsConnection()
c.Connect()

confname = "conf1"
userName = "user1"
confFile = "Scripts/ConfPkgEvents/AddVideoCpConf.xml"
#confFile = "Scripts/AddVideoCpConf.xml"
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
CheckNotifyValidity(notify, confname+"@plcm.com", "full", numOfParties, version)
CheckConfContactInfo(notify,confname+"@plcm.com")
#*************************************************************
    
print " -> Add a 323 user"
partyname = "Party1" 
partyip =  "1.2.3.4"
partyFile = "Scripts/ConfPkgEvents/AddDialOutH323Party.xml"

#c.AddParty(confId, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutH323Party.xml")
AddPartyWithInfo(c,confId, partyname, partyip, partyFile)

numOfParties = 1
sleep(6)

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
CheckPartyType(notify, 0, 'h323', 'video', 'full', partyname+"@"+partyip,  'connected')
CheckActionsValidity(notify ,0 , "connection-changed", 0)
CheckActionsValidity(notify ,0, "added", 1)
sleep(2)
#*************************************************************
    
print " -> Add 2 sip users"
partyname = "Party2" 
partyip =  "172.22.1.2"
partyFile2 = "Scripts/ConfPkgEvents/AddDialOutSipParty.xml"
AddSIPPartyWithInfo(c,confId, partyname, partyip, partyname+"@"+partyip, partyFile2)
#c.AddSIPParty(confId, partyname, partyip, partyname+"@"+partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutSipParty.xml")

partyname = "Party3" 
partyip =  "172.22.1.3"
AddSIPPartyWithInfo(c,confId, partyname, partyip, partyname+"@"+partyip, partyFile2)
#c.AddSIPParty(confId, partyname, partyip, partyname+"@"+partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutSipParty.xml")
numOfParties = 2
sleep(5)
print " -> Get notify"
version+=2
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER",userName+"@domain.com")
c.Send()
  
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)
CheckPartyType(notify, 0, 'sip', 'video', 'partial' ,"Party2@172.22.1.2" , 'connected' )
CheckPartyType(notify, 1, 'sip', 'video', 'full' ,"Party3@172.22.1.3" , 'connected' )
CheckActionsValidity(notify ,0, "connection-changed", 0)
#CheckActionsValidity(notify ,0, "added", 1)
CheckActionsValidity(notify ,1, "connection-changed", 0)
#CheckActionsValidity(notify ,1, "added", 1)

#*************************************************************
print " -> Mute party2"
mutedParty = 1;
print
print "Mute Audio by Direction (receive) in Conf ..."
MutePartyVideoAudioTest(c,confId, mutedParty)
#*************************************************************
print " -> Mute party1"
mutedParty = 0;
print
print "Mute Audio by Direction (receive) in Conf ..."
MutePartyVideoAudioTest(c,confId, mutedParty)
#*************************************************************
print " -> Change Audio Speaker : party3"
partyId = int(c.GetPartyId(confId, "Party3"))
c.ChangeDialOutAudioSpeaker(confId, partyId)  
sleep(5)

numOfParties = 2
print " -> Get notify"
version+=1
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER",userName+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
#print notify.toprettyxml()

CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)
CheckActiveSpeaker(notify, "sip:Party3@172.22.1.3","Party3@172.22.1.3")
CheckMuteViaFocus(notify, "sip:Party2@172.22.1.2")
CheckMuteViaFocus(notify, "sip:Party1@1.2.3.4;user=h323")
CheckActionsValidity(notify ,0, "mute-changed", 0)
CheckActionsValidity(notify ,1, "mute-changed", 0)

'''
print " -> Remove subscriber"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER",userName+"@domain.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","EXPIRES","0")
c.Send()
sleep(4)
'''
c.DeleteConf(confId)

c.Disconnect()


  