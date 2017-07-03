#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_3

#############################################################################
# Test Script which test the Awake of an Ad-Hoc Eq by an undefined 323 party
#############################################################################

from McmsConnection import *

def TestAdHocAwakeEqWithUndefPArty(connection,num_retries):
    #add a new profile
    ProfId = connection.AddProfile("profile1")
    #send a new EQ service
    print "Adding a new Entry Queue Service...."
    connection.SendXmlFile("Scripts/AdHocConfWithUndefParty1/AddEqService.xml","Status OK")
    
    # TODO: wait untill the EQ-Service will be added
    
    # send a new Reservation of EntryQueue
    eqName = "eqRsrv"
    connection.CreateAdHocEQ(eqName, ProfId, "Scripts/AdHocConfWithUndefParty1/CreateNewEq.xml")
    #Wait untill eq was added
    connection.WaitMRCreated(eqName,num_retries)
    
    # Create a new undefined party and add it to the EPSim
    partyName = "AdHockParty"
    connection.SimulationAddH323Party(partyName, eqName)
    
    
    #connect the undefined party
    connection.SimulationConnectH323Party(partyName)
    
    #Wait untill Eq was awake and the Ad-Hoc conf will be created
    numOfParties = 1
    eqConfId = connection.WaitUntillEQorMRAwakes(eqName,numOfParties,num_retries,True)
    
    #Send the DTMF which represent the new Ad-hoc conf
    adHocConfNumericId = "1515"
    connection.SimulationH323PartyDTMF(partyName, adHocConfNumericId)
    #Wait untill the target conf will be created
    #adHocConfId = WaitUntillAdHocConfCreated(connection,adHocConfNumericId,num_retries)
    adHocConfId = connection.WaitConfCreated(adHocConfNumericId, num_retries)
    
    connection.SimulationDeleteH323Party(partyName)
    
    #delete the eqt conf
    connection.DeleteConf(eqConfId)
    
    #delete the AdHoc conf
    connection.DeleteConf(adHocConfId)
    
    connection.WaitAllConfEnd(num_retries)
    
    #remove the EQ Resrv
    print "Remove the EQ reservation..."
    connection.SendXmlFile("Scripts/AdHocConfWithUndefParty1/RemoveEq.xml")
    #remove the profile
    connection.DelProfile(ProfId, "Scripts/AdHocConfWithUndefParty1/RemoveNewProfile.xml")
    
    return

#------------------------------------------------------------------------------

        
## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestAdHocAwakeEqWithUndefPArty(c,
                    40)# retries

c.Disconnect()


