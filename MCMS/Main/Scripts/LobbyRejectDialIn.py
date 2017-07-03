#!/mcms/python/bin/python

#Updated 18/3/2006
#By Yoella

# For list of processes not for valgrind look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=CSApi ConfParty


from McmsConnection import *

def LobbyRejectDialIn(connection,num_of_parties):
        
    confName = "NonExistingConf"
    ### Add parties to EP Sim and connect them
    for x in range(num_of_parties):
        partyname = "Party"+str(x+1)
        print "Adding Sim Undefined Party " + partyname 
        connection.LoadXmlFile("Scripts/UndefinedDialIn/SimAdd323Party.xml")
        connection.ModifyXml("H323_PARTY_ADD","PARTY_NAME",partyname)
        connection.ModifyXml("H323_PARTY_ADD","CONF_NAME",confName)
        connection.Send()
        
   
        
    for y in range(num_of_parties):
        partyname = "Party"+str(y+1) 
        print "Trying to Connect Sim Party " + partyname + "  Reject expected"
        connection.LoadXmlFile("Scripts/UndefinedDialIn/SimConnect323Party.xml")
        connection.ModifyXml("H323_PARTY_CONNECT","PARTY_NAME",partyname)
        connection.Send()
        
    sleep(5)

    for index in range(num_of_parties):
        partyName = "Party"+str(index+1)
        DelSimParty(connection,partyName)

    
    sleep(5)
	
    ### Adding SIP parties
    for x1 in range(num_of_parties):
        partyname = "SipParty"+str(x1+1)
        print "Adding Sim Undefined SIP Party " + partyname 
        connection.LoadXmlFile("Scripts/SimAddSipParty.xml")
        connection.ModifyXml("SIP_PARTY_ADD","PARTY_NAME",partyname)
        connection.ModifyXml("SIP_PARTY_ADD","CONF_NAME",confName)
        connection.ModifyXml("SIP_PARTY_ADD","CAPSET_NAME","FULL CAPSET")
        connection.Send()
     
    
        
    for y1 in range(num_of_parties):
        partyname = "SipParty"+str(y1+1) 
        print "Trying to Connect Sim SIP Party " + partyname + "  Reject expected"
        connection.LoadXmlFile("Scripts/SimConnectSipParty.xml")
        connection.ModifyXml("SIP_PARTY_CONNECT","PARTY_NAME",partyname)
        connection.Send()
        
    sleep(5)
    for index1 in range(num_of_parties):
        partyName = "SipParty"+str(index1+1)
        DelSimParty(connection,partyName)


    #sleep(1)   
    
    return

#------------------------------------------------------------------------------
def DelSimParty(connection,partyName):
    print "Deleting SIM party "+partyName+"..."
    connection.LoadXmlFile("Scripts/UndefinedDialIn/SimDel323Party.xml")
    connection.ModifyXml("H323_PARTY_DEL","PARTY_NAME",partyName)
    connection.Send()
    return


## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

LobbyRejectDialIn(c,3) # num of parties

c.Disconnect()

sleep(4)
