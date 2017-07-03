#!/usr/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

from McmsConnection import *
from ISDNFunctions import *

###------------------------------------------------------------------------------

def UpdatePartyVisualNameTest(connection, numRetries):

    confname = "Conf1"
    connection.CreateConf(confname)
    confid = connection.WaitConfCreated(confname,numRetries)
 
    # adding the first participant 
    partyname = "Party1" 
    partyip =  "1.2.3.1"
    connection.AddParty(confid, partyname, partyip, 'Scripts/AddVideoParty.xml')
    
    # adding the first ISDN participant
    isdnpartyname = "IsdnParty1"
    phone="33331"
    print "Adding ISDN Party ("+isdnpartyname+")"
    AddIsdnDialoutParty(connection,confid,isdnpartyname,phone)

    connection.WaitAllPartiesWereAdded(confid,2,numRetries)
    connection.WaitAllOngoingConnected(confid,numRetries)
    
     # Check if party was added and save it's ID
    party_id = connection.GetPartyId(confid, partyname)
    isdn_party_id = connection.GetPartyId(confid, isdnpartyname)
          
    # update ip participant 
    partyname = "Party,1,1" 
    #partyip =  "5.6.7.8"
    print "Updating ip party visual name"
    connection.LoadXmlFile('Scripts/UpdateVisualName/UpdateVisualName.xml')
    connection.ModifyXml("SET_PARTY_VISUAL_NAME","ID", confid)
    connection.ModifyXml("SET_PARTY_VISUAL_NAME","PARTY_ID",party_id)
    connection.ModifyXml("SET_PARTY_VISUAL_NAME","NAME",partyname)
    connection.Send()
    sleep(2)
    
    # update isdn participant 
    isdnpartyname = "Isdn,Par;ty,1;1" 
    #phone =  "333311"
    print "Updating isdn party visual name"
    connection.LoadXmlFile('Scripts/UpdateVisualName/UpdateVisualName.xml')
    connection.ModifyXml("SET_PARTY_VISUAL_NAME", "ID", confid)
    connection.ModifyXml("SET_PARTY_VISUAL_NAME", "PARTY_ID",isdn_party_id)
    connection.ModifyXml("SET_PARTY_VISUAL_NAME", "NAME",isdnpartyname)
    connection.Send()
    sleep(2)
      
    # disconnect participant 
    connection.DisconnectParty(confid,party_id)
    #connection.DisconnectParty(confid,isdn_party_id)
    connection.DeleteParty(confid,isdn_party_id)
    
    #connection.WaitAllOngoingDisConnected(confid, numRetries)
    print "Delete Conference..."
    connection.DeleteConf(confid)
         
    print "Wait until no conferences"
    connection.WaitAllConfEnd(50)
    
    return


##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()
print "Starting test: creating conference with 1 IP Participant and 1 ISDN Participant ..."
UpdatePartyVisualNameTest(c,                            
                		  40)
                
c.Disconnect()
