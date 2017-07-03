#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
# Test Script which Awakes a Meeting room with defined parties in it
#  
# Date: 16/01/05
# By  : Udi B.
#############################################################################

from McmsConnection import *

def TestAwakeMrWithDefinedParties(connection,num_retries):
    IsConfPartyUnderValgrind = connection.IsProcessUnderValgrind("ConfParty")
    
    print "Starting test TestAwakeMrWithDefinedParties"
    
    #add a new profile
    profId = connection.AddProfile("profile", "Scripts/AwakeMrByUndef/CreateNewProfile.xml")

    #Adding a new Meeting room
    mrName = "mr323"
    connection.CreateMR(mrName, profId, "Scripts/AwakeMRWithDefinedDialIn/MR_WithDefPArties.xml")

    #Get the Mr numeric id when mr was already added
    mrNumericId = ""
    mrId = ""
    mrId,mrNumericId = connection.WaitMRCreated(mrName,num_retries)

    # Create a new party and add it to the EPSim
    partyName = "SimParty"
    connection.SimulationAddH323Party(partyName, mrName)
    print "Sleeping for 2 seconds"
    sleep(2)    
    connection.SimulationConnectH323Party(partyName)
        
    numOfParties = 4
    
    # Give time before checking connection - failed under valgrind
    if (IsConfPartyUnderValgrind):
        command_line = "Bin/McuCmd set ConfParty DELAY_BETWEEN_DIAL_OUT_PARTY 10000" + '\n'
        os.system(command_line)
        print "Sleeping for 40 seconds .. "
        sleep(40)
        
    #Wait untill Meeting room is awake
    mrConfId = connection.WaitUntillEQorMRAwakes(mrName,numOfParties,num_retries)

    #Make sure all IVR Messages were finished
    connection.WaitAllOngoingNotInIVR(mrConfId,num_retries)
    
    if (IsConfPartyUnderValgrind):
        command_line = "Bin/McuCmd set ConfParty DELAY_BETWEEN_DIAL_OUT_PARTY 250" + '\n'
        os.system(command_line)
    
    #delete the new conf
    connection.DeleteConf(mrConfId)

    connection.WaitAllConfEnd(num_retries)

    #delete the Sim Party
    connection.SimulationDeleteH323Party(partyName)
    
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

