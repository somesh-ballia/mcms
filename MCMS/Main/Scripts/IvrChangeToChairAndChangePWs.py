#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
# Test Script for IVR entrance features 
#
# Date: 
# By  : 
#############################################################################


from McmsConnection import *
import string

#------------------------------------------------------------------------------
def TestIvrChangeToChairAndChangePWs(connection, num_retries):
 
    # Update the default IVR service - enable entrance features (Welcome, conf PW & chair PW)
    connection.SendXmlFile('Scripts/IvrChangeToChairAndChangePWs/UpdateIvrServiceWithEntranceFeatures.xml' )
       
    # Create conf
    confName = "IvrConf"
    confPW = "2222"
    chairPW = "3333"
    print "Adding Conf " + confName + " ..."
    connection.LoadXmlFile('Scripts/IvrChangeToChairAndChangePWs/AddCpConfWithPWs.xml' )
    connection.ModifyXml("RESERVATION","NAME",confName)
    connection.ModifyXml("RESERVATION","ENTRY_PASSWORD",confPW)
    connection.ModifyXml("RESERVATION","PASSWORD",chairPW)
    connection.Send()
    
    print "Wait until conf is created...",
    for retry in range(num_retries+1):
        connection.SendXmlFile('Scripts/IvrChangeToChairAndChangePWs/TransConfList.xml',"Status OK")
        confid = connection.GetTextUnder("CONF_SUMMARY","ID")
        if confid != "":
            print
            break
        if (retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not monitor conf")
        sys.stdout.write(".")
        sys.stdout.flush()
    print "Created Conf " + str(confid)

    partyIndex = 0
 
    # Add first party to EP Sim and connect it as regular party
    partyName = "Party"+str(partyIndex+1)
    partyId = AddParty(connection, partyName, partyIndex, confName, confid, num_retries)
    # Send the conf PW via DTMF
    sleep(2)
    print "Sending conference PW - Sending DTMF " + confPW +"#..."
    connection.SimulationH323PartyDTMF(partyName, confPW)    # '#' is appended to the end of the PW string

    WaitXOngoingInConf(connection,confid,partyIndex+1)
    partyIndex = partyIndex + 1

    # Set party as chair
    sleep(5)
    print "Setting party as chair..."
    connection.LoadXmlFile('Scripts/IvrChangeToChairAndChangePWs/SetAsChair.xml' )
    connection.ModifyXml("SET_LEADER","ID",confid)
    connection.ModifyXml("SET_LEADER","PARTY_ID",partyId)
    connection.Send()
 
    # Change chairperson PW via DTMF
    oldChairPW = chairPW
    chairPW = "5555"
    confOrChairPW = "CHAIR_PW"
    print "Changing chairperson PW..."
    ChangePW(connection,partyName,confOrChairPW,chairPW)
    WaitUntilPwChanged(connection,confid,confOrChairPW,chairPW)
 

    # Add second party to EP Sim and connect it as regular party
    partyName = "Party"+str(partyIndex+1)
    AddParty(connection, partyName, partyIndex, confName, confid, num_retries)
    # Send the conf PW via DTMF
    sleep(1)
    print "Sending conference PW - Sending DTMF " + confPW +"#..."
    connection.SimulationH323PartyDTMF(partyName, confPW)    # '#' is appended to the end of the PW string
 
    WaitXOngoingInConf(connection,confid,partyIndex+1)
    partyIndex = partyIndex + 1

    # Change party to chairperson via DTMF
    sleep(5)
    ChangeToChair(connection,partyName,oldChairPW,chairPW)    
    
    # Change conference PW via DTMF
    confPW = "4444"
    confOrChairPW = "CONF_PW"
    print "Changing conference PW..."
    ChangePW(connection,partyName,confOrChairPW,confPW)
    WaitUntilPwChanged(connection,confid,confOrChairPW,confPW)
    
    
    # Add third party to EP Sim and connect it as regular party with PW
    partyName = "Party"+str(partyIndex+1)
    AddParty(connection, partyName, partyIndex, confName, confid, num_retries)
    sleep(1)
    print "Sending the new conference PW - Sending DTMF " + confPW +"#..."
    connection.SimulationH323PartyDTMF(partyName, confPW)   # '#' is appended to the end of the PW string
 
    WaitXOngoingInConf(connection,confid,partyIndex+1)
    partyIndex = partyIndex + 1
    
    sleep(2)
    
    print 
    print "Deleting Parties"
    for x in range(partyIndex):
        connection.DeleteParty(confid,str(x+1))

    connection.WaitUntillPartyDeleted(confid,num_retries*(partyIndex + 1))
    print "Deleting Conf"
    connection.DeleteConf(confid)



#------------------------------------------------------------------------------
def AddParty(connection, partyName, partyIndex, confName, confid, num_retries):
    connection.SimulationAddH323Party(partyName, confName)
    connection.SimulationConnectH323Party(partyName)
    #Get the party id
    currPartyID = connection.GetCurrPartyID(confid,partyIndex,num_retries)
    if (currPartyID < 0):
        connection.Disconnect()                
        sys.exit("Error:Can not find partry id of party: "+partyName)
    print "found party id = "+str(currPartyID)
    
    connection.WaitAllOngoingConnected(confid,num_retries)

    return currPartyID

#------------------------------------------------------------------------------

def WaitXOngoingInConf(self,confid,numOfPartiesInConf,num_retries=30):
    """ Semilar to WaitAllOngoingNotInIVR()
        Monitor conference until X ongoing parties are not in IVR.
    """
    print "Wait until all ongoing parties are no longer in IVR",
    self.LoadXmlFile('Scripts/TransConf2.xml')
    self.ModifyXml("GET","ID",confid)
    self.Send()
    last_num_connected = 0
    for retry in range(num_retries+1):
        ongoing_parties = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
#        num_ongoing_parties = len(ongoing_parties)
        num_connected = 0
        for party in ongoing_parties:
            status = party.getElementsByTagName("ATTENDING_STATE")[0].firstChild.data
            if status != "inconf":
                if (retry == num_retries):
                    print self.xmlResponse.toprettyxml()
                    self.Disconnect()                
                    sys.exit("party not inconf :" + status)
                
            else:
                num_connected=num_connected+1;
        sys.stdout.write(".")
        if last_num_connected != num_connected:
#            con_string =  "["+str(num_connected)+"/"+str(len(ongoing_parties))+"]"
            con_string =  "["+str(num_connected)+"/"+str(numOfPartiesInConf)+"]"
            sys.stdout.write(con_string)
        sys.stdout.flush()                
        last_num_connected = num_connected                        
#        if num_connected == len(ongoing_parties):
        if num_connected == numOfPartiesInConf:
            print
            break
        sleep(1)            
        self.Send() 
    if num_connected != numOfPartiesInConf:
        print self.xmlResponse.toprettyxml()
        self.Disconnect()                
        sys.exit("party not inconf :" + status)




#------------------------------------------------------------------------------

def ChangePW(connection,partyName,confOrChairPW,password):
    sleep(1)
    print "Changing PW - Sending DTMF '*77'..."
    connection.SimulationH323PartyDTMFWithoutDelimiter(partyName, "*77")
    sleep(1)
    if ("CHAIR_PW" == confOrChairPW):
        print "Sending '2' for changing chairperson PW - Sending DTMF '2'..."
        connection.SimulationH323PartyDTMFWithoutDelimiter(partyName, "2")
    else:    # "CONF_PW"
        print "Sending '1' for changing conf PW - Sending DTMF '1'..."
        connection.SimulationH323PartyDTMFWithoutDelimiter(partyName, "1")
         
    # Send the new PW
    sleep(1)
    print "Sending the new PW - Sending DTMF " + password +"#..."
    connection.SimulationH323PartyDTMF(partyName, password)   # '#' is appended to the end of the PW string
    # Send the new PW again for confirmation
    sleep(1)
    print "Sending the new PW again for confirmation - Sending DTMF " + password +"#..."
    connection.SimulationH323PartyDTMF(partyName, password)   # '#' is appended to the end of the PW string
    


#------------------------------------------------------------------------------

def WaitUntilPwChanged(self,confid,confOrChairPW,password,num_retries=30):
    """ Monitor conference until PW is changed
    """
    print "Wait until PW is changed"
    self.LoadXmlFile('Scripts/TransConf2.xml')
    self.ModifyXml("GET","ID",confid)
    self.Send()
    for retry in range(num_retries+1):
        if ("CHAIR_PW" == confOrChairPW):
            currentPW = self.xmlResponse.getElementsByTagName("PASSWORD")[0].firstChild.data
        else:    # "CONF_PW"
            currentPW = self.xmlResponse.getElementsByTagName("ENTRY_PASSWORD")[0].firstChild.data            
        print "."
        if (retry == num_retries):
            print self.xmlResponse.toprettyxml()
            self.Disconnect()                
            sys.exit("PW wasn't changed")
        if (currentPW == password):
            print "PW was changed to: ", currentPW
            break
        sleep(1)            
        self.Send() 



#------------------------------------------------------------------------------

def ChangeToChair(connection,partyName,oldChairPW,chairPW):
    sleep(1)
    print "Changing party to chairperson - Sending DTMF '*78'..."
    connection.SimulationH323PartyDTMFWithoutDelimiter(partyName, "*78")
    sleep(1)
    print "Sending '#' for chairperson services - Sending DTMF '#'..."
    connection.SimulationH323PartyDTMFWithoutDelimiter(partyName, "#")
    # Send the OLD incorrect chairperson PW
    sleep(1)
    print "Sending the original chairperson PW (incorrect) - Sending DTMF " + oldChairPW +"#..."
    connection.SimulationH323PartyDTMF(partyName, oldChairPW)   # '#' is appended to the end of the PW string
    # Send the NEW correct chairperson PW
    sleep(1)
    print "Sending the new chairperson PW (correct) - Sending DTMF " + chairPW +"#..."
    connection.SimulationH323PartyDTMF(partyName, chairPW)   # '#' is appended to the end of the PW string


#------------------------------------------------------------------------------

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestIvrChangeToChairAndChangePWs(c,
                                 20)# retries
sleep(3)
c.Disconnect()


