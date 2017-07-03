#!/usr/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

#############################################################################
# Test Script which is checking conference with encrypted dial in parties
#
# Date: 12/11/05
# By  : Udi B.
#############################################################################

from McmsConnection import *
import os

#-----------------------------------------------------------------------------
def SimulationConnectISDNParty(connection, partyName):
    print "Connect participant from EPsim"
    connection.LoadXmlFile("Scripts/SimConnectEndpoint.xml")
    connection.ModifyXml("PARTY_CONNECT","PARTY_NAME",partyName)
    connection.Send()
#-----------------------------------------------------------------------------
def SimulationAddISDNParty(connection, partyName,phone):
    print "Add participant:" + partyName+", phone="+ phone
    connection.LoadXmlFile('Scripts/SimAddIsdnEP.xml')
    connection.ModifyXml("ISDN_PARTY_ADD","PARTY_NAME",partyName)
    connection.ModifyXml("ISDN_PARTY_ADD","PHONE_NUMBER",phone)
    connection.Send()

#-----------------------------------------------------------------------------
def AddISDN_DialoutParty(connection,confId,partyName,phone):
    print "Adding ISDN dilaout party: "+partyName+ ", from EMA with phone: " + phone
    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    connection.ModifyXml("PARTY","NAME",partyName)
    connection.ModifyXml("ADD_PARTY","ID",confId)
    connection.ModifyXml("PHONE_LIST","PHONE1",phone)
    connection.Send()

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
#------------------------------------------------------------------------------
def WaitPartyDisconnected(connection,confid,partyname,num_retries=30):
    print "Check if the party " + partyname + " is disconnected"
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()
    partyid=connection.GetPartyId(confid,partyname)
    wanted_status = "disconnected"
    for retry in range(num_retries+1):
        ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
        ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        i = 0
        for party in ongoing_parties:
            status = party.getElementsByTagName("DESCRIPTION")[0].firstChild.data
            if(partyid == ongoing_party_list[i].getElementsByTagName("ID")[0].firstChild.data):
                if status != "disconnected":
                    if (retry == num_retries):
                        print "party is connected : " + partyname
			return 0
                break
            i=i+1
        sys.stdout.write(".")
        sys.stdout.flush()
        if status == wanted_status:
            print "party is disconnected : " + partyname
            break
        connection.Send()
        sleep(1)
    return 1

# -----------------------------------------------------
def AddOneDialoutPartyISDN(connection,confId,numOfParty):
    partyname = "Party"+str(numOfParty+1)
    phone="3333"+str(numOfParty+1)
    AddISDN_DialoutParty(connection,confId,partyname,phone)
    return partyname

# -----------------------------------------------------
def TestISDNConf(connection,confId,numOfParties,num_retries,isRemoteDisconnect,isReconnect):
    print "Starting Test ISDN Conf..."
        
    #Adding ISDN parties:
    if isReconnect == False:
        connection.LoadXmlFile("Scripts/ISDN_Party.xml")
        for x in range(numOfParties):
            partyname = "Party"+str(x+1)
            phone="3333"+str(x+1)
            AddISDN_DialoutParty(connection,confId,partyname,phone)
	    sleep(3)
    else:
        ReconnectParties(confId)
                
    connection.WaitAllPartiesWereAdded(confId, numOfParties, num_retries*numOfParties)
    connection.WaitAllOngoingConnected(confId, numOfParties*num_retries)

    print "Sleeping to complete the IVR"
    sleep(5)

    #add one extra party
    print "Add one extra party"
    partyname = AddOneDialoutPartyISDN(connection,confId,numOfParties)

    #check if this extra party is disconnected
    is_disconnected = WaitPartyDisconnected(connection,confId,partyname,3)

    #if it does than delete one connected party and try to add it again
    if is_disconnected == 1:
        partyname = "Party"+str(1)
        party_id = connection.GetPartyId(confId, partyname)
	print "Disconnecting from EMA a first party"
	connection.DisconnectParty(confId,party_id)
	sleep(2)
	#add one extra party
        AddOneDialoutPartyISDN(connection,confId,numOfParties+1)
    else:
	print "Warning: Extra party was connected !!!"

    print "Sleep before disconnecting"
    sleep(10)

    print "Disconnect all parties"
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confId)
    connection.Send()
    ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    num_of_parties = len(ongoing_party_list)
    for index in range(num_of_parties):    
        party_id = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data
        if isRemoteDisconnect == False :
            print "Disconnecting party [" + party_id + "] from EMA"
            connection.DisconnectParty(confId,party_id)
	    sleep(1)
        else:
            partyname = "DIAL_OUT#1000"+str(x+1)
            print "Disconnecting from Remote"
            connection.SimulationDisconnectPSTNParty(partyname)
            
            
    connection.WaitAllOngoingDisConnected(confId,num_retries*numOfParties)
    
    

## ---------------------- Test --------------------------
if __name__ == '__main__':
    connection=McmsConnection()
    connection.Connect()
    
    #add a new profile
    ProfId = connection.AddProfile("profile1")
    #create the target Conf and wait untill it connected
    num_retries=30
    targetConfName = "ISDN_Conf"
    connection.CreateConfFromProfile(targetConfName, ProfId)
    confId  = connection.WaitConfCreated(targetConfName,num_retries)

#   TestISDNConf(connection,
#                confId,
#                80,
#                num_retries,
#                True, #isRemoteDisconnection
#                False)#isReconnect
    TestISDNConf(connection,
                 confId,
                 36,
                 num_retries,
                 False, #isRemoteDisconnection
                 False)#isReconnect
    connection.DeleteConf(confId)
    connection.WaitConfEnd(confId)
    
    connection.DelProfile(ProfId)
    connection.Disconnect()
