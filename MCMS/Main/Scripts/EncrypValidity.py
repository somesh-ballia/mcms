#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#-- EXPECTED_ASSERT(1)=CConfPartyManager::MovePartyToMROOM_OR_CONF - Party-Move-Error Failed, move validation failed, cannot move party, disconnect party

#############################################################################
# Test Script which is making sure that there is no possibility to move a party
# from un-encrypted EQ to an Encrypted on-going conference
#
# Date: 23/01/05
# By  : Udi B.
#############################################################################



from McmsConnection import *
import os

## ----------------------  --------------------------
def TestEncrypValidity(connection,numOfParties,num_retries):
    
    valgrindProcess =  os.getenv('PROCESS_UNDER_VALGRIND')
    print "Process Under valgrind is: " +str(valgrindProcess)
    needSleep = False
    if str(valgrindProcess) == 'EncryptionKeyServer':
        needSleep =True
        print "EncryptionKeyServer is under valgrind we need to sleep...." 
        sleep(180)
        print "Script wake up..."
        
    #add a new profile
    print "Adding new Encrypted Profile..."
    connection.LoadXmlFile("Scripts/EncryConf/CreateNewProfile.xml")
    connection.ModifyXml("RESERVATION",'ENCRYPTION','true')
    connection.Send()
    ProfId = connection.GetTextUnder("RESERVATION","ID")
    my_name = connection.GetTextUnder("RESERVATION","NAME")
    print "Profile, named: " + my_name + " ,ID = " + ProfId + ", is added"

    print "Adding new Un-Encrypted Profile..."
    connection.LoadXmlFile("Scripts/EncryConf/CreateNewProfile.xml")
    connection.ModifyXml("RESERVATION","NAME",'UnEncryptedProf')
    connection.ModifyXml("RESERVATION",'ENCRYPTION','false')
    connection.Send()
    notEncryProfId = connection.GetTextUnder("RESERVATION","ID")
    my_name = connection.GetTextUnder("RESERVATION","NAME")
    
    #send a new EQ service
    print "Adding a new Entry Queue Service...."
    connection.SendXmlFile("Scripts/CreateAdHoc3BlastUndefParties/AddEqService.xml","Status OK")

    # send a new Reservation of EntryQueue
    eqName = "eqRsrv"
    print "Adding a new Entry Queue " + eqName + " Reservation..."
    connection.CreateAdHocEQ(eqName,notEncryProfId,"Scripts/CreateAdHoc3BlastUndefParties/CreateNewEq.xml")

    #Wait untill eq was added
    eqNumericId = ""
    eqId = ""
    eqId,eqNumericId=connection.WaitMRCreated(eqName,num_retries)
    
    #create the target Conf and wait untill it connected
    targetConfName = "EncryptedConf"
    numericId='3333'
    confId  = AddConf(connection,targetConfName,ProfId,numericId,num_retries)

    #Add an un-encrypted party to the encrypted conf
    partyname = "NotEncyParty"
    partyip =  "1.2.3.5" 
    print "Adding Un-Encrypted Party to an encrypted Party..."
    connection.LoadXmlFile('Scripts/AddVideoParty1.xml')
    connection.ModifyXml("PARTY","NAME",partyname)
    connection.ModifyXml("PARTY","IP",partyip)
    connection.ModifyXml("ADD_PARTY",'ENCRYPTION_EX','no')
    connection.ModifyXml("ADD_PARTY","ID",confId)
    connection.Send("Participant encryption settings do not match conference settings")

    #Adding undeined party
    partyName = "UndefParty" 
    print "Adding Sim Undefined Party " + partyName 
    connection.LoadXmlFile("Scripts/SimAdd323Party.xml")
    connection.ModifyXml("H323_PARTY_ADD","PARTY_NAME",partyName)
    connection.ModifyXml("H323_PARTY_ADD","CONF_NAME",eqName)
    connection.Send()
    sleep(1)
    
    #connect the undefined party
    print "Connecting Sim Party " + partyName
    connection.LoadXmlFile("Scripts/SimConnect323Party.xml")
    connection.ModifyXml("H323_PARTY_CONNECT","PARTY_NAME",partyName)
    connection.Send()

    #Wait untill Eq was awake
    num_of_parties=1
    eqConfId = connection.WaitUntillEQorMRAwakes(eqName,num_of_parties,num_retries,True)

    #Send the DTMF for the target conference
    print "Sending DTNF: " +numericId + " tp party: " +partyName
    connection.SimulationH323PartyDTMF(partyName,numericId)

    #Make sure that the party was deleted from the EQ
    connection.WaitUntillPartyDeleted(eqConfId,num_retries)

    connection.DeleteConf(confId)
    connection.DeleteConf(eqConfId)
    connection.WaitAllConfEnd(num_retries*2)

    #remove the EQ Resrv
    print "Remove the EQ reservation..."
    connection.DelReservation(eqId,"Scripts/CreateAdHoc3BlastUndefParties/RemoveEq.xml")
   
    #remove the profile
    print "Deleting profile: " + my_name
    connection.LoadXmlFile("Scripts/EncryConf/RemoveNewProfile.xml")
    connection.ModifyXml("TERMINATE_PROFILE","ID",ProfId)
    connection.Send("Status OK")

    print "Deleting SIM party " + partyName +"..."
    connection.LoadXmlFile("Scripts/CreateAdHoc3BlastUndefParties/SimDel323Party.xml")
    connection.ModifyXml("H323_PARTY_DEL","PARTY_NAME",partyName)
    connection.Send()
        
    if needSleep :
        print "taking more time to shout down under valgrind..."
        sleep(60)

#------------------------------------------------------------------------------
def AddConf(connection,confName,profileID,numericId,num_retries):
    print "Adding Encrypted Conf..."
    connection.LoadXmlFile('Scripts/EncryConf/CreateNewConf.xml')
    connection.ModifyXml("RESERVATION","NAME",confName)             
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profileID)
    connection.ModifyXml("RESERVATION","NUMERIC_ID",numericId)
    connection.Send()
    confid = 0;
    print "Wait untill Conf create...",
    for retry in range(num_retries+1):
        connection.SendXmlFile('Scripts/EncryConf/TransConfList.xml',"Status OK")
        confid = connection.GetTextUnder("CONF_SUMMARY","ID")
        if (confid != "") :
            print
            break
        if (retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not monitor conf:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()
    print "Created Conf " + str(confid)
    return confid


#------------------------------------------------------------------------------
def FindConfInConfList(connection,confList,targetName):
    confId = ""
    for i in range(len(confList)):
        confName = confList[i].getElementsByTagName("NAME")[0].firstChild.data
        confId = confList[i].getElementsByTagName("ID")[0].firstChild.data
        if confName == targetName:
            return targetName,confId
    return targetName+"Error",confId

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()
TestEncrypValidity(c,
              3,
              20) # Num of retries


c.Disconnect()
