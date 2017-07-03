#!/usr/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

#############################################################################
#
# Date: 16/01/07
# By  : Ron S.
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
def SimulationAddISDNParty(connection, partyName,phone):
    print "Add participant:" + partyName+", phone="+ phone
    connection.LoadXmlFile('Scripts/SimAddIsdnEP.xml')
    connection.ModifyXml("ISDN_PARTY_ADD","PARTY_NAME",partyName)
    connection.ModifyXml("ISDN_PARTY_ADD","PHONE_NUMBER",phone)
    connection.Send()

## ----------------------  --------------------------
def TestDialInParty(connection,eqId,ProfId,numOfParties,num_retries,isRemoteDisconnect,addEP=1):    

    #Add Dial-in undefined parties
    for x in range(numOfParties):
        partyname = "IsdnParty"+str(x+1)
        phone="3344"
   	if addEP == 1 :
            SimulationAddISDNParty(connection,partyname,phone)
        SimulationConnectISDNParty(connection,partyname)
        sleep(2)
        
    eqConfId = connection.WaitUntillEQorMRAwakes(targetEqName,numOfParties,num_retries,True)
    connection.WaitAllOngoingConnected(eqConfId,numOfParties*num_retries)

    #Sleeping for IVR messages
    print "Sleeping..."
    sleep(10)
    
    # Check if all parties were added and save their IDs
    connection.LoadXmlFile('Scripts/TransConf2.xml')
    connection.ModifyXml("GET","ID",eqConfId)
    connection.Send()
    ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    if len(ongoing_party_list) < numOfParties:
        sys.exit("some parties are lost...")

    # delete all parties
    if isRemoteDisconnect == True :
        print "Disconnecting parties from remote"
        for x in range(numOfParties):
            partyname = "IsdnParty"+str(x+1)
     	    connection.SimulationDisconnectPSTNParty(partyname)   
            connection.DeletePSTNPartyFromSimulation(partyname)
            sleep(5)
    else:
        print "Disconnecting parties from EMA"
	for index in range(numOfParties):    
            party_id = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data
            partyname = ongoing_party_list[index].getElementsByTagName("NAME")[0].firstChild.data
            print "Deleting party " + partyname + ", party_id = " + party_id
	    connection.DeleteParty(eqConfId,party_id)
            sleep(2)

    connection.WaitAllOngoingDisConnected(eqConfId, numOfParties*num_retries)
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
c.CreatePSTN_EQ(targetEqName, eqPhone,ProfId)
eqId, eqNID = c.WaitMRCreated(targetEqName)

print "\n============ TestDialInParty & Disconnect From EMA ============="
TestDialInParty(c,
                eqId,  #EQ reservation ID
                ProfId,#Profile ID
                2,
                20, # Num of retries
                False)#Disconnect From EMA (by delete Conference)

print "\n============ TestDialInParty & Disconnect From Remote EP ============="
TestDialInParty(c,
                eqId,  #EQ reservation ID
                ProfId,#Profile ID
                2,
                20,   # Num of retries
                True, # Disconnect from Remote
		0)    # no need to add EP because it already exists
#remove the EQ Resrv
c.DelReservation(eqId, "Scripts/AwakeMrByUndef/RemoveMr.xml")

#remove the profile
c.DelProfile(ProfId)
c.Disconnect()
