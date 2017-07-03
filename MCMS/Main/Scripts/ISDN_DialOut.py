#!/usr/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

#############################################################################
# Test Script which is checking conference with ISDN dial in parties
#
# Date: 15/01/08
# By  : Ron S.
#############################################################################

from McmsConnection import *
import os

#-----------------------------------------------------------------------------
def AddISDN_DialoutParty(connection,confId,partyName,phone):
    print "Adding ISDN dilaout party: "+partyName+ ", from EMA with phone: " + phone
    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    connection.ModifyXml("PARTY","NAME",partyName)
    connection.ModifyXml("ADD_PARTY","ID",confId)
#    connection.ModifyXml("PARTY","NET_CHANNEL_NUMBER","12")
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
        
# -----------------------------------------------------
        
def TestISDNConf(connection,confId,numOfISDNParties,numOfLoops,num_retries):
    print "Starting Test ISDN Conf.. "

    for y in range(numOfLoops):        
        print "Add Isdn parties loop number " + str(y+1)     
        #Adding ISDN parties:
        connection.LoadXmlFile("Scripts/ISDN_Party.xml")
        for x in range(numOfISDNParties):
            partyname = "Party"+str(x+1+y*numOfISDNParties)
            phone="3333"+str(x+1+y*numOfISDNParties)
            AddISDN_DialoutParty(connection,confId,partyname,phone) 
            sleep(1)
                    
        connection.WaitAllPartiesWereAdded(confId,numOfISDNParties,num_retries*numOfISDNParties)
        sleep(2)
        connection.WaitAllOngoingConnected(confId)
        
        print "Sleeping to complete the IVR"
        sleep(15)
     
           
        print "Disconnect the parties"
        for x in range(numOfISDNParties):
            partyname = "Party"+str(x+1+y*numOfISDNParties)
            party_id = connection.GetPartyId(confId, partyname)
            print "Disconnecting from EMA"
            connection.DisconnectParty(confId,party_id)
            sleep(1)
                
        connection.WaitAllOngoingDisConnected(confId,num_retries*numOfISDNParties)        
         
        sleep(2)
        # Check if all parties were added and save their IDs
        num_of_parties = numOfISDNParties
        party_id_list = []
        connection.LoadXmlFile('Scripts/TransConf2.xml')
        connection.ModifyXml("GET","ID",confId)
        connection.Send()
        ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        if len(ongoing_party_list) < num_of_parties:
            sys.exit("some parties are lost...")
        for index in range(num_of_parties):    
            party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)
        
        # delete all parties  
        for x in range(num_of_parties):
            partyname = "Party"+str(x+1+y*numOfISDNParties)
            print "delete party: " + partyname    
            connection.DeleteParty(confId,party_id_list[x])
            sleep(2)
    
        
        sleep(2)
    

## ---------------------- Test --------------------------
if __name__ == '__main__':
    connection=McmsConnection()
    connection.Connect()
    
    #add a new profile
    ProfId = connection.AddProfile("profile1")
    #create the target Conf and wait untill it connected
    num_retries=50
    targetConfName = "ISDN_Conf"
    connection.CreateConfFromProfile(targetConfName, ProfId)
    confId  = connection.WaitConfCreated(targetConfName,num_retries)
    numberOfISDNParties = 3
    numOfLoops = 3
    TestISDNConf(connection,
                 confId,
                 numberOfISDNParties,
                 numOfLoops,
                 num_retries)
    
    connection.DeleteConf(confId)
    connection.WaitConfEnd(confId)
    
    connection.DelProfile(ProfId)
    connection.Disconnect()
