#!/usr/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

#############################################################################
# Test Script is checking if a reconnecting of ISDN participant works fine.
# Date: 02.03.2008
# By  : Olga S.
#############################################################################

from McmsConnection import *
import os

#-----------------------------------------------------------------------------
def AddISDN_DialoutParty(connection,confId,partyName,phone):
    print "Adding ISDN dilaout party: "+partyName+ ", from EMA with phone: " + phone
    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    connection.ModifyXml("PARTY","NAME",partyName)
    connection.ModifyXml("ADD_PARTY","ID",confId)
    connection.ModifyXml("PHONE_LIST","PHONE1",phone)
    connection.Send()

#------------------------------------------------------------------------------
def ChangeConfLayoutTypeTest(connection, confid, newConfLayout):
    print "Conference ID: "+ confid + " Changing Layout Type To: " + newConfLayout        
    connection.LoadXmlFile('Scripts/ConfVideoLayout/ChangConfLayout.xml')
    connection.ModifyXml("SET_VIDEO_LAYOUT","ID",confid)
    connection.ModifyXml("FORCE","LAYOUT",newConfLayout)       
    connection.Send()
    connection.WaitAllOngoingChangedLayoutType(confid,newConfLayout)
    return
 
# -----------------------------------------------------

def TestConfLayout(connection,confId,num_of_parties,num_retries):
    print "Starting Test ISDN Conf Layout..."
        
    #Add and connect ISDN parties:
    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    for x in range(num_of_parties):
        partyname = "Party"+str(x+1)
        phone="3333"+str(x+1)
        AddISDN_DialoutParty(connection,confId,partyname,phone)
	sleep(1)
                
    connection.WaitAllPartiesWereAdded(confId,num_of_parties,num_retries*num_of_parties)
    connection.WaitAllOngoingConnected(confId,num_retries*num_of_parties)

    #Changing conf layout type to each layout
    for layout in availableLayoutTypes:
        ChangeConfLayoutTypeTest(connection,confId,layout)
        sleep(1)

    print "Sleep before delete conference"
    sleep(10)

    return

## ---------------------- Test --------------------------

c = McmsConnection()
c.Connect()

#add a new profile
profId = c.AddProfile("profile1")
#create the target Conf and wait untill it connected
targetConfName = "ISDN_Conf"
num_retries=30
c.CreateConfFromProfile(targetConfName, profId)
confId  = c.WaitConfCreated(targetConfName,num_retries)

TestConfLayout(c,confId,
               3, #num of participants
               num_retries)

c.DeleteConf(confId)   
c.WaitAllConfEnd()

c.DelProfile(profId)
c.Disconnect()
