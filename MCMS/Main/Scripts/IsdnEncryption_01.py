#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#*PROCESSES_NOT_FOR_VALGRIND=ConfParty

#############################################################################
# Test Script which is checking conference with encrypted parties
#
# Date: 23/01/05
# By  : Udi B.
#############################################################################

from McmsConnection import *
import os

def AddISDN_DialoutParty(connection,confId,partyName,phone):
    print "Adding ISDN dilaout party: "+partyName+ ", from EMA with phone: " + phone
    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    connection.ModifyXml("PARTY","NAME",partyName)
    connection.ModifyXml("ADD_PARTY","ID",confId)
#    connection.ModifyXml("PARTY","NET_CHANNEL_NUMBER","12")
    connection.ModifyXml("PHONE_LIST","PHONE1",phone)
    connection.Send()


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

    ## for EncryptionKeyServer to complete keys
    sleep(30)

    ##needSleep =True    	    
    #add a new profile
    ProfId = connection.AddProfile("profile", "Scripts/EncryConf/CreateNewProfile.xml")
    
    #create the target Conf and wait untill it connected
    targetConfName = "EncryptedConf"
    
    connection.CreateConfFromProfile(targetConfName, ProfId, 'Scripts/EncryConf/CreateNewConf.xml')
    confId  = connection.WaitConfCreated(targetConfName,num_retries)



    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    partyname = "PartyIsdnEncryption_01"
    phone="654321"
    AddISDN_DialoutParty(connection,confId,partyname,phone)

    connection.WaitAllPartiesWereAdded(confId,1,num_retries)
    if(IsConfPartyUnderValgrind):
        sleep(60)
    connection.WaitAllOngoingConnected(confId)

    if(IsConfPartyUnderValgrind):
        sleep(15)
    print "Disconnecting from EMA"
    party_id = connection.GetPartyId(confId, partyname)
    connection.DisconnectParty(confId,party_id)
    sleep(1)
                
    connection.WaitAllOngoingDisConnected(confId,num_retries)                 
    sleep(2)
 
    connection.DeleteConf(confId)
    connection.WaitAllConfEnd(num_retries*2)
    
    ## remove the profile
    connection.DelProfile(ProfId, "Scripts/EncryConf/RemoveNewProfile.xml")
    sleep(2)


##     if needSleep :
##         print "taking more time to shout down under valgrind..."
##         sleep(60)
    
    
##     for y in range(numOfLoops):        
##         print "Add Isdn parties loop number " + str(y+1)     
##         #Adding ISDN parties:
##         connection.LoadXmlFile("Scripts/ISDN_Party.xml")
##         for x in range(numOfISDNParties):
##             partyname = "Party"+str(x+1+y*numOfISDNParties)
##             phone="3333"+str(x+1+y*numOfISDNParties)
##             AddISDN_DialoutParty(connection,confId,partyname,phone) 
##             sleep(1)
                    
##         connection.WaitAllPartiesWereAdded(confId,numOfISDNParties,num_retries*numOfISDNParties)
##         sleep(2)
##         connection.WaitAllOngoingConnected(confId)
        
##         print "Sleeping to complete the IVR"
##         sleep(15)
     
           
##         print "Disconnect the parties"
##         for x in range(numOfISDNParties):
##             partyname = "Party"+str(x+1+y*numOfISDNParties)
##             party_id = connection.GetPartyId(confId, partyname)
##             print "Disconnecting from EMA"
##             connection.DisconnectParty(confId,party_id)
##             sleep(1)
                
##         connection.WaitAllOngoingDisConnected(confId,num_retries*numOfISDNParties)        
         
##         sleep(2)
 


    
##     for x in range(numOfParties):
##             partyname = "Party" + str(x+1)
##             partyip =  "1.2.3." + str(x+1)
##             print "Adding Party " + partyname + ", with ip= " + partyip
##             connection.AddParty(confId, partyname, partyip, 'Scripts/AddVideoParty1.xml')
##             if(IsConfPartyUnderValgrind):
##                connection.WaitAllPartiesWereAdded(confId,x+1,num_retries*(x+1))
##                connection.WaitAllOngoingConnected(confId,num_retries*numOfParties,delayBetweenParticipants)

##     connection.WaitAllPartiesWereAdded(confId,numOfParties,num_retries*numOfParties)
##     connection.WaitAllOngoingConnected(confId,num_retries*numOfParties,delayBetweenParticipants)

##     connection.DeleteConf(confId)
    
##     connection.WaitAllConfEnd(num_retries*2)
    
##     #remove the profile
##     connection.DelProfile(ProfId, "Scripts/EncryConf/RemoveNewProfile.xml")

##     if needSleep :
##         print "taking more time to shout down under valgrind..."
##         sleep(60)
  

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()
TestEncryConf(c,
              3,
              20) # Num of retries


c.Disconnect()
