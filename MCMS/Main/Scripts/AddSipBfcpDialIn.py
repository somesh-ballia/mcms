#!/mcms/python/bin/python

#############################################################################
# 
#
# Date: 16/01/12
# By  : Anna Chen
#
# Re-Write date =03/10/13
# Re-Write name = Uri A.
#
#-LONG_SCRIPT_TYPE
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

#############################################################################

from McmsConnection import *
from PartyUtils.H323PartyUtils import *

c = McmsConnection()
c.Connect()
H323PartyUtilsClass = H323PartyUtils() # H323Party utils util class

H323PartyUtilsClass.SimulationDeleteAllSimParties(c)

delayBetweenParticipants = 1
if(c.IsProcessUnderValgrind("ConfParty")):
    delayBetweenParticipants = 3

#creating conference
confname = "Conf1"
c.CreateConf(confname, 'Scripts/CheckBfcpModes/AddCpConf.xml')
confid = c.WaitConfCreated(confname)

numParties = 3

#adding parties with capsets that are already defined by EP-SIM
x = 0
CapSetForScript = ["FULL CAPSET","FULL CAPSET BFCP TCP","FULL CAPSET NO BFCP"]
for y in range(numParties):
    CapSet = CapSetForScript[y]
    print "caps are: " + CapSet
    partyname = "Party"+str(y)
    c.SimulationAddSipParty(partyname, confname, CapSet)
    c.SimulationConnectSipParty(partyname)

c.WaitAllPartiesWereAdded(confid,numParties,20)
sleep(1)
c.WaitAllOngoingConnected(confid,numParties,2)    

sleep(5) 

# delete all parties  
for x in range(numParties):
    partyname = "Party"+str(x+1)
    print "disconnecting party: " + partyname 
    c.SimulationDisconnectSipParty(partyname)   
    c.SimulationDeleteSipParty(partyname)
    sleep(1)	

c.DeleteAllConf(2);
c.WaitAllConfEnd()
c.Disconnect()


##-----------------------------------------------------------------------------------------------

