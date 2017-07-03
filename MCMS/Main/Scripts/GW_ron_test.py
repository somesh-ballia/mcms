#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#-LONG_SCRIPT_TYPE

#############################################################################
# Test Script which test the Awake of an Ad-Hoc Eq by an undefined party (PSTN)
# and creates GW session
#  
# Date: 02/11/08
# By  : Eitan P.
#############################################################################

from McmsConnection import *

def CreateGW(connection,num_retries):

    partyName2 = "in0"
    gwName2="GW_Default_GW_Session(000)"
##    dialString = "2000*172.22.188.35"
    dialString = "2000*123456"
    connection.SimulationAddH323Party(partyName2, dialString)
    sleep(2)
    connection.SimulationConnectH323Party(partyName2)
    sleep(2)
    gwId2 = connection.WaitConfCreated(gwName2, num_retries)

    #Verify there are 2 parties in conf
    print "Verify that there are 2 parties in conf - GW_(00)"
    connection.WaitUntilAllPartiesConnected(gwId2,2,num_retries)			
    
    
    
##     # Create a new undefined party and add it to the EPSim
##     partyName1 = "in1"
##     gwName1="GW_Default_GW_Session(000)"
##     dialString = "2000*123**89*64"
##     connection.SimulationAddH323Party(partyName1, dialString)
##     sleep(2)
##     connection.SimulationConnectH323Party(partyName1)
##     sleep(2)
##     gwId1 = connection.WaitConfCreated(gwName1, num_retries)

##     #Verify there are 4 parties in conf
##     print "Verify that there are 4 parties in conf - GW_(00)"
##     connection.WaitUntilAllPartiesConnected(gwId1,4,num_retries)	
    
##     partyName2 = "in2"
##     gwName2="GW_Default_GW_Session(001)"
##     dialString = "2000*172.22.188.35"
##     connection.SimulationAddH323Party(partyName2, dialString)
##     sleep(2)
##     connection.SimulationConnectH323Party(partyName2)
##     sleep(2)
##     gwId2 = connection.WaitConfCreated(gwName2, num_retries)

##     #Verify there are 2 parties in conf
##     print "Verify that there are 2 parties in conf - GW_(01)"
##     connection.WaitUntilAllPartiesConnected(gwId2,2,num_retries)			
    
##     sleep(2)

##ron    #disconnect "in2" (the dial in party who initiated GW_01 conf
##ron    print "Disconnecting in2  DialIn party"
##ron    connection.SimulationDisconnectH323Party(partyName2)
    
##ron    sleep(2)
##ron    #Verify that GW conf has terminated
##ron    print "Verify that GW_(01) conf has terminated:"
##ron    connection.WaitConfEnd(gwId2,num_retries)
    
    #delete GW_00 conf
##ron    print "delete GW_(00) conf"
##ron    connection.DeleteConf(gwId1)
##ron    sleep(2)
    
##ron    connection.WaitAllConfEnd(num_retries)	
    
##     """
##     sleep(10)
##     # Create a new undefined party and add it to the EPSim
##     partyName = "AdHocParty"
##     phone = "3456"  # the EQ phone 
##     connection.SimulationAddPSTNParty(partyName,phone)
##     sleep(3)
##     connection.SimulationConnectPSTNParty(partyName)
##     sleep(2)
        
##     #Wait untill Eq was awake and the Ad-Hoc conf will be created
##     numOfParties = 1
##     eqConfId = connection.WaitUntillEQorMRAwakes(eqName,numOfParties,num_retries,True)
##     sleep(2)

##     #Send the DTMF which represent the new Ad-hoc conf
##     #This will create a new conf with a new dial out party
##     adHocConfNumericId = "1515"
##     adHocConfName = "GW_1515(12312301)"
##     print "sending DTMF: " + adHocConfNumericId
##     connection.SimulationH323PartyDTMF(partyName, adHocConfNumericId)
##     sleep(2)

##     #Wait untill the target conf will be created
##     adHocConfId = connection.WaitConfCreated(adHocConfName,num_retries)
##     sleep(2)
    
##     #Verify there are 2 parties in conf
##     print "Verify that there are 2 parties in conf"
##     connection.WaitUntilAllPartiesConnected(adHocConfId,2,num_retries)
    
    
##     sleep(2)
##     #delete the Sim Party
##     print "Disconnecting the PSTN DialIn party"
##     connection.SimulationDisconnectPSTNParty(partyName)
    
##     #Verify that GW conf has terminated
##     print "Verify that GW conf has terminated:"
##     connection.WaitConfEnd(adHocConfId,num_retries)
## #    sleep(2)
    
##     #delete the eqt conf
##     connection.DeleteConf(eqConfId)

##     connection.WaitAllConfEnd(num_retries)

## #    connection.WaitConfEnd(gwId1,num_retries)	
## #    sleep(2)
    
##     #remove the EQ Resrv
##     print "Remove the EQ reservation..."
##     connection.DelReservation(eqId, "Scripts/AdHocConfWithUndefParty1/RemoveEq.xml")
    
##     #remove the profile
##     connection.DelProfile(ProfId, "Scripts/AdHocConfWithUndefParty1/RemoveNewProfile.xml")
        
    return
##    """


## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

CreateGW(c,30)# retries

c.Disconnect()
