#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#-LONG_SCRIPT_TYPE

from McmsConnection import *
from ISDNFunctions import *

num_retries=50
num_of_parties=4


c = McmsConnection()
c.Connect()

confName = "conf1"
c.CreateConf(confName, 'Scripts/CreateCPConfWith4DialInParticipants/AddCpConf.xml')
confid = c.WaitConfCreated(confName,num_retries)

#add a new profile
ProfId = c.AddProfile("CPConfWith4DialInParticipantsProfile")
#create the EQ Conf and wait untill it is connected
targetEqName = "ISDN_EQ"
eqPhone="3344"
c.CreatePSTN_EQ(targetEqName, eqPhone, ProfId)
eqId, eqNID = c.WaitMRCreated(targetEqName)

delay = 1
if(c.IsProcessUnderValgrind("ConfParty")):
    delay = 2

# Add Dial In Parties
c.LoadXmlFile("Scripts/CreateCPConfWith4DialInParticipants/AddDialInParty.xml")
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    partyip =  "123.123.0." + str(x+1)
    print "Adding Party..." + partyname + " ip: " + partyip
    c.ModifyXml("PARTY","NAME",partyname)
    c.ModifyXml("PARTY","IP",partyip)
    c.ModifyXml("ADD_PARTY","ID",confid)
    c.Send()

c.WaitAllPartiesWereAdded(confid,num_of_parties,num_retries)

# Check if all parties were added and save their IDs
party_id_list = []
for index in range(num_of_parties):    
    party_name = "Party"+str(index+1)
    party_id_list.append(c.GetPartyId(confid, party_name))

sleep(5)
                         
# Add parties to EP Sim and connect them
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    c.SimulationAddH323Party(partyname, "conf1")
    c.SimulationConnectH323Party(partyname)
    if (c.IsProcessUnderValgrind("ConfParty")):
        c.WaitPartyConnected(confid,party_id_list[x], num_retries)
	    
c.WaitAllOngoingConnected(confid, num_retries)

for x in range(num_of_parties):
    partyname = "IsdnParty"+str(x+1)
    phone="3344"
    SimulationAddIsdnParty(c,partyname,phone)
    SimulationConnectIsdnParty(c,partyname)
    sleep(delay)
        
eqConfId = c.WaitUntillEQorMRAwakes(targetEqName,num_of_parties,num_retries,True)
c.WaitAllOngoingConnected(eqConfId,num_of_parties*num_retries)

#send the TDMF to the EPsim with the numeric id of the target conf
targetConfNumericId = c.GetConfNumericId(confid)
for x in range(num_of_parties):
    partyname_isdn = "IsdnParty"+str(x+1)
    c.SimulationH323PartyDTMF(partyname_isdn, targetConfNumericId)
    sleep(delay)

c.WaitAllPartiesWereAdded(confid, num_of_parties*2, num_retries*2)
c.WaitAllOngoingConnected(confid, num_retries*2)
c.WaitAllOngoingNotInIVR(confid)


# Disconnect and Delete parties in different ways 
c.SimulationDisconnectH323Party("Party1")

#DelSimParty("Party1") --> redundant - deleted by the destructor

print "Disconnecting second party from Operator..."+str(party_id_list[1])
c.DisconnectParty(confid,party_id_list[1])

print "Delete third party from conference..."+str(party_id_list[2])
c.DeleteParty(confid,party_id_list[2])

print "Delete fourth party from conference..."+str(party_id_list[3])
c.DeleteParty(confid,party_id_list[3])

# Disconnect and delete ISDN parties 
for x in range(num_of_parties):
    partyname_isdn = "IsdnParty"+str(x+1)
    c.SimulationDisconnectPSTNParty(partyname_isdn)   
    c.DeletePSTNPartyFromSimulation(partyname_isdn)
    sleep(delay*2)

c.WaitAllOngoingDisConnected(confid, num_retries*2)
#DelSimParty("Party3") --> redundant - deleted by the destructor

#remove the EQ Reservation
c.DeleteConf(eqConfId)
c.WaitConfEnd(eqConfId)
c.DelReservation(eqId, "Scripts/AwakeMrByUndef/RemoveMr.xml")
c.DelProfile(ProfId)

c.DeleteConf(confid);

#DelSimParty("Party4") --> redundant - deleted by the destructor
#DelSimParty("Party2") --> redundant - deleted by the destructor

c.WaitAllConfEnd()

c.Disconnect()


