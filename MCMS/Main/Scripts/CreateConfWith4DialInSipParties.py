#!/mcms/python/bin/python

#############################################################################
# Creating Conf with a 4 defined Dial In SIP participants
#
# Date: 23/01/05
# By  : Ron S.

#############################################################################


from McmsConnection import *

###---------------------------------------------------------------------------------------
num_retries=20
num_of_parties=4


##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

confname = "Conf1"
c.CreateConf(confname, 'Scripts/CreateConfWith4DialInSipParties/AddConf.xml')
confid = c.WaitConfCreated(confname,num_retries)

# Add Dial In SIP Parties
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    partyip =  "123.123.0." + str(x+1)
    partySipAdd = partyname + '@' + partyip
    c.AddSIPParty(confid, partyname, partyip, partySipAdd, "Scripts/CreateConfWith4DialInSipParties/AddDialInSipParty.xml")

c.WaitAllPartiesWereAdded(confid,num_of_parties,num_retries)
# Check if all parties were added and save their IDs
party_id_list = []
c.LoadXmlFile('Scripts/CreateConfWith4DialInSipParties/TransConf2.xml')
c.ModifyXml("GET","ID",confid)
c.Send()
ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
if len(ongoing_party_list) < num_of_parties:
    sys.exit("some parties are lost...")
for index in range(num_of_parties):    
    party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)    


# Add parties to EP Sim and connect them
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    c.SimulationAddSipParty(partyname, confname)
    c.SimulationConnectSipParty(partyname)
  
c.WaitAllOngoingConnected(confid,num_retries)
# Disconnect and Delete parties in different ways 
print "disconnect first participant from EPsim"
c.SendXmlFile("Scripts/CreateConfWith4DialInSipParties/SimDisconnectSipParty.xml")

c.SimulationDeleteSipParty("Party1")
 
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    c.SimulationDeleteSipParty(partyname)

print "Disconnecting second party from Operator..."
c.LoadXmlFile("Scripts/CreateConfWith4DialInSipParties/DisconnectSipParty.xml")
c.ModifyXml("SET_CONNECT","PARTY_ID",party_id_list[1])
c.Send()

print "Delete third party from conference..."
c.DeleteParty(confid,party_id_list[2])

c.SimulationDeleteSipParty("Party3")

c.DeleteConf(confid);

c.SimulationDeleteSipParty("Party4")
c.SimulationDeleteSipParty("Party2")

c.WaitAllConfEnd()

c.Disconnect()

##-----------------------------------------------------------------------------------------------

