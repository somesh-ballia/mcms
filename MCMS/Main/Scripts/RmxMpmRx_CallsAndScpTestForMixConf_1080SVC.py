#!/usr/bin/python

#-- SKIP_ASSERTS
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_MPMRX.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmRx.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_200HD_v100.0.cfs"


from McmsConnection import *
from string import *
from ConfPkgUtil import *
from pprint import pprint
from SysCfgUtils import *


util = SyscfgUtilities()
util.Connect()
util.SendSetCfgParam("ENABLE_HIGH_VIDEO_RES_AVC_TO_SVC_IN_MIXED_MODE", "YES")
util.SendSetCfgParam("ENABLE_HIGH_VIDEO_RES_SVC_TO_AVC_IN_MIXED_MODE", "YES")
util.Disconnect()

c = McmsConnection()
c.Connect()

#add a new profile
profId = c.AddProfile("MIX_PROFILE", "Scripts/ConfMediaType/NewMixAvcSvcProfile.xml")

# Creating conference
confname = "conf1"
c.CreateConfFromProfile(confname, profId)
confid = c.WaitConfCreated(confname)

c.LoadXmlFile('Scripts/TransConf2.xml')
c.ModifyXml("GET","ID",confid)
c.Send()

confType = c.GetTextUnder("RESERVATION","CONF_MEDIA_TYPE")
print
print "New CONF MEDIA TYPE reservation is: " + confType


#---------------------------------------
num_retries = 20
num_party = 3

#--- Defined SVC EP Dial In---
partyname = "Party"+str(1)
partyip =  "123.123.0." + str(1)
partySipAdd = partyname + '@' + partyip
#c.AddSIPParty(confid, partyname, partyip, partySipAdd, "Scripts/CheckMultiTypesOfCalls/AddDialInSipParty.xml")
print "Adding SIP Party..." + partyname
c.LoadXmlFile("Scripts/CheckMultiTypesOfCalls/AddDialInSipParty.xml")
c.ModifyXml("PARTY", "NAME", partyname)
c.ModifyXml3("PARTY", "ALIAS", "NAME", "")
c.ModifyXml3("PARTY", "ALIAS", "ALIAS_TYPE", "none")
c.ModifyXml("PARTY", "IP", partyip)
c.ModifyXml("ADD_PARTY", "ID", confid)
c.ModifyXml("PARTY", "SIP_ADDRESS", partySipAdd)
c.ModifyXml("PARTY", "ENDPOINT_MEDIA_TYPE", "media_relay")
c.Send()   

sleep(2)

# Add Sip party to EP Sim and connect him
partyname = "Party"+str(1)
c.SimulationAddSipParty(partyname, confname, "FULL CAPSET SVC")
c.SimulationConnectSipParty(partyname)

#Add the 2nd party
partyname = "Party"+str(2)
c.SimulationAddSipParty(partyname, confname, "FULL CAPSET SVC")
c.SimulationConnectSipParty(partyname)

#Add the 3nd party
partyname = "Party"+str(3)
c.SimulationAddSipParty(partyname, confname)
c.SimulationConnectSipParty(partyname)

sleep(2)
  
c.WaitAllPartiesWereAdded(confid,num_party,num_retries)
c.WaitAllOngoingConnected(confid,num_retries)

# Check if all parties were added and save their IDs
party_id_list = []
c.LoadXmlFile('Scripts/TransConf2.xml')
c.ModifyXml("GET","ID",confid)
c.Send()
ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
if len(ongoing_party_list) > num_party:
    sys.exit("more parties than should be...")
for index in range(num_party):    
    party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)
  


sleep(3)
print " -> Add subscriber"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME",confname+"@plcm.com")#conf1@10.227.2.118
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER",partySipAdd)
c.Send()

sleep(3)

print " -> Get notify"
version = 1
c.LoadXmlFile('Scripts/SimGetSipNotification.xml')
c.ModifyXml("SIP_GET_NOTIFICATION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_GET_NOTIFICATION","SUBSCRIBER",partySipAdd)
c.Send()
 
notifyContent = c.GetTextUnder("SIP_GET_NOTIFICATION","SUBSCR_NOTIFICATION")
notify = parseString(notifyContent)
#print notify.toprettyxml()

CheckUsersValidity(notify, num_party)

userVideoMap = GetUserVideoStreamsMap(notify)
pprint(userVideoMap)

sleep(5)
#*************************************************************
print " -> scp request for Party1: request the first stream of the other parties"
c.LoadXmlFile('Scripts/SimScpStreamsRequest.xml');
c.ModifyXml("SCP_STREAMS_REQUEST","CONF_NAME",confname)
c.ModifyXml("SCP_STREAMS_REQUEST","PARTY_NAME","Party1")
c.ModifyXml("SCP_STREAMS_REQUEST","NUMBER_OF_STREAMS",len(userVideoMap.keys()) - 1)
index = 0
for key in userVideoMap.keys():
    if userVideoMap[key]["width"][2] != "1920":
        sys.exit("Video of Party is not 1080HD!")
    if key == "Party1":
        continue
    c.AddXML("SCP_STREAMS_REQUEST", "STREAM_DESCRIPTION");
    c.AddXML("STREAM_DESCRIPTION", "SRC_ID", userVideoMap[key]["src-id"][0], index)
    c.AddXML("STREAM_DESCRIPTION", "WIDTH", userVideoMap[key]["width"][0], index)
    c.AddXML("STREAM_DESCRIPTION", "HEIGHT", userVideoMap[key]["height"][0], index)
    c.AddXML("STREAM_DESCRIPTION", "FRAME_RATE", userVideoMap[key]["frame-rate"][0], index)
    c.AddXML("STREAM_DESCRIPTION", "BIT_RATE", userVideoMap[key]["bit-rate"][0], index)
    index = index + 1
c.Send()
sleep(5)

print " -> Remove subscriber"
c.LoadXmlFile('Scripts/SimAddSipSubscription.xml')
c.ModifyXml("SIP_ADD_SUBSCRIPTION","CONF_NAME",confname+"@plcm.com")
c.ModifyXml("SIP_ADD_SUBSCRIPTION","SUBSCRIBER",partySipAdd)
c.ModifyXml("SIP_ADD_SUBSCRIPTION","EXPIRES","0")
c.Send()
sleep(5)

# delete all parties  
for x in range(num_party):
    partyname = "Party"+str(x+1)
    #print "delete party: " + partyname    
    c.DeleteParty(confid,party_id_list[x])
    sleep(1) 
    
sleep(2)   

c.DeleteConf(confid)

c.Disconnect()

