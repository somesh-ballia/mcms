#!/usr/bin/python

#-LONG_SCRIPT_TYPE

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

#############################################################################
#
# Date: 10/03/08
# By  : Olga S.
#############################################################################

from McmsConnection import *
import os

#-----------------------------------------------------------------------------    
def AddProfile(connection, profileName, fileName="Scripts/CreateNewProfile.xml"):
    print "Adding new Profile..."
    connection.LoadXmlFile(fileName)
    connection.ModifyXml("RESERVATION","NAME", profileName)
    connection.Send()
    profId = connection.GetTextUnder("RESERVATION","ID")
    print "Profile, named: " + profileName + " ,ID = " + profId + ", is added"
    return profId

#-----------------------------------------------------------------------------
def AddISDN_DialoutParty(connection,confId,partyName,phone):
    print "Adding ISDN dilaout party: "+partyName+ ", from EMA with phone: " + phone
    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    connection.ModifyXml("PARTY","NAME",partyName)
    connection.ModifyXml("ADD_PARTY","ID",confId)
    connection.ModifyXml("PHONE_LIST","PHONE1",phone)
    connection.Send()

## -----------------------------------------------------------------------------
def SimulationConnectISDNParty(connection, partyName):
    print "Connect participant from EPsim"
    connection.LoadXmlFile("Scripts/SimConnectEndpoint.xml")
    connection.ModifyXml("PARTY_CONNECT","PARTY_NAME",partyName)
    connection.Send()
#-----------------------------------------------------------------------------
def SimulationAddISDNParty(connection, partyName, phone):
    print "Add participant: " + partyName+", phone="+ phone
    connection.LoadXmlFile('Scripts/SimAddIsdnEP.xml')
    connection.ModifyXml("ISDN_PARTY_ADD","PARTY_NAME",partyName)
    connection.ModifyXml("ISDN_PARTY_ADD","PHONE_NUMBER",phone)
    connection.Send()

#-----------------------------------------------------------------------------

def TestMultiPartyConf(connection,ProfId,numOfConf,numDialIn,numDialOut,num_retries):    
    print "Test Multi Party Conf..."
    for y in range(numOfConf):
        #create the target Conf and wait untill it is connected
        profileName = "profile" + str(y+1)
        targetConfName = "TargetConf" + str(y)
        connection.CreateConfFromProfile(targetConfName, ProfId)
        targetConfID = connection.WaitConfCreated(targetConfName,num_retries)

        for x in range(numDialOut):
            partyname = "PartyOut"+str(x+1)
            phone="3333"+str(x+1)
            AddISDN_DialoutParty(connection,targetConfID,partyname,phone)

    	targetConfNumericId = connection.GetConfNumericId(targetConfID)
        phone_dialin="3344"

        for i in range(numDialIn):
            partyname = "PartyIn"+str(i+1)
	    if (0 == y):
 	        SimulationAddISDNParty(connection,partyname,phone_dialin)

	    SimulationConnectISDNParty(connection,partyname)
            eqConfId = connection.WaitUntillEQorMRAwakes(targetEqName,1,num_retries,True)

            #send the DTMF to the EPsim with the numeric id of the target conf
 	    connection.SimulationH323PartyDTMF(partyname, targetConfNumericId)
            sleep(1)


        n_retries = num_retries*(numDialIn + numDialOut)
        connection.WaitAllPartiesWereAdded(targetConfID, numDialIn+numDialOut, n_retries)
        connection.WaitAllOngoingConnected(targetConfID, n_retries)
        
        print "Sleeping to complete the IVR"
        sleep(2)

        # Check if all parties were added and save their IDs
        num_of_parties = numDialIn + numDialOut
        party_id_list = []
        connection.LoadXmlFile('Scripts/TransConf2.xml')
        connection.ModifyXml("GET","ID",targetConfID)
        connection.Send()
        ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        if len(ongoing_party_list) < num_of_parties:
            sys.exit("some parties are lost...")
        for index in range(num_of_parties):    
            party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)
        
        # delete all parties  
        for x in range(num_of_parties):
            partyname = ongoing_party_list[x].getElementsByTagName("NAME")[0].firstChild.data
            print "delete party: " + partyname    
            connection.DeleteParty(targetConfID,party_id_list[x])
            sleep(1)
    
        # delete conference  
    	connection.WaitAllOngoingDisConnected(targetConfID, n_retries)
        connection.DeleteConf(targetConfID)
    	connection.WaitConfEnd(targetConfID)
         
    # delete EQ  
    connection.DeleteConf(eqConfId)
    connection.WaitConfEnd(eqConfId)


## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()
#add a new profile
ProfId = c.AddProfile("profile")

#create the target Conf and wait untill it connected
targetEqName = "IsdnEQ"
eqPhone="3344"
numOfConf = 20
c.CreatePSTN_EQ(targetEqName, eqPhone,ProfId)
eqId, eqNID = c.WaitMRCreated(targetEqName)

TestMultiPartyConf(c,                
                   ProfId,#Profile ID
                   numOfConf,
                   2, 2,  #num of dial-in and dial-out participants
                   50)    #num of retries

#remove the EQ Resrv
c.DelReservation(eqId, "Scripts/AwakeMrByUndef/RemoveMr.xml")

#remove the profile
c.DelProfile(ProfId)
c.Disconnect()
