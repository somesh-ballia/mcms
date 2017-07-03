#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9

#############################################################################
# Test Script for PLC - Checks that PLC activation effects the current party layout
#
# Date: 11/4/05
# By  : Talya
#############################################################################


from McmsConnection import *
import string 
#------------------------------------------------------------------------------
def TestPLC(connection,num_of_parties,num_retries):
    #Create conf with 3 paries
    confName = "undefConf"
    connection.CreateConf(confName, 'Scripts/SpeakerChange/AddCpConf.xml')
    
    confid = connection.WaitConfCreated(confName,num_retries)

    ### Add parties to EP Sim and connect them
    partiesIdToName=dict()
    for x in range(num_of_parties):
        partyname = "Party"+str(x+1)
        connection.SimulationAddH323Party(partyname, confName)
        connection.SimulationConnectH323Party(partyname)
        #Get the party id
        currPartyID = connection.GetCurrPartyID(confid,x,num_retries)
        print "currPartyID = " + str(currPartyID)
        if (currPartyID < 0):
            connection.Disconnect()                
            sys.exit("Error:Can not find partry id of party: "+partyname)
        print "found party id ="+str(currPartyID)
        partiesIdToName[currPartyID] = partyname
        sleep(1)
    
    connection.WaitUntilAllPartiesConnected(confid,num_of_parties,num_retries*num_of_parties)
    connection.WaitAllOngoingNotInIVR(confid)
    
    confLayoutType = "2x2"
    connection.ChangeConfLayoutType(confid, confLayoutType)
    connection.WaitAllOngoingChangedLayoutType(confid, confLayoutType)
    
    sleep(2)
    
    print
    print "Party 1 enters PLC"
    partyIdForPLC = 1
    connection.SimulationH323PartyDTMFWithoutDelimiter(partiesIdToName[partyIdForPLC], "**")
    connection.WaitPartySeesConfLayout(confid, partyIdForPLC, confLayoutType)
    
    print "Party 1 changes to personal layout 1+5"
    newLayout = "1and5"
    connection.SimulationH323PartyDTMFWithoutDelimiter(partiesIdToName[partyIdForPLC], "6")
    connection.WaitPartySeesPersonalLayout(confid, partyIdForPLC, newLayout)
    
    print "Party 1 changes to personal layout 1x1"
    newLayout = "1x1"
    connection.SimulationH323PartyDTMFWithoutDelimiter(partiesIdToName[partyIdForPLC], "1")
    connection.WaitPartySeesPersonalLayout(confid, partyIdForPLC,newLayout)
    
    print "Party 1 returns to conf layout"
    connection.SimulationH323PartyDTMFWithoutDelimiter(partiesIdToName[partyIdForPLC], "0")
    connection.WaitPartySeesConfLayout(confid, partyIdForPLC, confLayoutType)
    
    
    print
    print "Party 2 enters PLC"
    partyIdForPLC = 2
    connection.SimulationH323PartyDTMFWithoutDelimiter(partiesIdToName[partyIdForPLC], "**")
    connection.WaitPartySeesConfLayout(confid, partyIdForPLC, confLayoutType)
    
    print "Party 2 changes to personal layout 1+5"
    newLayout = "1and5"
    connection.SimulationH323PartyDTMFWithoutDelimiter(partiesIdToName[partyIdForPLC], "6")
    connection.WaitPartySeesPersonalLayout(confid, partyIdForPLC, newLayout)
    
    print
    print "Party 3 enters PLC"
    partyIdForPLC = 3
    connection.SimulationH323PartyDTMFWithoutDelimiter(partiesIdToName[partyIdForPLC], "**")
    connection.WaitPartySeesConfLayout(confid, partyIdForPLC, confLayoutType)
    
    print "Party 3 changes to personal layout 1x2"
    newLayout = "1x2"
    connection.SimulationH323PartyDTMFWithoutDelimiter(partiesIdToName[partyIdForPLC], "2")
    connection.SimulationH323PartyDTMFWithoutDelimiter(partiesIdToName[partyIdForPLC], "2")
    connection.SimulationH323PartyDTMFWithoutDelimiter(partiesIdToName[partyIdForPLC], "2")
    connection.SimulationH323PartyDTMFWithoutDelimiter(partiesIdToName[partyIdForPLC], "2")
    connection.WaitPartySeesPersonalLayout(confid, partyIdForPLC, newLayout)    
    
    print "Party 3 returns to conf layout"
    connection.SimulationH323PartyDTMFWithoutDelimiter(partiesIdToName[partyIdForPLC], "0")
    connection.WaitPartySeesConfLayout(confid, partyIdForPLC, confLayoutType)
    
    print
    print "Deleting Parties"
    for key in partiesIdToName:
        #print "Party id is:"+str(key) + " , and party name is"+partiesIdToName[key]
        connection.DeleteParty(confid,str(key))

    connection.WaitUntillPartyDeleted(confid,num_retries*num_of_parties)
    print "Deleting Conf"
    connection.DeleteConf(confid)
    connection.WaitAllConfEnd()
    return


## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestPLC(c,
         3, # num of parties
         20)# retries

c.Disconnect()

