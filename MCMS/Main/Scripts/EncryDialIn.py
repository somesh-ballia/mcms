#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_10

#############################################################################
# Test Script which is checking conference with encrypted dial in parties
#
# Date: 14/02/05
# By  : Udi B.
#############################################################################

from McmsConnection import *
import os

## ----------------------  --------------------------
def TestEncryConf(connection,numOfParties,num_retries):
    
    valgrindProcess =  os.getenv('PROCESS_UNDER_VALGRIND')
    print "Process Under valgrind is: " +str(valgrindProcess)
    needSleep = False
    if str(valgrindProcess) == 'EncryptionKeyServer':
        needSleep =True
        print "EncryptionKeyServer is under valgrind we need to sleep...." 
        sleep(180)
        print "Script wake up..."
        
    #add a new profile
    ProfId = connection.AddProfile("profile", "Scripts/EncryDialIn/CreateNewProfile.xml")
    
    #create the target Conf and wait untill it connected
    targetConfName = "EncryptedConf"
    connection.CreateConfFromProfile(targetConfName, ProfId, 'Scripts/EncryDialIn/CreateNewConf.xml')
    confId  = connection.WaitConfCreated(targetConfName,num_retries)

    #Add Encrypted Dial-in defined parties
    connection.LoadXmlFile("Scripts/EncryDialIn/AddDefinedDialInParty.xml")
    for x in range(numOfParties):
        partyname = "Party"+str(x+1)
        partyip =  "123.123.0."+str(x+1)
        print "Adding Dial-in defined Party: "+partyname
        connection.ModifyXml("PARTY","NAME",partyname)
        connection.ModifyXml("PARTY","IP",partyip)
        connection.ModifyXml("ADD_PARTY",'ENCRYPTION_EX','yes')
        connection.ModifyXml("ADD_PARTY","ID",confId)
        connection.Send()
    
    connection.WaitAllPartiesWereAdded(confId,numOfParties,num_retries*numOfParties)

    ConnectXSimulationParties(connection,targetConfName,numOfParties,num_retries)

    connection.WaitAllOngoingConnected(confId,num_retries*numOfParties)

    connection.DeleteConf(confId)
    
    connection.WaitAllConfEnd(num_retries*2)
    
    #remove the profile
    connection.DelProfile(ProfId, "Scripts/EncryDialIn/RemoveNewProfile.xml")
    
    if needSleep :
        print "taking more time to shout down under valgrind..."
        sleep(60)
 
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
TestEncryConf(c,
              3,
              20) # Num of retries


c.Disconnect()
