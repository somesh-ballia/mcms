#!/mcms/python/bin/python

#############################################################################
# 
#
# Date: 05/03/06
# By  : Ron S.

#############################################################################


from McmsConnection import *

###---------------------------------------------------------------------------------------
num_retries=20
num_of_parties=6

##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

confname = "Conf1"
c.CreateConf(confname, 'Scripts/CheckMultiTypesOfCalls/AddConf.xml')
confid = c.WaitConfCreated(confname,num_retries)    


#Add Dial In SIP Party
partyname = "Party"+str(2)
partyip =  "123.123.0." + str(2)
partySipAdd = partyname + '@' + partyip
c.AddSIPParty(confid, partyname, partyip, partySipAdd, "Scripts/CheckMultiTypesOfCalls/AddDialInSipParty.xml")
#c.WaitAllPartiesWereAdded(confid,2,num_retries)

sleep(2)

# Add Sip party to EP Sim and connect him
partyname = "Party"+str(2)
c.SimulationAddSipParty(partyname, confname)
c.SimulationConnectSipParty(partyname)
  
c.WaitAllOngoingConnected(confid,num_retries)

sleep(2)

# adding 1 dial in SIP undefined video call
# Add Sip party to EP Sim and connect him
partyname = "Party"+str(4)
c.SimulationAddSipParty(partyname, confname)
c.SimulationConnectSipParty(partyname)

sleep(2)
  
#c.WaitAllOngoingConnected(confid,num_retries)


#adding 1 dial in SIP undefined call when remote caps are audio only.
print 'adding dial in SIP audio call'
partyname = "Party"+str(6)
partyip =  "123.123.0." + str(6)
partySipAdd = partyname + '@' + partyip
c.AddSIPParty(confid, partyname, partyip, partySipAdd, "Scripts/CheckTypesOfDialInCalls/AddDialInSipAudioParty.xml")

sleep(2)

#c.WaitAllPartiesWereAdded(confid,2,num_retries)

# Add Sip party to EP Sim and connect him
partyname = "Party"+str(6)
c.SimulationAddSipParty(partyname, confname)
c.SimulationConnectSipParty(partyname)
  
#c.WaitAllOngoingConnected(confid,num_retries)

sleep(2)

c.WaitAllPartiesWereAdded(confid,3,num_retries)
c.WaitAllOngoingConnected(confid,num_retries)
party_id_list = []
c.LoadXmlFile('Scripts/CreateCPConfWith4DialInParticipants/TransConf2.xml')
c.ModifyXml("GET","ID",confid)
c.Send()
ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
if len(ongoing_party_list) < 6:
    sys.exit("some parties are lost...")
for index in range(num_of_parties):    
    party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)


#adding 1 rejected SIP incoming invite
# Add Sip party to EP Sim and connect him
partyname = "Party"+str(8)
c.SimulationAddSipParty(partyname, confname2)
print "Connecting Sim Sip Party "+partyname+"..."
c.LoadXmlFile("Scripts/CheckMultiTypesOfCalls/SimConnectSipParty.xml")
c.ModifyXml("SIP_PARTY_CONNECT","PARTY_NAME",partyname)
c.ModifyXml("SIP_PARTY_CONNECT","CONF_NAME",confname2)
c.Send()
  
sleep(2)

c.WaitAllPartiesWereAdded(confid,6,num_retries)
c.WaitAllOngoingConnected(confid,num_retries)

# Check if all parties were added and save their IDs
party_id_list = []
c.LoadXmlFile('Scripts/CreateCPConfWith4DialInParticipants/TransConf2.xml')
c.ModifyXml("GET","ID",confid)
c.Send()
ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
if len(ongoing_party_list) > num_of_parties:
    sys.exit("more parties than should be...")
for index in range(num_of_parties):    
    party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)
    
# delete all parties  
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    print "delete party: " + partyname    
    c.DeleteParty(confid,party_id_list[x])
    c.Send()
    sleep(1)
    
sleep(2)    

c.DeleteConf(confid);
c.WaitAllConfEnd()
c.Disconnect()


##-----------------------------------------------------------------------------------------------

