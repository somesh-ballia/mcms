#!/mcms/python/bin/python

#############################################################################
# Test Script which Awakes a Meeting room Via Entry Queue by 3 Blast undefined parties
#  
# Date: 16/01/06
# By  : Udi B.
# Last Update :26/01/06 By Yoella
#############################################################################

from McmsConnection import *

def TestAwakeMrBy3BlastUndefViaEntryQ(connection,num_retries):
    print "Starting test TestAwakeMrBy3BlastUndefViaEntryQ"
    
    #add a new profile
    profId = connection.AddProfile("Prof1", "Scripts/CreateAdHoc3BlastUndefParties/CreateNewProfile.xml")

    #Adding a new Meeting room
    mrName = "mrRsrv"
    connection.CreateMR(mrName, profId)

    #Get the Mr numeric id when mr was already added
    mrNumericId = ""
    mrId = ""
    mrId,mrNumericId = connection.WaitMRCreated(mrName,num_retries)
    
    #Add a new EQ service
    print "Adding a new Entry Queue Service...."
    connection.SendXmlFile("Scripts/CreateAdHoc3BlastUndefParties/AddEqService.xml","Status OK")

    # TODO: wait untill the EQ-Service will be added

    # send a new Reservation of EntryQueue
    eqName = "eqRsrv"
    connection.CreateAdHocEQ(eqName, profId, "Scripts/CreateAdHoc3BlastUndefParties/CreateNewEq.xml")

    #Wait untill eq was added
    connection.WaitMRCreated(eqName,num_retries)
    
    num_parties = 1
    ConnectXSimulationParties(connection,eqName,num_parties,num_retries)
    
    eqConfId = connection.WaitUntillEQorMRAwakes(eqName,num_parties,num_retries,True)
       
    DtmfXSimulationParties(connection,mrNumericId,num_parties)
    sleep(3)
    
    #Wait untill the MR conf will be on Air
    mrConfId = connection.WaitUntillEQorMRAwakes(mrName, num_parties,num_retries)
    
    #Delete the parties
    RemoveXSimulationParties(connection,num_parties)
    
    #delete the AdHoc conf
    connection.DeleteConf(mrConfId)
    
    #delete the eq conf
    connection.DeleteConf(eqConfId)

    connection.WaitAllConfEnd(num_retries)
    
    #remove the IVR Service - Missing Does Not Supported yet
   
   
    #remove the MR Resrv
    connection.DelReservation(mrId, "Scripts/AwakeMrByUndef/RemoveMr.xml")
    #remove the EQ Resrv
    connection.DelReservation(2, "Scripts/AwakeMrByUndef/RemoveMr.xml")
    #remove the profile
    connection.DelProfile(profId, "Scripts/CreateAdHoc3BlastUndefParties/RemoveNewProfile.xml")
    
    return

#------------------------------------------------------------------------------
def ConnectXSimulationParties(connection,eqName,num_parties,num_retries):
    # Create a new undefined party and add it to the EPSim
    for x in range(num_parties):
        partyName = "Party" + str(x+1)
        connection.SimulationAddH323Party(partyName, eqName)
        #connect the undefined party
        connection.SimulationConnectH323Party(partyName)
        
    sleep(3)
    return 
#------------------------------------------------------------------------------
def DtmfXSimulationParties(connection,adHocConfNumericId,num_parties):
     
    #Send the DTMF which represent the new Ad-hoc conf
    for x in range(num_parties):
        partyName = "Party" + str(x+1)
        connection.SimulationH323PartyDTMF(partyName, adHocConfNumericId)
    return
#------------------------------------------------------------------------------
def RemoveXSimulationParties(connection,num_parties):
     
    #delete the Sim Parties
    for x in range(num_parties):
        partyName = "Party" + str(x+1)
        connection.SimulationDeleteH323Party(partyName)
           
    return

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestAwakeMrBy3BlastUndefViaEntryQ(c,
                    30)# retries

c.Disconnect()

