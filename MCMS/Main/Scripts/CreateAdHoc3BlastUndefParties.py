#!/mcms/python/bin/python
 
#############################################################################
# Test Script which connect 3 DailIn parties in Blast to an AdHoc configured EntryQue
# This test is checking the following:
# 1.The Creation of an Ad-Hoc Eq by an undefined party that awake EntryQueue
# 2.Move Party frome EQ to On Going AdHoc Conf (party 2 and up)
# 
# Date: 20/01/06
# By  : Udi B.
# Updated by Yoella on 25/01/06 from one party to 3
#############################################################################

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8
#*PROCESSES_NOT_FOR_VALGRIND=ConfParty

from McmsConnection import *
from ISDNFunctions import * 

def TestCreateAdHoc3BlastUndefParties(connection,num_retries):
    #add a new profile
    profId = connection.AddProfile("profile", "Scripts/CreateAdHoc3BlastUndefParties/CreateNewProfile.xml")
    #send a new EQ service
    print "Adding a new Entry Queue Service...."
    connection.SendXmlFile("Scripts/CreateAdHoc3BlastUndefParties/AddEqService.xml","Status OK")

    # TODO: wait untill the EQ-Service will be added

    # send a new Reservation of EntryQueue
    eqName = "eqRsrv"
    connection.CreateAdHocEQ(eqName, profId, "Scripts/CreateAdHoc3BlastUndefParties/CreateNewEq.xml")

    #Wait untill eq was added
    eqId, eqNID = connection.WaitMRCreated(eqName,num_retries)
    
    num_parties = 3
    ConnectXSimulationParties(connection,eqName,num_parties,num_retries)
    ConnectXIsdnSimulationParties(connection,eqName,num_parties,num_retries)
    
    #Wait untill Eq was awake and the Ad-Hoc conf will be created
    eqConfId = connection.WaitUntillEQorMRAwakes(eqName,num_parties*2,num_retries*2,True)
    connection.WaitUntilAllPartiesConnected(eqConfId, num_parties*2, num_parties*2*num_retries)
    
    adHocConfNumericId = "1515"
    
    print "Sending DTMFs from first party..."
    DtmfXSimulationParties(connection,adHocConfNumericId,1)
    
    #Wait untill the target conf will be created
    adHocConfId = connection.WaitConfCreated(adHocConfNumericId,num_parties*2*num_retries)
    
    print "Sending DTMFs from rest parties..."
    DtmfXSimulationParties(connection,adHocConfNumericId,num_parties,True)
        
    
    connection.WaitUntilAllPartiesConnected(adHocConfId,num_parties*2,num_parties*2*num_retries)
    connection.WaitAllOngoingNotInIVR(adHocConfId)
    sleep(2)
    
    RemoveXSimulationParties(connection,num_parties)
    sleep(4)
    
            
    #delete the eq conf
    connection.DeleteConf(eqConfId)

    #delete the AdHoc conf
    connection.DeleteConf(adHocConfId)

    connection.WaitAllConfEnd(num_retries)
    
    #remove the IVR Service - Missing Does Not Supported yet
    
    #remove the EQ Resrv
    print "Remove the EQ reservation..."
    connection.DelReservation(eqId, "Scripts/CreateAdHoc3BlastUndefParties/RemoveEq.xml")
    #remove the profile
    connection.DelProfile(profId, "Scripts/CreateAdHoc3BlastUndefParties/RemoveNewProfile.xml")
    return


#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
def ConnectXSimulationParties(connection,eqName,num_parties,num_retries):
     
    # Create a new undefined party and add it to the EPSim
    for x in range(num_parties):
        partyName = "AdHockParty" + str(x+1)
        connection.SimulationAddH323Party(partyName, eqName)    
        #connect the undefined party
        connection.SimulationConnectH323Party(partyName)
  
    return
#------------------------------------------------------------------------------
def DtmfXSimulationParties(connection,adHocConfNumericId,num_parties,startFromSecondParty = False):
    #Send the DTMF which represent the new Ad-hoc conf
    for x in range(num_parties*2):
        if (x == 0 and startFromSecondParty == True):
           continue
        if (x < num_parties):   
        	partyName = "AdHockParty" + str(x+1)
        else:
        	partyName = "IsdnAdHockParty" + str(x+1)
        #connection.SimulationH323PartyDTMF(partyName, adHocConfNumericId)
        #partyName = "IsdnAdHockParty" + str(x+1)
        connection.SimulationH323PartyDTMF(partyName, adHocConfNumericId)
#------------------------------------------------------------------------------

def RemoveXSimulationParties(connection,num_parties):
     
    #delete the Sim Parties
    for x in range(num_parties*2):
        if (x < num_parties):
            partyName = "AdHockParty" + str(x+1)
            connection.SimulationDeleteH323Party(partyName)
        else:
            partyname_isdn = "IsdnAdHockParty"+str(x+1)
            c.DeletePSTNPartyFromSimulation(partyname_isdn)         
    return
#------------------------------------------------------------------------------

def ConnectXIsdnSimulationParties(connection,eqName,num_parties,num_retries):
     
    # Create new ISDN undefined parties and add them to the EPSim
    for x in range(num_parties):
        partyName = "IsdnAdHockParty" + str(x+1+num_parties)
        phone="3234"
        SimulationAddIsdnParty(c,partyName,phone)    
        #connect the ISDN undefined party
        SimulationConnectIsdnParty(c,partyName)
   	sleep(1)
    return

#------------------------------------------------------------------------------
         
## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestCreateAdHoc3BlastUndefParties(c,50)# retries

c.Disconnect()


