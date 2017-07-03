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

def TestAwakeMrWithUndefinedParty(connection,num_retries):
    print "Starting test TestAwakeMrWithUndefinedParty"
    
    #add a new profile
    profId = connection.AddProfile("profile", "Scripts/AwakeMrByUndef/CreateNewProfile.xml")

    #Adding a new Meeting room
    mrName = "mrSip"
    connection.CreateMR(mrName, profId, "Scripts/AwakeMrByUndef/CreateNewMR.xml")

    #Get the Mr numeric id when mr was already added
    mrNumericId = ""
    mrId = ""
    mrId,mrNumericId = connection.WaitMRCreated(mrName,num_retries)

    # Create a new party and add it to the EPSim
    partyName = "SimParty"
    connection.SimulationAddSipParty(partyName, mrName)
    connection.SimulationConnectSipParty(partyName)

    #Wait untill Meeting room is awake
    numOfParties = 1
    mrConfId = connection.WaitUntillEQorMRAwakes(mrName,numOfParties,num_retries)

    #delete the new conf
#   connection.DeleteConf(mrConfId)

#    connection.WaitAllConfEnd(num_retries)

    #delete the Sim Party
#    connection.SimulationDeleteH323Party(partyName)
    
    #remove the MR Resrv
#    connection.DelReservation(mrId, 'Scripts//AddRemoveMrNew/DeleteMR.xml')

    #remove the profile
#    connection.DelProfile(profId, "Scripts/AwakeMrByUndef/RemoveNewProfile.xml")

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestAwakeMrWithUndefinedParty(c,
                    30)# retries

c.Disconnect()

