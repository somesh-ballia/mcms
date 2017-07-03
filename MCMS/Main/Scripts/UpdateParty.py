#!/mcms/python/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

from McmsConnection import *
from ISDNFunctions import *

###------------------------------------------------------------------------------

def UpdatePartyTest(connection, numRetries):

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
    
    # disconnect participants 
    connection.DisconnectParty(confid,party_id)
    connection.DisconnectParty(confid,isdn_party_id)
    
    connection.WaitAllOngoingDisConnected(confid, numRetries)
       
    # update ip participant 
    partyname = "Party11" 
    partyip =  "5.6.7.8"
    print "Updating party"
    connection.LoadXmlFile('Scripts/UpdateParty/UpdateParty.xml')
    connection.ModifyXml("UPDATE_PARTY", "ID", confid)
    connection.ModifyXml("PARTY","NAME",partyname)
    connection.ModifyXml("PARTY","ID",party_id)
    connection.ModifyXml("PARTY","IP",partyip)
    connection.Send()
    sleep(2)
    
    # update isdn participant 
    isdnpartyname = "IsdnParty11" 
    phone =  "333311"
    print "Updating isdn party"
    connection.LoadXmlFile('Scripts/UpdateParty.xml')
    connection.ModifyXml("UPDATE_PARTY", "ID", confid)
    connection.ModifyXml("PARTY","NAME",isdnpartyname)
    connection.ModifyXml("PARTY","ID",isdn_party_id)
    connection.ModifyXml("PHONE_LIST","PHONE1",phone)
   
    
    connection.Send()
    sleep(2)
    
    
    connection.WaitAllOngoingConnected(confid,numRetries)
   
    # Check if party was added and save it's ID
    party_id = connection.GetPartyId(confid, partyname)
    isdn_party_id = connection.GetPartyId(confid, isdnpartyname)
    
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
UpdatePartyTest(c,                            
                40)
                
c.Disconnect()
