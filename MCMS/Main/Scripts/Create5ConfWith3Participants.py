#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=GideonSim
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_30SD.xml"
#-LONG_SCRIPT_TYPE

#############################################################################
# Test Script which Creating X confernces with Y participants
#
# Date: 4/01/05
# By  : Udi B.

#############################################################################

from McmsConnection import *
from ISDNFunctions import *

###------------------------------------------------------------------------------
def SimpleXConfWithYParticipaints(connection,numOfConfs,numOfParties,
                                  confFile,
                                  partyFile,
                                  numRetries):
    
    IsConfPartyUnderValgrind = connection.IsProcessUnderValgrind("ConfParty")
    confIdArray = [0]*numOfConfs
    for confNum in range(numOfConfs):
        confname = "Conf"+str(confNum+1)
        connection.CreateConf(confname, confFile)
        confid = connection.WaitConfCreated(confname,numRetries)
        confIdArray[confNum] = confid
        print "Created conf with id " + str(confIdArray[confNum]) 
        
        for x in range(numOfParties):
            partyname = "Party" + str(x+1)+str(confIdArray[confNum])
            partyip =  "1.2."+ str(confIdArray[confNum]) + "." + str(x+1)
            print "Adding Party " + partyname + ", with ip= " + partyip
            connection.AddParty(confid, partyname, partyip, partyFile)

        for x in range(numOfParties):
            partyname = "IsdnParty"+str(x+1)+str(confIdArray[confNum])
            phone="3333"+str(x+1)+str(confIdArray[confNum])
            AddIsdnDialoutParty(connection,confIdArray[confNum],partyname,phone) 
            if (IsConfPartyUnderValgrind and x==0):
                # we need long sleep time in order to let the ip parties to change their content mode from 264 to 263
                print "---- sleeping for 30 seconds to let ip parties finish changing their content mode to 263 ----"
                sleep(28)  
            sleep(2)
            
        connection.WaitAllPartiesWereAdded(confIdArray[confNum],numOfParties*2,numRetries*2)
        connection.WaitAllOngoingConnected(confIdArray[confNum],numRetries*2)
        connection.WaitAllOngoingNotInIVR(confIdArray[confNum],numRetries*2)
        
        print"=================================================================="    
    #print "Sleeping for 5 seconds..."
    #sleep(5)
        
    for deletedConfNum in range(numOfConfs):
        connection.DeleteConf(confIdArray[deletedConfNum])
	connection.WaitConfEnd(confIdArray[deletedConfNum], numRetries)
        
    connection.WaitAllConfEnd(50)
    
    return

##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

SimpleXConfWithYParticipaints(c,
                              5,# Num of Confs
                              3,# Num of parties
                              'Scripts/Create5ConfWith3Participants/AddVideoCpConfTemplate.xml',
                              'Scripts/Create5ConfWith3Participants/AddVideoParty1.xml',
                              60) #num of retries

c.Disconnect()

# it solves leaks in logger. (Yuri R, 29/07.06)
sleep(5)
##------------------------------------------------------------------------------
