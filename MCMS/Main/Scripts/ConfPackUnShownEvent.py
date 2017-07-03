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
#confFile = "Scripts/AddVideoCpConf.xml"
confFile = "Scripts/ConfPkgEvents/AddVideoCpConf.xml"
numRetries = 3
numOfParties = 0

#c.CreateConf(confname, confFile)
CreateConfWithInfo(c,confname, confFile)
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
partyip =  "1.2.3.4"
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
    
print " -> Add a sip Audio only user"
partyname = "Party2" 
partyip =  "172.22.1.2"
#partyFile2 = "Scripts/ConfPkgEvents/AddDialOutSipParty.xml"
partyFile2 = "Scripts/AddAudioOnlySipParty.xml"
AddSIPPartyWithInfo(c,confId, partyname, partyip, partyname+"@"+partyip, partyFile2)

#c.AddSIPParty(confId, partyname, partyip, partyname+"@"+partyip, "Scripts/AddAudioOnlySipParty.xml")
numOfParties = 1
#partyId = int(c.GetPartyId(confId, "Party2"))
c.WaitAllOngoingConnected(confId,3)
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
CheckPartyType(notify, 0, 'sip', 'audio', 'full',partyname+"@"+partyip, 'connected')
CheckActionsValidity(notify ,0, "connection-changed", 0)
CheckActionsValidity(notify ,0, "added", 1)

#*************************************************************
print " -> Add Dial In SIP Party "


partyname = "Party3"
partyip =  "172.22.1.3" 
partySipAdd = partyname + '@' + partyip
AddSIPPartyWithInfo(c,confId, partyname, partyip, partyname+"@"+partyip, partyFile2)
#c.AddSIPParty(confId, partyname, partyip, partySipAdd, "Scripts/CheckMultiTypesOfCalls/AddDialInSipParty.xml")

#*************************************************************
print "Deleting Party3"
partyId = c.GetPartyId(confId , "Party3")
c.DeleteParty(confId, partyId)
#sleep(5)
#numOfParties = 3

print " -> Get notify"
#version+=1
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER",userName+"@domain.com")
c.Send()
 
#notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
#notify = parseString(notifyContent)
#print notify.toprettyxml()


#*************************************************************
print " -> Mute party2"
mutedParty = 1;
print
print "Mute Audio by Direction (receive) in Conf ..."
MutePartyVideoAudioTest(c,confId, mutedParty)
sleep(2)

print " -> Get notify"
version+=1
numOfParties = 0
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER",userName+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()

CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)
CheckMuteViaFocus(notify, "sip:Party2@172.22.1.2")
CheckActionsValidity(notify ,0, "mute-changed", 0)
sleep(4)
#*****************************************************************
print " -> Disconnect party2"
c.DisconnectParty(confId,1)
c.WaitPartyDisConnected(confId,2,3)

#*****************************************************************
print " -> Connect party2"

c.LoadXmlFile('Scripts/DisconnectIPParty.xml')
c.ModifyXml("SET_CONNECT", "ID",confId)
c.ModifyXml("SET_CONNECT", "PARTY_ID", 1)
c.ModifyXml("SET_CONNECT", "CONNECT" ,"true")
c.Send()
c.WaitAllOngoingConnected(confId,3)

#numOfParties = 3
#sleep(1)
sleep(4)
print " -> Get notify"
version+=1
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER",userName+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()

#CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)
#CheckPartyType(notify, 0, 'h323', 'video', 'partial', 'muted-via-focus')
#CheckPartyType(notify, 1, 'sip', 'video', 'partial', 'muted-via-focus')
#CheckPartyType(notify, 2, 'sip', 'video', 'partial', 'disconnected')

#*****************************************************************
'''
print "Deleting Party2"
partyId = c.GetPartyId(confId , "Party2")
c.DeleteParty(confId, partyId)
sleep(5)
numOfParties = 1

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
CheckPartyType(notify, 0, 'sip', 'video','deleted', "Party2@172.22.1.2" )
CheckActionsValidity(notify ,0, "connection-changed", 0)
CheckActionsValidity(notify ,0, "deleted", 1)
'''
sleep(1)
print " -> Remove subscriber"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER",userName+"@domain.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","EXPIRES","0")
c.Send()
sleep(1)
c.DeleteConf(confId)
sleep(3)


c.Disconnect()


  