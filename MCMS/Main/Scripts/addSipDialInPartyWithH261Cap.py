#!/mcms/python/bin/python

# #############################################################################
# Creating Conf with Dial In SIP party, which has H261 video codec only 
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

#creating conference
confName = "SipH261Conf"
c.CreateConf(confName, 'Scripts/UndefinedDialIn/AddCpConf.xml')
confid = c.WaitConfCreated(confName)

numParties = 2
delayBetweenParticipants = 3
CapSet = "H261+ALL"

#Add dial out SIP Party and disconnect it
partyname1 = "Party1##SIMULATION_FORCE_H261"
c.AddVideoParty(confid, partyname1, "1.2.3.2",True)
c.WaitAllOngoingConnected(confid,3)
SipPartyid = c.GetPartyId(confid, partyname1)    

partyname = "Party"+str(numParties)
#numParties = numParties + 1
c.SimulationAddSipParty(partyname, confName,CapSet)
c.SimulationConnectSipParty(partyname)
       
c.WaitAllPartiesWereAdded(confid,numParties,20)
sleep(1)
c.WaitAllOngoingConnected(confid,numParties,2)        

sleep(1)
        
# delete all parties  
for x in range(numParties):
    partyname = "Party"+str(x+1)
    print "disconnecting party: " + partyname 
    c.SimulationDisconnectSipParty(partyname)   
    c.SimulationDeleteSipParty(partyname)
    sleep(1)
c.DisconnectParty(confid,SipPartyid) 

c.DeleteConf(confid)
c.WaitAllConfEnd()

c.Disconnect()
