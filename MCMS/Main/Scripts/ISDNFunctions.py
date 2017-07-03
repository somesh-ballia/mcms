#!/mcms/python/bin/python
#FUNCTIONS fo ISDN Scripts
#  
# Date: 04/05/08
# By  : Olga
#############################################################################

from McmsConnection import *
import os

#------------------------------------------------------------------------------
def AddIsdnProfile(connection, profileName, rate, fileName="Scripts/CreateNewProfile.xml"):
    print "Adding Isdn Profile..."
    connection.LoadXmlFile(fileName)
    connection.ModifyXml("RESERVATION","NAME", profileName)
    connection.ModifyXml("RESERVATION","TRANSFER_RATE", rate)
    connection.Send()
    ProfId = connection.GetTextUnder("RESERVATION","ID")
    print "Profile, named: " + profileName + " ,ID = " + ProfId + ", is added"
    return ProfId

#-----------------------------------------------------------------------------
def SimulationConnectIsdnParty(connection, partyName):
    print "Connect participant from EPsim"
    connection.LoadXmlFile("Scripts/SimConnectEndpoint.xml")
    connection.ModifyXml("PARTY_CONNECT","PARTY_NAME",partyName)
    connection.Send()

#-----------------------------------------------------------------------------
def SimulationAddIsdnParty(connection, partyName,phone,chnlNum="NONE"):
    print "Add participant: " + partyName+", phone="+ phone
    connection.LoadXmlFile('Scripts/SimAddIsdnEP.xml')
    connection.ModifyXml("ISDN_PARTY_ADD","PARTY_NAME",partyName)
    connection.ModifyXml("ISDN_PARTY_ADD","PHONE_NUMBER",phone)
    if chnlNum != "NONE":
    	connection.ModifyXml("ISDN_PARTY_ADD","CHANNELS_DIAL_IN_NUMBER",chnlNum)
    connection.Send()

#-----------------------------------------------------------------------------
def AddIsdnDialoutParty(connection,confId,partyName,phone,chnlNum="NONE", expected_status="Status OK"):
    print "Adding ISDN dilaout party: "+partyName+ ", from EMA with phone: " + phone
    connection.LoadXmlFile('Scripts/ISDN_Party.xml')
    connection.ModifyXml("PARTY","NAME",partyName)
    connection.ModifyXml("ADD_PARTY","ID",confId)
    connection.ModifyXml("PHONE_LIST","PHONE1",phone)
    if chnlNum != "NONE":
    	connection.ModifyXml("PARTY","NET_CHANNEL_NUMBER",chnlNum)
    connection.Send(expected_status)

#-----------------------------------------------------------------------------
#def AddIsdnDialinParty(connection,confId,partyName,phone,chnlNum,expected_status="Status OK"):
#    print "Adding ISDN dilain party: "+partyName+ ", from EMA with phone: " + phone
#    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
#    connection.ModifyXml("PARTY","NAME",partyName)
#    connection.ModifyXml("ADD_PARTY","ID",confId)
#    connection.ModifyXml("PHONE_LIST","PHONE1",phone)
#    connection.ModifyXml("PARTY","NET_CHANNEL_NUMBER",chnlNum)
#    connection.ModifyXml("PARTY","CONNECTION","dial_in")
#    connection.Send(expected_status)

#------------------------------------------------------------------------------
def TestDialOutISDN(connection, confid, num_of_parties, num_retries, deleteConf="TRUE",delayBetweenParticipants=0):

    for x in range(num_of_parties):
        partyname = "IsdnParty"+str(x+1)
        phone="3333"+str(x+1)
        print "Adding Party ("+partyname+")"
    	AddIsdnDialoutParty(connection,confid,partyname,phone)
        sleep(delayBetweenParticipants+2)

    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()
    ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    all_num_parties = len(ongoing_party_list)

    connection.WaitAllPartiesWereAdded(confid,all_num_parties,num_retries*all_num_parties)
    connection.WaitAllOngoingConnected(confid,num_retries*all_num_parties,delayBetweenParticipants+1)

    if deleteConf=="TRUE":
        print "delayBetweenParticipants = " + str(delayBetweenParticipants)
        if(delayBetweenParticipants > 0):
    	    participants_id_list = []
    	    for x in range(all_num_parties):
    	        some_party_id = connection.GetTextUnder("PARTY","ID",x)
                if some_party_id != "":  
                    participants_id_list.append(some_party_id)  
            for x in participants_id_list:
                connection.DeleteParty(confid,x)
                print "Delete party id - "+ x
                sleep(delayBetweenParticipants)
            
        #print "Delete Conference..."
        connection.DeleteConf(confid)                
            
        #print "Wait until no conferences..."
        connection.WaitAllConfEnd()
