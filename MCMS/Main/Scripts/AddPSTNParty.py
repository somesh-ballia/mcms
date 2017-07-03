#!/usr/bin/python

from McmsConnection import *
import os

#-----------------------------------------------------------------------------
def AddPSTNDialoutParty(c,confId,partyName,phone):
    print "Adding PSTN dilaout party: "+partyName+ ", from EMA with phone: " + phone
    c.LoadXmlFile("Scripts/AddPSTNParty/PSTN_party.xml")
    c.ModifyXml("PARTY","NAME",partyName)
    c.ModifyXml("ADD_PARTY","ID",confId)
    c.ModifyXml("PHONE_LIST","PHONE1",phone)
    c.Send(#"STATUS_OUT_OF_RANGE"
            )




def TestPSTNConf(connection,ProfId,numOfParties,num_retries,):
    print "StartingTestPSTNConf.. "
    
    #create the target Conf and wait untill it connected
    targetConfName = "ron1"
    connection.CreateConfFromProfile(targetConfName, ProfId)
    confId  = connection.WaitConfCreated(targetConfName,num_retries)
    sleep(5)
    #Adding PSTN parties:
    connection.LoadXmlFile("Scripts/AddPSTNParty/PSTN_party.xml")
    partyname = "Party1"
    phone="33331"
    AddPSTNDialoutParty(connection,confId,partyname,phone)
#    sleep(1)
        
#    connection.WaitAllPartiesWereAdded(confId,numOfParties,num_retries*numOfParties)
#    connection.WaitAllOngoingConnected(confId)
    sleep(10)
    
    ''' 
    #Adding PSTN parties:
    connection.LoadXmlFile("Scripts/AddPSTNParty/PSTN_party.xml")
    partyname1 = "Party2"
    phone="33332"
    AddPSTNDialoutParty(connection,confId,partyname1,phone)
#    sleep(1)
        
    connection.WaitAllPartiesWereAdded(confId,2,num_retries*numOfParties)
    connection.WaitAllOngoingConnected(confId)
    sleep(2)
    '''
 
    #Add Dial In SIP Party
    partyname = "pstn_22223"
    partyip =  "123.123.0.1"
    partySipAdd = partyname + '@' + partyip
    connection.AddSIPParty(confId, partyname, partyip, partySipAdd, "Scripts/CheckMultiTypesOfCalls/AddDialInSipParty.xml")
    sleep(10)
  
    print "Disconnect the parties"
    party_id = connection.GetPartyId(confId, partyname)
    connection.DisconnectParty(confId,party_id)
    
    '''
    party_id = connection.GetPartyId(confId, partyname1)
    connection.DisconnectParty(confId,party_id)
    '''
             
#    connection.WaitAllOngoingDisConnected(confId)
    sleep(10)
    connection.DeleteConf(confId)
    connection.WaitConfEnd(confId)
    

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()
#add a new profile
ProfId = c.AddProfile("profile1")
    
TestPSTNConf(c,
             ProfId,#ProfileId
             1,
             20) # Num of retries
             
c.DelProfile(ProfId)
c.Disconnect()
 