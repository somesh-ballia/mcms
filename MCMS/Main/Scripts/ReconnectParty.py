#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#-LONG_SCRIPT_TYPE
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

#############################################################################
# Test Script which is checking if a reconnecting of a participant works fine.
#
# Date: 4/01/05
# By  : Udi B.
# re_write date: 24/09/13
# re_write by: Uri A.
#############################################################################

from McmsConnection import *

def TestReconnectParty(connection,num_of_parties,num_retries):
    confFile = 'Scripts/AddConf.xml'
    partyFile = 'Scripts/AddVoipParty1.xml'
    
    connection.SimpleXmlConfPartyTest(confFile,partyFile,num_of_parties,num_retries,"false")

    #Get ConfId
    connection.SendXmlFile('Scripts/ReconnectParty/TransConfList.xml',"Status OK")
    confid = connection.GetTextUnder("CONF_SUMMARY","ID")
    if confid == "":
        connection.Disconnect()                
        sys.exit("Can not monitor conf:" + status)

    delayBetweenParticipants = 1
    if(c.IsProcessUnderValgrind("ConfParty")):
        delayBetweenParticipants = 2
    print 'delay for SIP parties will be: ' + str(2*delayBetweenParticipants)

    startFrom = 4
    # adding 2 dial out H323 video calls
    AddDialOutH323Calls(c, confid, delayBetweenParticipants, startFrom, "H323VideoParty", "Scripts/CheckMultiTypesOfCalls/AddDialOutH323Party.xml")
    
    startFrom = 6
    # adding 3 dial out SIP audio calls
    AddDialOutSipCalls(c, confid, 2*delayBetweenParticipants, startFrom, "SipAudioParty", "Scripts/CheckMultiTypesOfCalls/AddDialOutSipAudioParty.xml")

    startFrom = 9
    # adding 3 dial out Sip video calls
    AddDialOutSipCalls(c, confid, 2*delayBetweenParticipants, startFrom, "SipVideoParty", "Scripts/CheckMultiTypesOfCalls/AddDialOutSipParty.xml")
    
    sleep(2)
       
    # Disconnecting all parties
    ConnectDisconnectAllParties(connection,confid,"false")
    connection.WaitAllOngoingDisConnected(confid)

    #ReconnectAllParties
    ConnectDisconnectAllParties(connection,confid,"true")
    connection.WaitAllOngoingConnected(confid,num_retries)
    
    connection.DeleteConf(confid)   
    connection.WaitAllConfEnd()
    return


#------------------------------------------------------------------------------
def AddDialOutH323Calls(c, confid, delay, startFrom, partyPrefix, xmlFile):
    print 'adding 2 dial out H323 video calls'     
    for x in range(2):
        partyname = partyPrefix + str(x + startFrom) 
        partyip =  "1.2.3." + str(x + startFrom)
        c.AddParty(confid, partyname, partyip, xmlFile)
        sleep(delay)

#------------------------------------------------------------------------------
def AddDialOutSipCalls(c, confid, delay, startFrom, partyPrefix, xmlFile):
    print 'adding 3 dial out Sip video calls'
    for x in range(3):
        partyname = partyPrefix + str(x + startFrom) 
        partyip =  "1.2.3." + str(x + startFrom)
        partySipAdd = partyname + '@' + partyip
        c.AddSIPParty(confid, partyname, partyip, partySipAdd, xmlFile)
        sleep(delay)

#------------------------------------------------------------------------------
def ConnectDisconnectAllParties(connection,confid,connectVal):
    if connectVal == "true":
        msgStr = "Connecting"
    else:
        msgStr = "Disconnecting"

    connection.LoadXmlFile('Scripts/ReconnectParty/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()
    
    ongoing_parties = connection.xmlResponse.getElementsByTagName("PARTY")
    num_ongoing_parties = len(ongoing_parties)
    print msgStr + " " + str(num_ongoing_parties) + " parties in conf " + str(confid)
    
    for x in range(num_ongoing_parties):
        partyId = ongoing_parties[x].getElementsByTagName("ID")[0].firstChild.data
        print msgStr + " party " + str(partyId)
        connection.LoadXmlFile('Scripts/ReconnectParty/TransSetConnect.xml')
        connection.ModifyXml("SET_CONNECT","ID",confid)
        connection.ModifyXml("SET_CONNECT","CONNECT",connectVal)
        connection.ModifyXml("SET_CONNECT","PARTY_ID",partyId)
        connection.Send()
    return

#------------------------------------------------------------------------------


## ---------------------- Test --------------------------
if __name__ == '__main__':
	c = McmsConnection()
	c.Connect()
	TestReconnectParty(c,
	                   3,
	                   20) # Num of retries
	
	
	c.Disconnect()
