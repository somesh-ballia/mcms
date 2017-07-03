#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROC-disabled-ESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

from PartyUtils.H323PartyUtils import *
#############################################################################
# Test Script which Awakes a Meeting room by an undefined party
#  
# Date: 16/01/05
# By  : Udi B.
#############################################################################

from McmsConnection import *

def TestAwakeMrWithUndefParty(connection,num_retries):
    print "Starting test TestAwakeMrWithUndefParty"
    
    #add a new profile
    profId = connection.AddProfile("MrProfile", "Scripts/AwakeMrByUndef/CreateNewProfile.xml")

    #Adding a new Meeting room
    mrName = "mrRsrv"
    connection.CreateMR(mrName, profId, "Scripts/AwakeMrByUndef/CreateNewMR.xml")

    #Get the Mr numeric id when mr was already added
    mrNumericId = ""
    mrId = ""
    mrId,mrNumericId = connection.WaitMRCreated(mrName,num_retries)

    # Create a new undefined party and add it to the EPSim
    partyName = "PartyUndef1"
    connection.SimulationAddH323Party(partyName, mrName) 
    connection.SimulationConnectH323Party(partyName)

    #Wait untill Meeting room is awake
    numOfParties = 1
    mrConfId = connection.WaitUntillEQorMRAwakes(mrName,numOfParties,num_retries)
    
    #delete the Sim Party
    H323PartyUtilsClass = H323PartyUtils() # H323Party utils util class
    
    H323PartyUtilsClass.SimulationDeleteAllSimParties(connection,"TRUE")
    #connection.SimulationDeleteH323Party(partyName)
    sleep(1)
    #delete the new conf
    connection.DeleteConf(mrConfId)
    connection.WaitAllConfEnd(num_retries)
    
    #remove the MR Resrv
    connection.DelReservation(mrId,"Scripts/AwakeMrByUndef/RemoveMr.xml")
	
    #remove the profile
    connection.DelProfile(profId, "Scripts/AwakeMrByUndef/RemoveNewProfile.xml")
    sleep(1)
    return


## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestAwakeMrWithUndefParty(c,
                    30)# retries

c.Disconnect()

