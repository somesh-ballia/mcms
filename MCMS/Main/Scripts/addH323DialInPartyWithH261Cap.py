#!/mcms/python/bin/python

# #############################################################################
# Creating Conf with Dial In H323 party, which has H61 video codec only 
#
# Date: 03/09/07
# By  : Uri 

#
############################################################################
#
#

from CapabilitiesSetsDefinitions import *
from McmsConnection import *

c = McmsConnection()
c.Connect()

# creating conference
confName = "undefConf"
c.CreateConf(confName, 'Scripts/UndefinedDialIn/AddCpConf.xml')
confid = c.WaitConfCreated(confName)

numParties = 0
delayBetweenParticipants = 3

partyname = "Party"+str(numParties+1)
numParties = numParties + 1
CapSet = "H261+ALL"
c.SimulationAddH323Party(partyname, confName,CapSet)
c.SimulationConnectH323Party(partyname)
    
       
c.WaitAllPartiesWereAdded(confid,numParties,20)
sleep(1)
c.WaitAllOngoingConnected(confid,numParties*5,delayBetweenParticipants) 
#c.WaitAllOngoingConnected(confid,numParties,2)        

sleep(15)
        
# Check if all parties were added and save their IDs
party_id_list = []
c.LoadXmlFile('Scripts/CreateCPConfWith4DialInParticipants/TransConf2.xml')
c.ModifyXml("GET","ID",confid)
c.Send()
ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
if len(ongoing_party_list) < numParties:
    sys.exit("some parties are lost...")
for index in range(numParties):    
    party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)

    
    
# delete all parties  
for x in range(numParties):
    partyname = "Party"+str(x+1)
    print "disconnecting party: " + partyname 
    c.SimulationDisconnectH323Party(partyname)   
    c.SimulationDeleteH323Party(partyname)
    sleep(1)
 

c.DeleteConf(confid)
c.WaitAllConfEnd()

c.Disconnect()
