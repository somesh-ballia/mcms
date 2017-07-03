#!/usr/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
# Test Script which is checking conference with encrypted dial in parties
#
# Date: 12/11/05
# By  : Udi B.
#############################################################################

from McmsConnection import *
import os

def TestPSTNConf(connection,numOfParties,num_retries):
    print "StartingTestPSTNConf.. "
    #add a new profile
    ProfId = connection.AddProfile("profile1")
    
    #create the target Conf and wait untill it connected
    targetConfName = "PSTN_Conf"
    connection.CreateConfFromProfile(targetConfName, ProfId)
    confId  = connection.WaitConfCreated(targetConfName,num_retries)

    #Adding PSTN parties:
    connection.LoadXmlFile("Scripts/PSTN_Part.xml")
    for x in range(numOfParties):
        partyname = "Party"+str(x+1)
        #partyip =  "123.123.0."+str(x+1)
        print "Adding Dial-out defined Party: "+partyname
        connection.ModifyXml("PARTY","NAME",partyname)
        #connection.ModifyXml("PARTY","IP",partyip)
        #connection.ModifyXml("ADD_PARTY",'ENCRYPTION_EX','yes')
        connection.ModifyXml("ADD_PARTY","ID",confId)
        connection.Send()

    connection.WaitAllPartiesWereAdded(confId,numOfParties,num_retries*numOfParties)
    connection.WaitAllOngoingConnected(confId)
    
    print "Remote Disconnect the parties"
    for x in range(numOfParties):
        #EpSim specified dial-out name
        partyname = "DIAL_OUT#1000"+str(x+1)
        connection.SimulationDisconnectPSTNParty(partyname)
        
    connection.WaitAllOngoingDisConnected(confId)

    connection.DeleteConf(confId)
    connection.DelProfile(ProfId)

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()
TestPSTNConf(c,
              1,
              20) # Num of retries


c.Disconnect()
