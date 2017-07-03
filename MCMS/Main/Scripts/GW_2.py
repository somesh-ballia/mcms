#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

#############################################################################
# Test Script which test the Awake of an Ad-Hoc Eq by an undefined party (PSTN)
# and creates GW session
#  
# Date: 02/11/08
# By  : Eitan P.
#############################################################################

from McmsConnection import *
from ISDNFunctions import *

def CreateGW(connection,num_retries):
    
    connection.SendXmlFile('Scripts/UpdateIsdnDialInNum.xml')
    # Create a new undefined party and add it to the EPSim
    partyName = "IsdnIn1"
    gwNamePrefix="GW_Default_GW_Session"
    dialString = "3000"
    SimulationAddIsdnParty(connection,partyName, dialString)
  
    SimulationConnectIsdnParty(connection,partyName)
    gwId1 = connection.WaitConfCreatedByNamePrefix(gwNamePrefix, num_retries)
    connection.WaitUntilAllPartiesConnected(gwId1,1, num_retries)
    #sleep(3)
    #dialInPartyId = connection.GetPartyId(gwId1, partyName)
    #connection.WaitPartyConnected(gwId1,dialInPartyId, num_retries)
    #sleep(5)
    
    print "send the DTMF to the EPsim with the ip of the ip party"
    H323Ip = "1*2*3*4"
    connection.SimulationH323PartyDTMF(partyName, H323Ip)

    #Verify there are 2 parties in conf
    print "Verify that there are 2 parties in conf - GW_Default_GW_Session(000)"
    connection.WaitUntilAllPartiesConnected(gwId1,2,num_retries)		
    
    sleep(2)

    #disconnect "in2" (the dial in party who initiated GW_01 conf
    print "Disconnecting Isdn dial in party (from simulation)"
    connection.SimulationDisconnectPSTNParty(partyName)	

    #Verify that GW conf has terminated
    print "Verify that GW_Default_GW_Session(00X) conf has terminated:"
    connection.WaitConfEnd(gwId1,num_retries)
    
    connection.WaitAllConfEnd(num_retries)

    connection.DeletePSTNPartyFromSimulation(partyName)
    
## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

CreateGW(c,30)# retries

c.Disconnect()	
