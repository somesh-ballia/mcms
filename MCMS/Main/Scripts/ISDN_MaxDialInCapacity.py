#!/usr/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

#############################################################################
# Test Script checks conference with max numbers ISDN dial-in parties
#
# Date: 26/01/07
# By  : Olga S.
#############################################################################

from McmsConnection import *
import os


#-----------------------------------------------------------------------------
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


## ----------------------  --------------------------
def TestDialInParty(connection,eqId,numOfParties,num_retries,isRemoteDisconnect):    

    targetConfNumericId = connection.GetConfNumericId(confId)
    #Add Dial-in undefined parties
    for x in range(numOfParties):
        partyname = "IsdnParty"+str(x+1)
        phone="3344"
	#add EP
        SimulationAddISDNParty(connection,partyname,phone)
	#connect EP
        SimulationConnectISDNParty(connection,partyname)
        sleep(2)
        #send the DTMF to the EPsim with the numeric id of the target conf
 	connection.SimulationH323PartyDTMF(partyname, targetConfNumericId)
        sleep(2)
      
#    eqConfId = connection.WaitUntillEQorMRAwakes(targetEqName,numOfParties,num_retries,True)
    eqConfId, eqName = connection.WaitEqConfCreated(targetEqName,num_retries)

    #Sleeping for IVR messages
    print "Sleeping..."
    sleep(20)
    
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",confId)
    connection.Send()
    ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    num_of_parties = len(ongoing_party_list)
    if num_of_parties < numOfParties:
        sys.exit("Some parties are lost...")

    if isRemoteDisconnect == True :
        print "Disconnecting parties from remote"
        for x in range(numOfParties):
            partyname = ongoing_party_list[x].getElementsByTagName("NAME")[0].firstChild.data
            print "Deleting party " + partyname
            connection.DeletePSTNPartyFromSimulation(partyname)
            sleep(2)            
    else:
        print "Disconnecting parties from EMA"
 	for x in range(num_of_parties):    
            partyname = ongoing_party_list[x].getElementsByTagName("NAME")[0].firstChild.data
            print "Deleting party " + partyname
            party_id = connection.GetPartyId(confId, partyname)
            connection.DeleteParty(confId,party_id)
            sleep(2)
     
    connection.WaitAllOngoingDisConnected(confId)
    connection.DeleteConf(eqConfId)
    connection.WaitConfEnd(eqConfId)



## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()
#add a new profile
ProfId = c.AddProfile("profile")

#create the target Conf and wait untill it connected
targetEqName = "ISDN_EQ"
eqPhone="3344"
c.CreatePSTN_EQ(targetEqName, eqPhone,ProfId)
eqId, eqNID = c.WaitMRCreated(targetEqName)

#create the target main Conf and wait untill it is connected
num_retries=20
targetConfName = "ISDN_Conf"
c.CreateConfFromProfile(targetConfName, ProfId)
confId = c.WaitConfCreated(targetConfName,num_retries)

'''
TestDialInParty(c,
                eqId,  #EQ reservation ID
                35,    #Num of parties
                num_retries, # Num of retries
                True)  #Disconnect from Remote
'''
TestDialInParty(c,
                eqId,  #EQ reservation ID
                35,
                num_retries, # Num of retries
                False)#Disconnect From EMA (by delete Conference)

#remove the EQ Resrv
c.DelReservation(eqId, "Scripts/AwakeMrByUndef/RemoveMr.xml")
    
#remove the main Conf
c.DeleteConf(confId)
c.WaitConfEnd(confId)

#remove the profile
c.DelProfile(ProfId)
c.Disconnect()
