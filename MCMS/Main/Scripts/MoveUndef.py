#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
# Test Script which test the Move party scenario from EQ to target conf
#  
# Date: 16/01/05
# By  : Udi B.
#############################################################################

from McmsConnection import *

def TestAwakeEqWithUndefPArty(connection,num_retries):
    #add a new profile
    profId = connection.AddProfile("profile", "Scripts/MoveUndef/CreateNewProfile.xml")
    
    #send a new EQ service
    print "Adding a new Entry Queue Service...."
    connection.SendXmlFile("Scripts/MoveUndef/AddEqService.xml","Status OK")

    # TODO: wait untill the EQ-Service will be added
    
    # send a new Reservation of EntryQueue
    eqName = "eqRsrv"
    connection.CreateAdHocEQ(eqName, profId, "Scripts/MoveUndef/CreateNewEq.xml")

    #Wait untill eq was added
    connection.WaitMRCreated(eqName,num_retries)
    
    #create the target Conf and wait untill it connected
    targetConfName = "targetConf"
    connection.CreateConfFromProfile(targetConfName,profId,"Scripts/MoveUndef/CreateNewConf.xml")
    targetConfID  = connection.WaitConfCreated(targetConfName,num_retries)
    
    targetConfNumericId = GetConfNumericId(connection,targetConfID)
    print "Target Conf Numeric id is " + str(targetConfNumericId)

    # Create a new undefined party and add it to the EPSim
    partyName = "moveParty"
    connection.SimulationAddH323Party(partyName, eqName)
    #connect the undefined party
    connection.SimulationConnectH323Party(partyName)

    #Wait untill Eq was awake
    numOfParties = 1
    eqConfId = connection.WaitUntillEQorMRAwakes(eqName,numOfParties,num_retries,True)
    
    #send the TDMF to the EPsim with the numeric id of the target conf
    connection.SimulationH323PartyDTMF(partyName, targetConfNumericId)

    #Wait untill the party will be connected in the target conf
    WaitUntilPartyConnected(connection,targetConfID,numOfParties,num_retries*numOfParties)

    
    # Create a new undefined party and add it to the EPSim
    partyName2 = "moveParty2"
    numOfParties = numOfParties + 1
    connection.SimulationAddH323Party(partyName2, eqName)
    #connect the undefined party
    connection.SimulationConnectH323Party(partyName2)

    print "Sleeping for 3 seconds untill party will be connect and IVR will start"
    sleep(3)

    #send the TDMF to the EPsim with the numeric id of the target conf
    connection.SimulationH323PartyDTMF(partyName2, targetConfNumericId)

    #Wait untill the party will be connected in the target conf
    WaitUntilPartyConnected(connection,targetConfID,numOfParties,num_retries*numOfParties)
    
    #Delete the ongoing EQ
    connection.DeleteConf(eqConfId)

    #Wait untill conf deleted 
    connection.WaitConfEnd(eqConfId)
    
    #Create 3 undefined party and add it to the EPSim
    partyName3 = "moveParty3"
    numOfParties = numOfParties + 1
    newEQNumOfParties=1
    connection.SimulationAddH323Party(partyName3, eqName)
    #connect the undefined party
    connection.SimulationConnectH323Party(partyName3)
    
    #Wait untill the second Eq will be awaken
    eqConfId = connection.WaitUntillEQorMRAwakes(eqName,newEQNumOfParties,num_retries,True)

    #send the TDMF to the EPsim with the numeric id of the target conf
    connection.SimulationH323PartyDTMF(partyName3, targetConfNumericId)

    #Wait untill the party will be connected in the target conf
    WaitUntilPartyConnected(connection,targetConfID,numOfParties,num_retries*numOfParties)

    #delete the Sim Party
    connection.SimulationDeleteH323Party(partyName)
    connection.SimulationDeleteH323Party(partyName2)
    connection.SimulationDeleteH323Party(partyName3)
    
    #delete the target conf
    connection.DeleteConf(targetConfID)
    
    #delete the Eq-conf
    connection.DeleteConf(eqConfId)
    
    connection.WaitAllConfEnd(num_retries*2)
    
    #remove the EQ Resrv
    print "Remove the EQ reservation..."
    connection.SendXmlFile("Scripts/MoveUndef/RemoveEq.xml")
    
    #remove the profile
    connection.DelProfile(profId, "Scripts/MoveUndef/RemoveNewProfile.xml")
    return

#------------------------------------------------------------------------------
def WaitUntilPartyConnected(connection,confid,num_of_parties,num_retries):
    print "Wait untill party will move to target conf...",
    connection.LoadXmlFile('Scripts/MoveUndef/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    
    for retry in range(num_retries+1):
        connection.Send()
        ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        if (len(ongoing_party_list) ==  num_of_parties):
            partyId  = ongoing_party_list[0].getElementsByTagName("ID")[0].firstChild.data
            partyName = ongoing_party_list[0].getElementsByTagName("NAME")[0].firstChild.data
            print
            print "Party "+ partyName + " with id: " + str(partyId) +" Was moved to the target conf" 
            break
        if (retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not find moved Party in the target conf")
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)
    return 

#------------------------------------------------------------------------------
def GetConfNumericId(connection,targetConfID):
    connection.LoadXmlFile('Scripts/MoveUndef/TransConf2.xml')
    connection.ModifyXml("GET","ID",targetConfID)
    connection.Send()
    numericId = connection.GetTextUnder("RESERVATION","NUMERIC_ID")
    return numericId

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestAwakeEqWithUndefPArty(c,
                    30)# retries

c.Disconnect()


