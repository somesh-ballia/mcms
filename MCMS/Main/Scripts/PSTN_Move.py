#!/usr/bin/python


#############################################################################
#
# Date: 06/12/06
# By  : Udi B.
#############################################################################

from McmsConnection import *
import os   

## ----------------------  --------------------------
def TestDialInParty(connection,eqId,ProfId,numOfParties,num_retries,isRemoteDisconnect):    
    
    #create the target Conf and wait untill it connected
    targetConfName = "targetConf"
    connection.CreateConfFromProfile(targetConfName,ProfId,"Scripts/MoveUndef/CreateNewConf.xml")
    targetConfID  = connection.WaitConfCreated(targetConfName,num_retries)
    
    targetConfNumericId = connection.GetConfNumericId(targetConfID)
    print "Target Conf Numeric id is " + str(targetConfNumericId)
    
    #Add Dial-in un-defined parties
    for x in range(numOfParties):
        partyname = "PstnParty"+str(x+1)
        phone="3355"
        connection.SimulationAddPSTNParty(partyname,phone)
        connection.SimulationConnectPSTNParty(partyname)
        sleep(1)
    
    eqConfId = connection.WaitUntillEQorMRAwakes(targetEqName,numOfParties,num_retries,True)
    connection.WaitAllOngoingConnected(eqConfId)
    
    #send the TDMF to the EPsim with the numeric id of the target conf
    for x in range(numOfParties):
       partyname = "PstnParty"+str(x+1)
       connection.SimulationH323PartyDTMF(partyname, targetConfNumericId)

    #Make sure the parties are connected in the target Conf.
    connection.WaitAllPartiesWereAdded(targetConfID,numOfParties,numOfParties*num_retries)
    
    connection.WaitAllOngoingConnected(targetConfID)
    connection.WaitAllOngoingNotInIVR(targetConfID)
	
    if isRemoteDisconnect == True :
        print "Disconnect the parties from Remote"
        for x in range(numOfParties):
            partyname = "PstnParty"+str(x+1)
            connection.DeletePSTNPartyFromSimulation(partyname)
        connection.WaitAllOngoingDisConnected(targetConfID)
        
    else:
        print "Disconnect the parties from EMA"

    #Delete The Target Conference
    connection.DeleteConf(targetConfID)
    connection.WaitConfEnd(targetConfID)

    #delete The EQ COnf
    connection.DeleteConf(eqConfId)
    connection.WaitConfEnd(eqConfId)
    
    
    print "delete participants from the Simulation"
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
eqPhone="3355"
c.CreatePSTN_EQ(targetEqName, eqPhone,ProfId)
eqId, eqNID = c.WaitMRCreated(targetEqName)

TestDialInParty(c,
                eqId,  #EQ reservation ID
                ProfId,#Profile ID
                3,
                20, # Num of retries
                True)#Disconnect from Remote

TestDialInParty(c,
                eqId,  #EQ reservation ID
                ProfId,#Profile ID
                3,
                20, # Num of retries
                False)#Disconnect From EMA (by delete Conference)

		
#remove the EQ Resrv
c.DelReservation(eqId, "Scripts/AwakeMrByUndef/RemoveMr.xml")

#remove the profile
c.DelProfile(ProfId)
c.Disconnect()
