#!/usr/bin/python

#*PROC-disabled-ESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

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
    print "Adding ISDN dialout party: "+partyName+ ", from EMA with phone: " + phone
    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    connection.ModifyXml("PARTY","NAME",partyName)
    connection.ModifyXml("ADD_PARTY","ID",confId)
    connection.ModifyXml("PHONE_LIST","PHONE1",phone)
    connection.Send()

# -----------------------------------------------------
def ReconnectParties(connection,confid):
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
        sleep(1)
        
# -----------------------------------------------------
def DisconnectParties(connection, confId, numOfParties, partyName):
    for x in range(numOfParties):
        partyname = partyName+str(x+1)
        party_id = connection.GetPartyId(confId, partyname)
        print "Disconnecting from EMA " + partyname
        connection.DisconnectParty(confId,party_id)
 	sleep(1)

# -----------------------------------------------------

def TestISDNConf(connection,confId,numOfParties,num_retries,isRemoteDisconnect,isReconnect):
    print "Starting Test ISDN Conf... "
    isdnPartyName = "PartyISDN_"
    ipPartyName = "PartyIP_"

    if isReconnect == False:
        #Adding ISDN parties:
        connection.LoadXmlFile("Scripts/ISDN_Party.xml")
        for x in range(numOfParties):
            partyname = isdnPartyName+str(x+1)
            phone="3333"+str(x+1)
            AddISDN_DialoutParty(connection,confId,partyname,phone)
            sleep(1)
   
        #Adding IP parties:
        connection.LoadXmlFile("Scripts/ISDN_Party.xml")
        for x in range(numOfParties):
            partyname = ipPartyName+str(x+1)
	    partyip =  "1.2.3." + str(x+1)
	    connection.AddParty(confId, partyname, partyip,'Scripts/AddVideoParty1.xml')
	    sleep(1)

    else:
        ReconnectParties(connection,confId)
                  
    connection.WaitAllPartiesWereAdded(confId,numOfParties*2,num_retries*numOfParties*2)
    connection.WaitAllOngoingConnected(confId,num_retries*numOfParties*2)
    
    print "Sleeping to complete the IVR"
    sleep(10)


    print "Disconnect the parties"

    if isRemoteDisconnect == False:
    	DisconnectParties(connection, confId, numOfParties, ipPartyName)
    	DisconnectParties(connection, confId, numOfParties, isdnPartyName)
    else:
    	for x in range(numOfParties):
            temp_partyname = "DIAL_OUT#1000"+str(x+1)
            print "Disconnecting from Remote " + temp_partyname
            connection.SimulationDisconnectPSTNParty(temp_partyname)
	    sleep(1)
	    
	for x in range(numOfParties):
            temp_partyname = "DIAL_OUT#1000"+str(x+1+numOfParties)
            print "Disconnecting from Remote " + temp_partyname
	    connection.SimulationDisconnectH323Party(temp_partyname)
	    sleep(1)

    connection.WaitAllOngoingDisConnected(confId, num_retries*numOfParties*2)



## ---------------------- Test --------------------------
if __name__ == '__main__':
    connection=McmsConnection()
    connection.Connect()
    
    #add a new profile
    ProfId = connection.AddProfile("profile1")
    #create the target Conf and wait untill it connected
    num_retries=20
    targetConfName = "ISDN_Conf"
    connection.CreateConfFromProfile(targetConfName, ProfId)
    confId  = connection.WaitConfCreated(targetConfName,num_retries)

    TestISDNConf(connection,
                 confId,
                 3,
                 num_retries,
                 True, #isRemoteDisconnection
                 False)#isReconnect
    TestISDNConf(connection,
                 confId,
                 3,
                 num_retries,
                 False, #isRemoteDisconnection
                 True)#isReconnect

    connection.DeleteConf(confId)
    connection.WaitConfEnd(confId)
    
    connection.DelProfile(ProfId)
    connection.Disconnect()
