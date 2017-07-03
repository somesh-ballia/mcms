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
sleep(5)
#*************************************************************
    
print " -> Add a 323 user"
partyname = "Party1" 
partyip =  "1.2.3.4"
partyFile = "Scripts/ConfPkgEvents/AddDialOutH323Party.xml"

AddPartyWithInfo(c,confId, partyname, partyip, partyFile)
numOfParties = 1
sleep(6)

print " -> Get notify"
version+=2
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER",userName+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
#CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)

CheckPartyType(notify, 0, 'h323', 'video', 'partial' , "Party1@1.2.3.4" , 'connected' )
CheckActionsValidity(notify ,0 , "connection-changed", 0)

#CheckActionsValidity(notify ,0, "added", 1)
sleep(5)
#*************************************************************
    
print " -> Add 2 sip users"
partyname = "Party2" 
partyip =  "172.22.1.2"
partyFile2 = "Scripts/ConfPkgEvents/AddDialOutSipParty.xml"

AddSIPPartyWithInfo(c,confId, partyname, partyip, partyname+"@"+partyip, partyFile2)
#c.AddSIPParty(confId, partyname, partyip, partyname+"@"+partyip, partyFile2)
partyname = "Party3" 
partyip =  "172.22.1.3"
#partyFile3 = "Scripts/ConfPkgEvents/AddDialOutSipParty.xml"
AddSIPPartyWithInfo(c,confId, partyname, partyip, partyname+"@"+partyip, partyFile2)
#c.AddSIPParty(confId, partyname, partyip, partyname+"@"+partyip, partyFile3)
numOfParties = 2
sleep(6)

print " -> Get notify"
version+=2
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER",userName+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
#CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)


CheckPartyType(notify, 0, 'sip', 'video', 'partial' ,"Party2@172.22.1.2" , 'connected' )
CheckPartyType(notify, 1, 'sip', 'video', 'full' ,"Party3@172.22.1.3" , 'connected' )


CheckActionsValidity(notify ,0, "connection-changed", 0)
#CheckActionsValidity(notify ,0, "added", 1)

CheckActionsValidity(notify ,1, "connection-changed", 0)
#CheckActionsValidity(notify ,1, "added", 1)
sleep(5);
#*************************************************************
print " -> Mute party2"
mutedParty = 1;
#WaitAllButSelfSeesPartyInLayout(c, confid, mutedParty)
print
print "Mute Audio by Direction (receive) in Conf ..."
#SipMutePartyAudioDirectionTest(c, confId, mutedParty, "receive")
#MutePartyVideoTest(c,confId, mutedParty)
MutePartyVideoAudioTest(c,confId, mutedParty)
#MutePartyAudioTest(c,confId, mutedParty)
#MuteParty(c,"Party2")
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
CheckMuteViaFocus(notify, "sip:Party2@172.22.1.2")
CheckActionsValidity(notify ,0, "mute-changed", 0)

#*************************************************************

print " ->UNMute party2"
#unmutedParty = 1;
#WaitAllButSelfSeesPartyInLayout(c, confid, mutedParty)
print
print "UnMute Audio by Direction (receive) in Conf ..."
UnMutePartyVideoAudioTest(c,confId, mutedParty)
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
CheckUnMuteViaFocus(notify, "sip:Party2@172.22.1.2")
CheckActionsValidity(notify ,0, "mute-changed", 0)
  
  
#*****************************************************************

print " -> Change Audio Speaker : party3"
partyId = int(c.GetPartyId(confId, "Party3"))
c.ChangeDialOutAudioSpeaker(confId, partyId)  

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
CheckActiveSpeaker(notify, "sip:Party3@172.22.1.3" , "Party3@172.22.1.3")

#*****************************************************************
print " -> Disconnect party3"
c.DisconnectParty(confId,2)
c.WaitPartyDisConnected(confId,2,3)
#sleep(6)

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
CheckPartyType(notify, 0, 'sip', 'video', 'partial',"Party3@172.22.1.3", 'disconnecting')
CheckActionsValidity(notify ,0, "connection-changed", 0)
sleep(5)

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
CheckPartyType(notify, 0, 'sip', 'video', 'partial', "Party3@172.22.1.3", 'disconnected')
CheckActionsValidity(notify ,0, "connection-changed", 0)
sleep(3)
#*****************************************************************

print "Deleting Party2"
partyId = c.GetPartyId(confId , "Party2")
c.DeleteParty(confId, partyId)
sleep(6)
numOfParties = 1

print " -> Get notify"
version+=2
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER",userName+"@domain.com")
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
print notify.toprettyxml()
#CheckNotifyValidity(notify, confname+"@plcm.com", "partial", numOfParties, version)
#CheckPartyType(notify, 0, 'sip', 'video', 'deleted','Party2@172.22.1.2')
#CheckActionsValidity(notify ,0, "connection-changed", 0)
#CheckActionsValidity(notify ,0, "deleted", 1)

#CheckPartyType(notify, 0, 'sip', 'video', 'partial', 'disconnecting')


'''
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
CheckPartyType(notify, 0, 'sip', 'video', 'deleted')
sleep(6)
'''

#*****************************************************************

sleep(1)
print " -> Remove subscriber"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER",userName+"@domain.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","EXPIRES","0")
c.Send()
sleep(5)

c.DeleteConf(confId)

c.Disconnect()

