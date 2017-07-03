#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
# Test Script which is checking if disonnect of an undefined participant 
# remove the party from the DB
#
# Date: 4/01/05
# By  : Udi B.
#############################################################################

from McmsConnection import *

def TestDisconnectUndefined(connection,num_of_parties,num_retries):
    confFile = 'Scripts/ReconnectParty/AddVoipConf.xml'
    partyFile = 'Scripts/ReconnectParty/AddVoipParty1.xml'
    
    connection.SimpleXmlConfPartyTest(confFile,partyFile,num_of_parties,num_retries,"false")

    #Get ConfId
    connection.SendXmlFile('Scripts/ReconnectParty/TransConfList.xml',"Status OK")
    confid = connection.GetTextUnder("CONF_SUMMARY","ID")
    if confid == "":
        connection.Disconnect()                
        sys.exit("Can not monitor conf:" + status)
        
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

c = McmsConnection()
c.Connect()
TestReconnectParty(c,
                   3,
                   20) # Num of retries


c.Disconnect()
