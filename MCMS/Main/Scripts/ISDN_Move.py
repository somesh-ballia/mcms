#!/usr/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_20
#-LONG_SCRIPT_TYPE


#############################################################################
#
# Date: 22/01/08
# By  : Ron S.
#############################################################################

from McmsConnection import *
import os   
from PartyUtils.H323PartyUtils import *
H323PartyUtilsClass = H323PartyUtils() # H323Party utils util class


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
def TestDialInParty(connection,eqId,ProfId,numOfParties,num_retries,isRemoteDisconnect):    
    
    #create the target Conf and wait untill it connected
    targetConfName = "targetConf"
    connection.CreateConfFromProfile(targetConfName,ProfId)
    
    targetConfID  = connection.WaitConfCreated(targetConfName,num_retries)
    
    targetConfNumericId = connection.GetConfNumericId(targetConfID)
    print "Target Conf Numeric id is " + str(targetConfNumericId)
    
    #Add Dial-in un-defined parties
    for x in range(numOfParties):
        partyname = "IsdnParty"+str(x+1)
        phone="3355"
        SimulationAddISDNParty(connection,partyname,phone)
        SimulationConnectISDNParty(connection,partyname)
        sleep(2)
    
    eqConfId = connection.WaitUntillEQorMRAwakes(targetEqName,numOfParties,num_retries,True)
    connection.WaitAllOngoingConnected(eqConfId,numOfParties*num_retries)
    
    #send the TDMF to the EPsim with the numeric id of the target conf
    for x in range(numOfParties):
       partyname = "IsdnParty"+str(x+1)
       connection.SimulationH323PartyDTMF(partyname, targetConfNumericId)

    #Make sure the parties are connected in the target Conf.
    connection.WaitAllPartiesWereAdded(targetConfID,numOfParties,numOfParties*num_retries)
    connection.WaitAllOngoingConnected(targetConfID,numOfParties*num_retries)

    #Sleeping for IVR messages
    print "Sleeping Untill IVR finished..."
    sleep(5)

    if isRemoteDisconnect == True :
        print "Disconnect the parties from Remote"
        for x in range(numOfParties):
            partyname = "IsdnParty"+str(x+1)
            connection.DeletePSTNPartyFromSimulation(partyname)
        connection.WaitAllOngoingDisConnected(targetConfID)
        
    else:
        print "Disconnect the parties from EMA"     
         
       # Check if all parties were added and save their IDs
      
        party_id_list = []

        connection.LoadXmlFile('Scripts/TransConf2.xml')
        connection.ModifyXml("GET","ID",targetConfID)
        connection.Send()
        ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        if len(ongoing_party_list) < numOfParties:
            sys.exit("some parties are lost...")
        for index in range(numOfParties):    
            party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)

        # delete all parties  
        for x in range(numOfParties):
            partyname = "IsdnParty"+str(x+1)
            print "delete party: " + partyname    
            connection.DeleteParty(targetConfID,party_id_list[x])
            sleep(2)


    #Delete The Target Conference
    connection.DeleteConf(targetConfID)
    connection.WaitConfEnd(targetConfID)

    #delete The EQ COnf
    connection.DeleteConf(eqConfId)
    connection.WaitConfEnd(eqConfId)
         
#------------------------------------------------------------------------------
def ConnectXSimulationParties(connection,targetConf,num_parties,num_retries):     
    # Create a new undefined party and add it to the EPSim
    for x in range(num_parties):
        partyName = "Party" + str(x+1)
        connection.SimulationAddH323Party(partyName, targetConf)
        connection.SimulationConnectH323Party(partyName)
        
    return

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()
#add a new profile
ProfId = c.AddProfile("profile")

#create the target Conf and wait untill it connected
targetEqName = "IsdnnEQ"
eqPhone="3355"
c.CreatePSTN_EQ(targetEqName, eqPhone,ProfId)
eqId, eqNID = c.WaitMRCreated(targetEqName)

TestDialInParty(c,
                eqId,  #EQ reservation ID
                ProfId,#Profile ID
                3,
                30, # Num of retries
                False)#Disconnect from Remote

'''
TestDialInParty(c,
                eqId,  #EQ reservation ID
                ProfId,#Profile ID
                3,
                20, # Num of retries
                False)#Disconnect From EMA (by delete Conference)
'''
        
#remove the EQ Resrv
c.DelReservation(eqId, "Scripts/AwakeMrByUndef/RemoveMr.xml")

#remove the profile
c.DelProfile(ProfId)
H323PartyUtilsClass.SimulationDeleteAllSimParties(c)
c.Disconnect()
