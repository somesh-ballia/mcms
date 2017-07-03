#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#-LONG_SCRIPT_TYPE


from McmsConnection import *



#------------------------------------------------------------------------------
def ConnectDisconnectAllParties(connection,confid,connectVal):
    if connectVal == "true":
        msgStr = "Connecting"
    else:
        msgStr = "Disconnecting"

    connection.LoadXmlFile('Scripts/ReconnectParty/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()
    
    ongoing_parties = connection.xmlResponse.getElementsByTagName("PARTY")
    num_ongoing_parties = len(ongoing_parties)
    print msgStr + " " + str(num_ongoing_parties) + " parties in conf " + str(confid)
    
    for x in range(num_ongoing_parties):
        partyId = ongoing_parties[x].getElementsByTagName("ID")[0].firstChild.data
        print msgStr + " party " + str(partyId)
        connection.LoadXmlFile('Scripts/ReconnectParty/TransSetConnect.xml')
        connection.ModifyXml("SET_CONNECT","ID",confid)
        connection.ModifyXml("SET_CONNECT","CONNECT",connectVal)
        connection.ModifyXml("SET_CONNECT","PARTY_ID",partyId)
        connection.Send()
    return
#------------------------------------------------------------------------------

def SleepAndTestConf(connection,confIdList,minutes,loops):
    for x in range(loops) :
        #Wait minutes
        print "\n(" + str(x) + " out of " + str(loops) + ") Sleeping for " + str(minutes) + " minutes.......\n"
        
        for a in range(minutes) :
            sleep(60)
            for confid in confIdList:
                connection.WaitAllOngoingConnected(confid,num_retries)
                
        for confid in confIdList:
            # Disconnecting all parties
            ConnectDisconnectAllParties(connection,confid,"false")
            connection.WaitAllOngoingDisConnected(confid)
        
            ConnectDisconnectAllParties(connection,confid,"true")
            connection.WaitAllOngoingConnected(confid,num_retries)
#------------------------------------------------------------------------------

def SimpleXmlConfPartyTest(connection,profId,partyFile,num_of_parties,num_retries,deleteConf="TRUE",confName='Conf1'):
    connection.CreateConfFromProfile(confName,profId,"Scripts/LongConnDisc/CreateLongConf.xml")
    confid = connection.WaitConfCreated(confName,num_retries)
    
    connection.LoadXmlFile(partyFile)
    for x in range(num_of_parties):
        partyname = "Party"+str(x+1)
        partyip =  "1.2.3." + str(x+1)
        print "Adding Party ("+partyname+") to confID " + str(confid)
        connection.ModifyXml("PARTY","NAME",partyname)
        connection.ModifyXml("PARTY","IP",partyip)
        connection.ModifyXml("ADD_PARTY","ID",confid)
        connection.Send()

    all_num_parties = num_of_parties
    
    connection.WaitAllPartiesWereAdded(confid,all_num_parties,num_retries*all_num_parties)    
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()
    some_party_id = connection.GetTextUnder("PARTY","ID")
    connection.WaitAllOngoingConnected(confid,num_retries*all_num_parties)

    if deleteConf=="TRUE":        
        #print "Delete one Party..."
        connection.DeleteParty(confid,some_party_id)
        
        #print "Delete Conference..."
        connection.DeleteConf(confid)
        
        #print "Wait until no conferences..."
        connection.WaitAllConfEnd()

    return confid

#------------------------------------------------------------------------------

                
if __name__ == '__main__':
    connection = McmsConnection()
    connection.Connect()

    partyFile = 'Scripts/AddVideoParty1.xml'
    num_of_parties = 3
    num_retries = num_of_parties * 5
    hour = 60
    confIdList = list()

    #add a new profile
    profId = connection.AddProfile("profile", "Scripts/LongConnDisc/Profile_Long.xml")
 
    #Create Conference with 3 parties
    time = 2* hour
    confid = SimpleXmlConfPartyTest(connection,profId,partyFile,num_of_parties,num_retries,"false")
    confIdList.append(confid)
    
    minutes = 10

    SleepAndTestConf(connection,confIdList,minutes,time / minutes)
    
    #delete conferences
    for confID in confIdList:
        connection.DeleteConf(confid)   
        connection.WaitAllConfEnd()
        
    #Test another conferences for 3 hour
    time = 4 * hour
    confIdList =list()
    
    #Create a new conference with 4 parties
    confid = SimpleXmlConfPartyTest(connection,profId,partyFile,4,num_retries,"false",'Conf1')
    confIdList.append(confid)
    
    #create a new conference with 5 parties
    confid = SimpleXmlConfPartyTest(connection,profId,partyFile,5,num_retries,"false",'Conf2')
    confIdList.append(confid)
    
    #Create another conf with 3 parties
    confid = SimpleXmlConfPartyTest(connection,profId,partyFile,3,num_retries,"false",'Conf3')
    confIdList.append(confid)

    SleepAndTestConf(connection,confIdList,minutes,time / minutes)

    #delete conferences
    for confID in confIdList:
        connection.DeleteConf(confID)

    #delete profile
    connection.DelProfile(profId)
    connection.WaitAllConfEnd(20 * len(confIdList))
    connection.Disconnect()



