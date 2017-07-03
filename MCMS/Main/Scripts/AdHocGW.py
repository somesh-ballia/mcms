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

def TestAdHocGW(connection,num_retries):
   
    #add a new profile
    ProfId = connection.AddProfile("profile1", "Scripts/AdHocConfWithUndefParty1/CreateNewProfile.xml")

    #send a new EQ service
    print "Adding a new Entry Queue Service...."
    connection.SendXmlFile("Scripts/AdHocConfWithUndefParty1/AddEqService.xml","Status OK")
    
    # send a new Reservation of EntryQueue
    eqName = "AdHocGwTest"
    connection.CreateAdHocEQ(eqName, ProfId,"Scripts/AdHocConfWithUndefParty1/CreateAdHocGWEQ.xml")

    #Wait untill eq was added
    eqId, eqNid = connection.WaitMRCreated(eqName, num_retries)
    
    sleep(10)
    # Create a new undefined party and add it to the EPSim
    partyName = "AdHocParty"
    phone = "3456"  # the EQ phone 
    connection.SimulationAddPSTNParty(partyName,phone)
    sleep(3)
    connection.SimulationConnectPSTNParty(partyName)
        
    #Wait untill Eq was awake and the Ad-Hoc conf will be created
    numOfParties = 1
    eqConfId = connection.WaitUntillEQorMRAwakes(eqName,numOfParties,num_retries,True)

    #Send the DTMF which represent the new Ad-hoc conf
    #This will create a new conf with a new dial out party
    adHocConfNumericId = "1515"
    adHocConfName = "GW_1515(12312301)"
    print "sending DTMF: " + adHocConfNumericId
    connection.SimulationH323PartyDTMF(partyName, adHocConfNumericId)

    #Wait untill the target conf will be created
    adHocConfId = connection.WaitConfCreated(adHocConfName,num_retries)
    
    #Verify there are 2 parties in conf
    print "Verify that there are 2 parties in conf"
    connection.WaitUntilAllPartiesConnected(adHocConfId,2,num_retries)
    
    #delete the Sim Party
    print "Disconnecting the PSTN DialIn party"
    connection.SimulationDisconnectPSTNParty(partyName)
    
    #Verify that GW conf has terminated
    print "Verify that GW conf has terminated:"
    connection.WaitConfEnd(adHocConfId,num_retries)
    
    #delete the eqt conf
    connection.DeleteConf(eqConfId)

    connection.WaitAllConfEnd(num_retries)
    
    #remove the EQ Resrv
    print "Remove the EQ reservation..."
    connection.DelReservation(eqId, "Scripts/AdHocConfWithUndefParty1/RemoveEq.xml")
    
    #remove the profile
    connection.DelProfile(ProfId, "Scripts/AdHocConfWithUndefParty1/RemoveNewProfile.xml")
        
    return


## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestAdHocGW(c,30)# retries

c.Disconnect()