#!/usr/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
# Test Script which is checking conference with encrypted dial in parties
#
# Date: 12/11/05
# By  : Udi B.
#############################################################################

from McmsConnection import *
import os

# -----------------------------------------------------
def ReconnectParties(confid):
    connection.LoadXmlFile('Scripts/ReconnectParty/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()
    
    ongoing_parties = connection.xmlResponse.getElementsByTagName("PARTY")
    num_ongoing_parties = len(ongoing_parties)
    print "Reconnecting " + str(num_ongoing_parties) + " parties in conf " + str(confid)
    
    for x in range(num_ongoing_parties):
        partyId = ongoing_parties[x].getElementsByTagName("ID")[0].firstChild.data
        print "Reconnecting party " + str(partyId)
        connection.LoadXmlFile('Scripts/ReconnectParty/TransSetConnect.xml')
        connection.ModifyXml("SET_CONNECT","ID",confid)
        connection.ModifyXml("SET_CONNECT","CONNECT","true")
        connection.ModifyXml("SET_CONNECT","PARTY_ID",partyId)
        connection.Send()
        
# -----------------------------------------------------
        
def TestPSTNConf(connection,confId,numOfParties,num_retries,isRemoteDisconnect,isReconnect):
    print "StartingTestPSTNConf.. "
        
    #Adding PSTN parties:
    if isReconnect == False:
        connection.LoadXmlFile("Scripts/PSTN_Party.xml")
        for x in range(numOfParties):
            partyname = "Party"+str(x+1)
            phone="3333"+str(x+1)
            connection.AddPSTN_DialoutParty(confId,partyname,phone)
    else:
        ReconnectParties(confId)
                
    connection.WaitAllPartiesWereAdded(confId,numOfParties,num_retries*numOfParties)
    connection.WaitAllOngoingConnected(confId)

    print "Sleeping to complete the IVR"
    sleep(7)
    
    print "Disconnect the parties"
    for x in range(numOfParties):
        partyname = "Party"+str(x+1)
        party_id = connection.GetPartyId(confId, partyname)
        if isRemoteDisconnect == False :
            print "Disconnecting from EMA"
            connection.DisconnectParty(confId,party_id)
        else:
            partyname = "DIAL_OUT#1000"+str(x+1)
            print "Disconnecting from Remote"
            connection.SimulationDisconnectPSTNParty(partyname)
            
            
    connection.WaitAllOngoingDisConnected(confId)
    
    

## ---------------------- Test --------------------------
if __name__ == '__main__':
    connection=McmsConnection()
    connection.Connect()
    
    #add a new profile
    ProfId = connection.AddProfile("profile1")
    #create the target Conf and wait untill it connected
    num_retries=20
    targetConfName = "PSTN_Conf"
    connection.CreateConfFromProfile(targetConfName, ProfId)
    confId  = connection.WaitConfCreated(targetConfName,num_retries)

#   TestPSTNConf(connection,
#                confId,
#                80,
#                num_retries,
#                True, #isRemoteDisconnection
#                False)#isReconnect
    TestPSTNConf(connection,
                 confId,
                 80,
                 num_retries,
                 False, #isRemoteDisconnection
                 False)#isReconnect
    connection.DeleteConf(confId)
    connection.WaitConfEnd(confId)
    
    connection.DelProfile(ProfId)
    connection.Disconnect()
