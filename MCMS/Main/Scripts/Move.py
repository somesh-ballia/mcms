#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
# Test Script which test the AirTel Move party scenario from one conf 
# to another one,  and to and back operator conference
#  
# Date: 31/05/09
# By  : Romem.
#############################################################################

from McmsConnection import *
from ISDNFunctions  import *

def TestSimpleMoveBetween2Conf(connection,num_retries):
    #add a new profile
    profId = connection.AddProfile("profile", "Scripts/MoveUndef/CreateNewProfile.xml")
    num_retries = 3
    numOfIpParties = 3
    numOfIsdnParties = 3
    numOfSipParties = 3

  #create the source Conf and wait untill it connects   
    sourceConfName = "sourceConf"
    connection.CreateConfFromProfile(sourceConfName,profId)
    sourceConfID  = connection.WaitConfCreated(sourceConfName,num_retries)  
    AddH323PartiesDialOut(sourceConfID, sourceConfName,numOfIpParties,num_retries,expected_status="Status OK")
    
    #create the target Conf and wait untill it connected
    targetConfName = "targetConf"
    connection.CreateConfFromProfile(targetConfName,profId)
    targetConfID  = connection.WaitConfCreated(targetConfName,num_retries)
    AddH323PartiesDialOut(targetConfID, targetConfName,numOfIpParties,num_retries,expected_status="Status OK")

    for numParties in range(numOfIpParties):
        #Get the party id
        currPartyID = connection.GetCurrPartyID(sourceConfID,numOfIpParties-numParties-1,num_retries)
        movedPartyName = GetPartyNameByPartyId(connection,sourceConfID, currPartyID)
        MoveParty(connection,currPartyID,sourceConfID,targetConfID)
        sleep(2)
        WaitUntilPartyConnected(connection,targetConfID,movedPartyName,num_retries*numOfIpParties)
    # Add ISDN parties and move them to target conf
    ConnectIsdnDialOutParties(connection,sourceConfID,numOfIsdnParties)
    sleep(2)
    MoveIsdnParties(connection,sourceConfID,targetConfID,num_retries)
    sleep(2)
     # Add SIP parties and move them to target conf
    AddSipPartiesDialOut(sourceConfID, sourceConfName,numOfSipParties,num_retries,expected_status="Status OK")
    sleep(2)
    MoveSipParties(connection,sourceConfID,targetConfID,num_retries)

    sleep(2)
    

     #delete the source conf
    connection.DeleteConf(sourceConfID)
    #delete the target conf
    connection.DeleteConf(targetConfID)
    
    
    #remove the profile
    connection.DelProfile(profId, "Scripts/MoveUndef/RemoveNewProfile.xml")
    return

#------------------------------------------------------------------------------
def WaitUntilPartyConnected(connection,confid,partyName,num_retries):
    print "Wait untill party will move to target conf...",
    connection.LoadXmlFile('Scripts/MoveUndef/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    print "Moved party name is: " + str(partyName)
    for retry in range(num_retries+1):
        connection.Send()
        ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        num_of_party_items_in_list =  len(ongoing_party_list)
        for partyItem in range(num_of_party_items_in_list):
            partyNameFromList = ongoing_party_list[partyItem].getElementsByTagName("NAME")[0].firstChild.data
            if(partyName == partyNameFromList):
               print
               print "Party: "+ partyName + " Was moved to the target conf" 
               return
        if (retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not find moved Party in the target conf")
        sys.stdout.write(".")
        sys.stdout.flush()
        sleep(1)         
    return 
#------------------------------------------------------------------------------
def GetPartyNameByPartyId(connection,confid, partyId):
    """Monitor party in conference, and return it's name.
        
    confid - target conference id
    partyId - id of the party to monitor
        """
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()
    ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    print "the num of ongoing parties are: " + str(len(ongoing_party_list))
    print "GetPartyNameByPartyId, ConfId: " +str(confid) + " PartyId: " +str(partyId)
    
    if (len(ongoing_party_list) == 0):
        ScriptAbort("the party was not connected...")
        return none
    else:  
        for index in range(len(ongoing_party_list)):
            listPartyId = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data
			  #ongoing_party_list[i].getElementsByTagName("ID")[0].firstChild.data
            print "list party Id: " + str(listPartyId) + " Required party id: " + str(partyId) 
            if(str(partyId) == listPartyId):
                partyMovedName = ongoing_party_list[index].getElementsByTagName("NAME")[0].firstChild.data
                print "Moved party is: " + "Party Name with ID: " + str(partyId) +  "is: " + partyMovedName
                return partyMovedName
    return -1
#------------------------------------------------------------------------------
def MoveParty(connection,partyId,sourceConfId,targetConfId):
     print "Move Party to Conference. " + "Source Conf Id: " + str(sourceConfId) + " Target Conf Id: " + " Party ID: " + str(partyId)
     connection.LoadXmlFile('Scripts/AirTelMove/MoveRegular.xml')
     connection.ModifyXml("MOVE_PARTY","ID",sourceConfId)
     connection.ModifyXml("MOVE_PARTY","TARGET_ID",targetConfId)
     connection.ModifyXml("MOVE_PARTY","PARTY_ID",partyId)
     connection.Send()
     return
    
#------------------------------------------------------------------------------
def AddH323PartiesDialOut(confid, confName,num_of_parties,num_retries,expected_status="Status OK"):
    	 for x in range(num_of_parties):
            partyname = confName+"Party" + str(x+1)
            partyip =  "1.2.3." + str(x+1) 
            print "Adding Party " + partyname + ", with ip= " + partyip
            c.AddParty(confid, partyname, partyip, 'Scripts/AddVideoParty1.xml', expected_status)
       	    sleep(4)        
 
#------------------------------------------------------------------------------
def AddSipPartiesDialOut(confid, confName,num_of_parties,num_retries,expected_status="Status OK"):
    	 for x in range(num_of_parties):
            partyname = confName+"SipParty" + str(x+1)
            partyip =  "1.2.3." + str(x+1) 
            print "Adding Party " + partyname + ", with ip= " + partyip
            c.AddParty(confid, partyname, partyip, 'Scripts/CreateConfWith3SipParticipants/AddSipParty.xml', expected_status)
       	    sleep(4)        
 
#------------------------------------------------------------------------------
def ConnectIsdnDialOutParties(connection,confId,numOfIsdnParties):
    partyname_out = "Isdn"
    phoneout="333"
    chnlNum="channel_6"
    for i in range(numOfIsdnParties):
       phone = phoneout + str(i)
       partyName = partyname_out + str(i)       
       print "Add Isdn Dial Out party, Name: " + partyName
       AddIsdnDialoutParty(connection,confId,partyName,phone,chnlNum)
       sleep(6)
#------------------------------------------------------------------------------
def MoveIsdnParties(connection,sourceConfID,targetConfID,num_retries):
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",sourceConfID)
    connection.Send()
    ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    
    if (len(ongoing_party_list) == 0):
        ScriptAbort("the party was not connected...")
        return 
    else:  
        for index in range(len(ongoing_party_list)):
            interfaceType = ongoing_party_list[index].getElementsByTagName("INTERFACE")[0].firstChild.data
            if(interfaceType == "isdn"):
                listPartyId = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data
                partyMovedName = ongoing_party_list[index].getElementsByTagName("NAME")[0].firstChild.data
                print "Moved ISDN party is: " + "Party Name with ID: " + str(listPartyId) +  "is: " + partyMovedName
                MoveParty(connection,listPartyId,sourceConfID,targetConfID)
                sleep(2)
                WaitUntilPartyConnected(connection,targetConfID,partyMovedName,num_retries)

#------------------------------------------------------------------------------
def MoveSipParties(connection,sourceConfID,targetConfID,num_retries):
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",sourceConfID)
    connection.Send()
    ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    
    if (len(ongoing_party_list) == 0):
        ScriptAbort("the party was not connected...")
        return 
    else:  
        for index in range(len(ongoing_party_list)):
            interfaceType = ongoing_party_list[index].getElementsByTagName("INTERFACE")[0].firstChild.data
            if(interfaceType == "sip"):
                listPartyId = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data
                partyMovedName = ongoing_party_list[index].getElementsByTagName("NAME")[0].firstChild.data
                print "Moved Sip party is: " + "Party Name with ID: " + str(listPartyId) +  "is: " + partyMovedName
                MoveParty(connection,listPartyId,sourceConfID,targetConfID)
                sleep(2)
                WaitUntilPartyConnected(connection,targetConfID,partyMovedName,num_retries)
    
    
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

TestSimpleMoveBetween2Conf(c,30)# retries

c.Disconnect()


