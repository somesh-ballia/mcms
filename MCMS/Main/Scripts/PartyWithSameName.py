#!/mcms/python/bin/python

#############################################################################
# Eror handling Script which Creating Party with a name of already existing Party 
#
# Date: 17/01/05
# By  : Ron S.

#############################################################################

from McmsConnection import *

###------------------------------------------------------------------------------
def AddPartyWithExistPartyName(connection,confFile,partyFile,numRetries):
    
    confname = "Conf"
    connection.CreateConf(confname)

    confid = connection.WaitConfCreated(confname,numRetries)
        
    print "Create conf with id " + str(confid) 
      
# adding the first participant 
    partyname = "PartyWithSameName" 
    partyip =  "1.2.3.1"
    connection.AddParty(confid, partyname, partyip, partyFile)

    connection.WaitAllPartiesWereAdded(confid,1,numRetries)
    connection.WaitAllOngoingConnected(confid,numRetries)
    
# trying to add second participant with the same name as the first participant    
    partyip =  "1.2.3.2"
    connection.AddParty(confid, partyname, partyip, partyFile)

#    ongoing_parties = connection.xmlResponse.getElementsByTagName("PARTY")
#    num_ongoing_parties = len(ongoing_parties)
    
    connection.LoadXmlFile('Scripts/PartyWithSameName/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    print "Wait Untill the Party:" + partyname + " is added..",
    for retry in range(numRetries+1):
        ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY_STATUS")
        num_ongoing_parties = len(ongoing_parties)
        if 2 == num_ongoing_parties:
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("exit due to: Party with already existing name was connected")
        
        sys.stdout.write(".")
        sys.stdout.flush()
        if (retry == numRetries):
            print
            print connection.xmlResponse.toprettyxml()
            print "The Party:" + partyname + " was not connected because that a Party with the same name is already exist" 
            break
        sleep(1)
            
             
        

    #print "Delete Conference..."
    connection.DeleteConf(confid)
         
    #print "Wait until no conferences"
    connection.WaitAllConfEnd(50)
    
    return

##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

AddPartyWithExistPartyName(c,
                           'Scripts/PartyWithSameName/AddVoipConf.xml',
                           'Scripts/PartyWithSameName/AddVoipParty.xml',
                            20) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------
