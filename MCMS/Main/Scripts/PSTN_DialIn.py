#!/usr/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
#
# Date: 06/12/06
# By  : Udi B.
#############################################################################

from McmsConnection import *
import os

## ----------------------  --------------------------
def TestDialInParty(connection,eqId,ProfId,numOfParties,num_retries,isRemoteDisconnect):    

    #Add Dial-in un-defined parties
    for x in range(numOfParties):
        partyname = "PstnParty"+str(x+1)
        phone="3344"
        connection.SimulationAddPSTNParty(partyname,phone)
        connection.SimulationConnectPSTNParty(partyname)
        sleep(1)
        
    eqConfId = connection.WaitUntillEQorMRAwakes(targetEqName,numOfParties,num_retries,True)
    connection.WaitAllOngoingConnected(eqConfId)

    #Sleeping for IVR messages
    print "Sleeping..."
    sleep(2)
    
    if isRemoteDisconnect == True :
        print "Disconnecting parties from remote"
        for x in range(numOfParties):
            partyname = "PstnParty"+str(x+1)
            connection.DeletePSTNPartyFromSimulation(partyname)
            
        connection.WaitAllOngoingDisConnected(eqConfId)
      
    connection.DeleteConf(eqConfId)
    connection.WaitAllConfEnd(num_retries)
    if isRemoteDisconnect == False :
        for x in range(numOfParties):
            partyname = "PstnParty"+str(x+1)
            connection.DeletePSTNPartyFromSimulation(partyname)
    
         
#------------------------------------------------------------------------------
def ConnectXSimulationParties(connection,targetConf,num_parties,num_retries):     
    # Create a new undefined party and add it to the EPSim
    for x in range(num_parties):
        partyName = "Party" + str(x+1)
        connection.SimulationAddH323Party(partyName, targetConf)
        connection.SimulationConnectH323Party(partyName)
        
    return

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()
#add a new profile
ProfId = c.AddProfile("profile")

#create the target Conf and wait untill it connected
targetEqName = "PstnEQ"
eqPhone="3344"
c.CreatePSTN_EQ(targetEqName, eqPhone,ProfId)
eqId, eqNID = c.WaitMRCreated(targetEqName)

TestDialInParty(c,
                eqId,  #EQ reservation ID
                ProfId,#Profile ID
                2,
                20, # Num of retries
                True)#Disconnect from Remote

TestDialInParty(c,
                eqId,  #EQ reservation ID
                ProfId,#Profile ID
                2,
                20, # Num of retries
                False)#Disconnect From EMA (by delete Conference)

#remove the EQ Resrv
c.DelReservation(eqId, "Scripts/AwakeMrByUndef/RemoveMr.xml")

#remove the profile
c.DelProfile(ProfId)
c.Disconnect()
