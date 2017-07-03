#!/usr/bin/python

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~##
##Creating Conf with Dial In SIP party that supports Video VP8 only
##
## Date: March 2014
## By  : Natali 
##
##
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~##
#
#

from CapabilitiesSetsDefinitions import *
from McmsConnection import *

c = McmsConnection()
c.Connect()
delayBetweenParticipants = 1
if(c.IsProcessUnderValgrind("ConfParty")):
    delayBetweenParticipants = 3
if(c.IsProcessUnderValgrind("EndpointsSim")):
    delayBetweenParticipants = 2

#creating conference
confName = "SipDifferentPartiesConf"
c.CreateConf(confName, 'Scripts/UndefinedDialIn/AddCpConf.xml')
confid = c.WaitConfCreated(confName)

numParties = 0
##CapSet1 = "FULL CAPSET"
CapSet1 = "WEBRTC_CAPS"
#CapSet2 = "Audio Only"
#CapSet3 = "Invite No SDP A+V"
#CapSet1 = "Invite No SDP A only"

#adding parties with capsets that are already defined by EP-SIM
for y in range(1):
    numParties = numParties + 1
    partyname = "Party"+str(numParties)
    CapSet = CapSet1 ##+str(y) ##N.A. DEBUG VP8
    c.SimulationAddSipParty(partyname, confName,CapSet)
    c.SimulationConnectSipParty(partyname)
       
c.WaitAllPartiesWereAdded(confid,numParties,20)
sleep(1)
c.WaitAllOngoingConnected(confid,numParties,2)        

sleep(300)
        
# delete all parties  
for x in range(numParties):
    partyname = "Party"+str(x+1)
    print "disconnecting party: " + partyname 
    c.SimulationDisconnectSipParty(partyname)   
    c.SimulationDeleteSipParty(partyname)
    sleep(1)
 

c.DeleteConf(confid)
c.WaitAllConfEnd()

c.Disconnect()