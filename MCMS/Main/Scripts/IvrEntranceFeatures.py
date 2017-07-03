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
def TestIvrEntranceFeatures(connection, num_retries):

#    # Create new IVR service with Welcome, Conf Password & Chairperson Password features 
#    serviceName = "IvrEntranceService"
#    print "Adding IVR service " + serviceName + " ..."
#    connection.LoadXmlFile('Scripts/IvrEntranceFeatures/AddIVRWithEntranceFeatures.xml')
#    connection.ModifyXml("AV_COMMON","SERVICE_NAME",serviceName)
#    connection.Send()
#
#    # Set as default IVR service
#    print "Set as default IVR service..."
#    connection.LoadXmlFile('Scripts/IvrEntranceFeatures/SetDefaultIvrService.xml' )
#    connection.ModifyXml("SET_DEFAULT","SERVICE_NAME",serviceName)
#    connection.Send()
 
    # Update the default IVR service - enable entrance features (Welcome, conf PW & chair PW)
    connection.SendXmlFile('Scripts/IvrEntranceFeatures/UpdateIvrServiceWithEntranceFeatures.xml' )
       
    # Create conf
    confName = "IvrEntranceConf"
    confPW = "2222"
    chairPW = "3333"
    print "Adding Conf " + confName + " ..."
    connection.LoadXmlFile('Scripts/IvrEntranceFeatures/AddCpConfWithPWs.xml' )
    connection.ModifyXml("RESERVATION","NAME",confName)
    connection.ModifyXml("RESERVATION","ENTRY_PASSWORD",confPW)
    connection.ModifyXml("RESERVATION","PASSWORD",chairPW)
    connection.Send()
    
    print "Wait until conf is created...",
    for retry in range(num_retries+1):
        connection.SendXmlFile('Scripts/IvrEntranceFeatures/TransConfList.xml',"Status OK")
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
    AddParty(connection, partyName, partyIndex, confName, confid, num_retries)
    # Send the conf PW via DTMF
    sleep(1)
    print "Sending DTMF " + confPW +"#..."
    connection.SimulationH323PartyDTMF(partyName, confPW)    # '#' is appended to the end of the PW string

    WaitXOngoingInConf(connection,confid,partyIndex+1)
    partyIndex = partyIndex + 1





     # Add second party to EP Sim and connect it as chairperson
    partyName = "Party"+str(partyIndex+1)
    AddParty(connection, partyName, partyIndex, confName, confid, num_retries)
    # Send a faulty PW via DTMF for three times and then the correct chair PW
    faultyPW = "1234"
    sleep(1)
    print "Sending DTMF " + faultyPW +"#..."
    connection.SimulationH323PartyDTMF(partyName, faultyPW)   # '#' is appended to the end of the PW string
    sleep(1)
    print "Sending DTMF " + faultyPW +"#..."
    connection.SimulationH323PartyDTMF(partyName, faultyPW)   # '#' is appended to the end of the PW string
    sleep(1)
    print "Sending DTMF " + chairPW +"#..."
    connection.SimulationH323PartyDTMF(partyName, chairPW)    # '#' is appended to the end of the PW string
    
    WaitXOngoingInConf(connection,confid,partyIndex+1)
    partyIndex = partyIndex + 1
    
    
    
    

    # Update the default IVR service again - disable conf PW feature and check the Billing Code checkbox 
    connection.SendXmlFile('Scripts/IvrEntranceFeatures/UpdateIvrDisableConfPWCheckBillingCode.xml' )

     # Add third party to EP Sim and connect it as chairperson + billing code
    partyName = "Party"+str(partyIndex+1)
    AddParty(connection, partyName, partyIndex, confName, confid, num_retries)
    # Press '#' for chairperson services and then chair PW
    sleep(1)
    print "Sending DTMF '#'..."
    connection.SimulationH323PartyDTMFWithoutDelimiter(partyName, "#")
    sleep(1)
    print "Sending DTMF " + chairPW +"#..."
    connection.SimulationH323PartyDTMF(partyName, chairPW)   # '#' is appended to the end of the PW string

    # Send billing code by DTMF
    billingCode = "98765"
    sleep(1)
    print "Sending DTMF " + billingCode +"#..."
    connection.SimulationH323PartyDTMF(partyName, billingCode)   # '#' is appended to the end of the PW string
    

    WaitXOngoingInConf(connection,confid,partyIndex+1)
    partyIndex = partyIndex + 1

#    print
#    print "Deleting Parties"
#    for x in range(partyIndex):
#        connection.DeleteParty(confid,str(x))
#
#    connection.WaitUntillPartyDeleted(confid,num_retries*(partyIndex + 1))
#    print "Deleting Conf"
#    connection.DeleteConf(confid)


    print "Deleting Conf by DTMF"
    sleep(1)
    print "Sending DTMF '*87' from Party3 (chairperson)..."
    connection.SimulationH323PartyDTMFWithoutDelimiter("Party3", "*87")
    connection.WaitAllConfEnd()
    return



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

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestIvrEntranceFeatures(c,
                        20)# retries

c.Disconnect()


