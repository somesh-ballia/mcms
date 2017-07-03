#!/mcms/python/bin/python

# ##############################################################################
# Dialing in, H323, to conf "1234", requires profile 1080p60
#
# Date: 20/03/12
# By  : Assaf
#
# Re-Write date =03/10/13
# Re-Write name = Uri A.
#
#-LONG_SCRIPT_TYPE
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
################################################################################
#
#

from CapabilitiesSetsDefinitions import *
from ResourceUtilities import *
from PartyUtils.H323PartyUtils import *


c = ResourceUtilities()
c.Connect()
H323PartyUtilsClass = H323PartyUtils() # H323Party utils util class
H323PartyUtilsClass.SimulationDeleteAllSimParties(c)

# Creating profile
profId4096_Motion = c.AddProfileWithRate("Profile4096_motion",4096,"motion")

# Creating conference
confName = "undefConf"
c.CreateConfFromProfile(confName, profId4096_Motion)
confid = c.WaitConfCreated(confName)

numParties = 3
delayBetweenParticipants = 3

CapSet = "FULL CAPS 1080p60"
for y in range(numParties):
    partyname = "Party"+str(y)
    c.SimulationAddH323Party(partyname, confName,CapSet)
    c.SimulationConnectH323Party(partyname)
    
c.WaitAllPartiesWereAdded(confid,numParties,20)
sleep(1)
c.WaitAllOngoingConnected(confid,numParties*5,delayBetweenParticipants) 
#c.WaitAllOngoingConnected(confid,numParties,2)        

sleep(10)
        
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
c.DelProfile(profId4096_Motion)
c.Disconnect()
