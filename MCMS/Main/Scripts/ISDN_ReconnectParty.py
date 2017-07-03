#!/usr/bin/python

# For list of profiles look at RunTest.sh
#*PROC-disabled-ESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer McuMngr Configurator BackupRestore

#############################################################################
# Test Script is checking if a reconnecting of ISDN participant works fine.
# Date: 02.03.2008
# By  : Olga S.
#############################################################################

from McmsConnection import *
import os

#-----------------------------------------------------------------------------
def AddISDN_DialoutParty(connection,confId,partyName,phone):
    print "Adding ISDN dilaout party: "+partyName+ ", from EMA with phone: " + phone
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

# -----------------------------------------------------

def TestReconnectParty(connection,confId,num_of_parties,num_retries):
    print "Starting Test ISDN Conf..."
        
    #Add and connect ISDN parties:
    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    for x in range(num_of_parties):
        partyname = "Party"+str(x+1)
        phone="3333"+str(x+1)
        AddISDN_DialoutParty(connection,confId,partyname,phone)
	sleep(1)
                
    connection.WaitAllPartiesWereAdded(confId,num_of_parties,num_retries*num_of_parties)
    connection.WaitAllOngoingConnected(confId,num_retries*num_of_parties)

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
        print "Disconnecting party [" + party_id + "] from EMA"
        connection.DisconnectParty(confId,party_id)
	sleep(1)

    connection.WaitAllOngoingDisConnected(confId)
    
    ReconnectParties(connection,confId)
    connection.WaitAllOngoingConnected(confId,num_retries*num_of_parties)

    print "Sleep before delete conference"
    sleep(10)

    connection.DeleteConf(confId)   
    connection.WaitAllConfEnd()
    return

## ---------------------- Test --------------------------

c = McmsConnection()
c.Connect()

#add a new profile
profId = c.AddProfile("profile1")
#create the target Conf and wait untill it connected
targetConfName = "ISDN_Conf"
num_retries=30
c.CreateConfFromProfile(targetConfName, profId)
confId  = c.WaitConfCreated(targetConfName,num_retries)

TestReconnectParty(c,confId,
                   3, #num of participants
                   num_retries)


c.DelProfile(profId)
c.Disconnect()
