#!/usr/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind


#############################################################################
# Test Script checks conference with one ISDN dial-in party and one dial-out; 
#      each of them has 6 channels, i.e. their rate equals to 768k
# Date: 20/01/08
# By  : Olga S.
#############################################################################

from McmsConnection import *
from ISDNFunctions import *
        
# -----------------------------------------------------
        
def TestISDNConf(connection,num_retries,isRemoteDisconnect):
    print "Starting Test ISDN Conf... "
        

    #Adding ISDN parties:
    partyname_in = "PartyIn"
    phone="3344"
    numChannels="12"
    SimulationAddIsdnParty(connection,partyname_in,phone,numChannels)
    SimulationConnectIsdnParty(connection,partyname_in)

    eqConfId = connection.WaitUntillEQorMRAwakes(targetEqName,1,num_retries,True)
    #connection.WaitAllOngoingConnected(eqConfId, num_retries)

    #send the TDMF to the EPsim with the numeric id of the target conf
    targetConfNumericId = connection.GetConfNumericId(confId)
    connection.SimulationH323PartyDTMF(partyname_in, targetConfNumericId)

    #Sleeping for IVR messages
    print "Sleeping untill IVR finishes..."
    sleep(5)

    partyname_out = "PartyOut"
    phoneout="3333"
    chnlNum="channel_12"
    print "Add Dial Out party"
    AddIsdnDialoutParty(connection,confId,partyname_out,phoneout,chnlNum)

    #Make sure the parties are connected in the target Conf.
    connection.WaitAllPartiesWereAdded(confId, 2, num_retries)
    connection.WaitAllOngoingConnected(confId, 2*num_retries)
    
    sleep(10)

    # Check if all parties were added and save their IDs
    num_of_parties = 2 
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confId)
    connection.Send()
    ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    if len(ongoing_party_list) < num_of_parties:
        sys.exit("some parties are lost...")

    # delete all parties
    if isRemoteDisconnect == False:
        print "Disconnecting parties from EMA"
	for index in range(num_of_parties):    
            party_id = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data
            connection.DeleteParty(confId,party_id)
            sleep(2)
    else:
        print "Disconnecting parties from remote EP"
        connection.DeletePSTNPartyFromSimulation(partyname_in)
        connection.DeletePSTNPartyFromSimulation(partyname_out)
        sleep(2)

    connection.WaitAllOngoingDisConnected(confId)
    connection.DeleteConf(eqConfId)
    connection.WaitConfEnd(eqConfId)


## ---------------------- Test --------------------------
if __name__ == '__main__':
    connection=McmsConnection()
    connection.Connect()
    
    #add a new profile
    ProfId = AddIsdnProfile(connection, "profile1", "768")

    #create the EQ Conf and wait untill it is connected
    targetEqName = "ISDN_EQ"
    eqPhone="3344"
    connection.CreatePSTN_EQ(targetEqName, eqPhone,ProfId)
    eqId, eqNID = connection.WaitMRCreated(targetEqName)
    
    #create the target Conf and wait untill it is connected
    num_retries=20
    targetConfName = "ISDN_Conf"
    connection.CreateConfFromProfile(targetConfName, ProfId)
    confId  = connection.WaitConfCreated(targetConfName,num_retries)
    TestISDNConf(connection, num_retries, False) #isRemoteDisconnection

    #remove the EQ Resrv
    connection.DelReservation(eqId, "Scripts/AwakeMrByUndef/RemoveMr.xml")
    
    #remove the main Conf
    connection.DeleteConf(confId)
    connection.WaitConfEnd(confId)

    #remove the profile
    connection.DelProfile(ProfId)
    connection.Disconnect()
