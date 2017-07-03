#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
# Test Script which Awakes a Meeting room with defined Sip party
#  
# Date: 16/01/05
# By  : Udi B.
#############################################################################

from McmsConnection import *

def TestAwakeMrWithDefinedParties(connection,num_retries):
    print "Starting test TestAwakeMrWithDefinedParties"
    
    IsConfPartyUnderValgrind = connection.IsProcessUnderValgrind("ConfParty")
    
    #add a new profile
    profId = connection.AddProfile("profile", "Scripts/AwakeMrByUndef/CreateNewProfile.xml")

    #Adding a new Meeting room
    mrName = "mrSip"
    connection.CreateMR(mrName, profId, "Scripts/AwakeMRWithDefinedDialIn/SipMR_WithDefParties.xml")

    #Get the Mr numeric id when mr was already added
    mrNumericId = ""
    mrId = ""
    mrId,mrNumericId = connection.WaitMRCreated(mrName,num_retries)

    # Create a new party and add it to the EPSim
    partyDilaInName = "SimParty"
    connection.SimulationAddSipParty(partyDilaInName, mrName)
    print "Sleeping for 2 seconds"
    sleep(2) 
    connection.SimulationConnectSipParty(partyDilaInName)

    # Give time before checking connection - failed under valgrind
    if (IsConfPartyUnderValgrind):
        command_line = "Bin/McuCmd set ConfParty DELAY_BETWEEN_DIAL_OUT_PARTY 10000" + '\n'
        os.system(command_line)
        print "Sleeping for 40 seconds .. "
        sleep(40)
        
    #Wait untill Meeting room is awake
    numOfParties = 4
        
    mrConfId = connection.WaitUntillEQorMRAwakes(mrName,numOfParties,num_retries)

    #Make sure all IVR Messages were finished
    connection.WaitAllOngoingNotInIVR(mrConfId,num_retries)
    
    if (IsConfPartyUnderValgrind):
        command_line = "Bin/McuCmd set ConfParty DELAY_BETWEEN_DIAL_OUT_PARTY 250" + '\n'
        os.system(command_line)
        
    #Deleting all parties
    partyId =""
    for index in range(numOfParties):
        if index == numOfParties -1 :
            partyname="SipDialIn"
        else:
            partyname="DialOut"+str(index+1)           
            
        partyId = connection.GetPartyId(mrConfId , partyname)
        connection.DeleteParty(mrConfId,partyId)

    connection.WaitUntillPartyDeleted(mrConfId,num_retries)

    #delete the new conf
    connection.DeleteConf(mrConfId)

    connection.WaitAllConfEnd(num_retries)

    #delete the Sim Party
    connection.SimulationDeleteH323Party(partyDilaInName)
    
    #remove the MR Resrv
    connection.DelReservation(mrId, 'Scripts//AddRemoveMrNew/DeleteMR.xml')

    #remove the profile
    connection.DelProfile(profId, "Scripts/AwakeMrByUndef/RemoveNewProfile.xml")

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestAwakeMrWithDefinedParties(c,
                    30)# retries

c.Disconnect()

