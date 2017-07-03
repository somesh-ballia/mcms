#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROC-disabled-ESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

#############################################################################
# Test Script which is checking conference with encrypted parties
#
# Date: 23/01/05
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
      
    delayBetweenParticipants = 1
    IsConfPartyUnderValgrind = connection.IsProcessUnderValgrind("ConfParty")
    if(IsConfPartyUnderValgrind):
    	delayBetweenParticipants = 10
    	    
    #add a new profile
    ProfId = connection.AddProfile("profile", "Scripts/EncryConf/CreateNewProfile.xml")
    
    #create the target Conf and wait untill it connected
    targetConfName = "EncryptedConf"
    
    connection.CreateConfFromProfile(targetConfName, ProfId, 'Scripts/EncryConf/CreateNewConf.xml')
    confId  = connection.WaitConfCreated(targetConfName,num_retries)
    
    for x in range(numOfParties):
            partyname = "Party" + str(x+1)
            partyip =  "1.2.3." + str(x+1)
            print "Adding Party " + partyname + ", with ip= " + partyip
            connection.AddParty(confId, partyname, partyip, 'Scripts/AddVideoParty1.xml')
            if(IsConfPartyUnderValgrind):
               connection.WaitAllPartiesWereAdded(confId,x+1,num_retries*(x+1))
               connection.WaitAllOngoingConnected(confId,num_retries*numOfParties,delayBetweenParticipants)

    connection.WaitAllPartiesWereAdded(confId,numOfParties,num_retries*numOfParties)
    connection.WaitAllOngoingConnected(confId,num_retries*numOfParties,delayBetweenParticipants)

    connection.DeleteConf(confId)
    
    connection.WaitAllConfEnd(num_retries*2)
    
    #remove the profile
    connection.DelProfile(ProfId, "Scripts/EncryConf/RemoveNewProfile.xml")

    if needSleep :
        print "taking more time to shout down under valgrind..."
        sleep(60)
  

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()
TestEncryConf(c,
              3,
              20) # Num of retries


c.Disconnect()
