#!/usr/bin/python

#*PROC-disabled-ESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer McuMngr Configurator BackupRestore

#############################################################################
#
# Date: 22/01/08
# By  : Ron S.
#############################################################################

from McmsConnection import *
import os   

#-----------------------------------------------------------------------------
def AddISDN_DialoutParty(connection,confId,partyName,partyChannels,phone):
    print "Adding ISDN dilaout party: "+partyName+ ", from EMA with phone: " + phone + " , num of channels: " + partyChannels
    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    connection.ModifyXml("PARTY","NAME",partyName)
    connection.ModifyXml("ADD_PARTY","ID",confId)
    connection.ModifyXml("PARTY","NET_CHANNEL_NUMBER",partyChannels)
    connection.ModifyXml("PHONE_LIST","PHONE1",phone)
    connection.Send()
#-----------------------------------------------------------------------------    
def CreateConfFromProfile(connection, confName, profileID, rate, fileName='Scripts/AddCpConf.xml'):
    print "Adding Conf " + confName + " ..."
    connection.LoadXmlFile(fileName)
    connection.ModifyXml("RESERVATION","NAME",confName)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profileID) 
    connection.ModifyXml("RESERVATION","TRANSFER_RATE",str(rate)) 
    connection.Send()
#-----------------------------------------------------------------------------    
def AddProfile(connection, profileName, rate, fileName="Scripts/CreateNewProfile.xml"):
    print "Adding new Profile..."
    connection.LoadXmlFile(fileName)
    connection.ModifyXml("RESERVATION","NAME", profileName)
    connection.ModifyXml("RESERVATION","TRANSFER_RATE",str(rate))
    connection.Send()
    ProfId = connection.GetTextUnder("RESERVATION","ID")
    print "Profile, named: " + profileName + " ,ID = " + ProfId + ", is added"
    return ProfId
## -----------------------------------------------------------------------------

def TestMultiRateConf(connection,ProfId,numOfConf,numOfISDNParties,num_retries,isRemoteDisconnect):    
    rate = 384
    for y in range(numOfConf):
    #create the target Conf and wait untill it connected
        profileName = "profile" + str(y+1)
        ProfId = AddProfile(connection,profileName,rate)
        targetConfName = "targetConf" + str(rate)
        CreateConfFromProfile(connection,targetConfName,ProfId,rate,"Scripts/MoveUndef/CreateNewConf.xml")
        targetConfID  = connection.WaitConfCreated(targetConfName,num_retries)
        if 0 == y :
            rate = rate + 128
        else:
            rate = rate + 256
     
        num_of_channels = 3
        for x in range(numOfISDNParties):
            partyname = "Party"+str(x+1)
            phone="3333"+str(x+1)
            partyChannels = "channel_" + str(num_of_channels)
            num_of_channels = num_of_channels * 2
            AddISDN_DialoutParty(connection,targetConfID,partyname,partyChannels,phone) 
            sleep(2)

   
        connection.WaitAllPartiesWereAdded(targetConfID,numOfISDNParties,num_retries*numOfISDNParties)
        sleep(2)
        connection.WaitAllOngoingConnected(targetConfID,num_retries*numOfISDNParties)
        
        print "Sleeping to complete the IVR"
        sleep(10)
     
        '''   
        print "Disconnect the parties"
        for x in range(numOfISDNParties):
            partyname = "Party"+str(x+1+y*numOfISDNParties)
            party_id = connection.GetPartyId(targetConfID, partyname)
     #       if isRemoteDisconnect == False :
            print "Disconnecting from EMA"
            connection.DisconnectParty(targetConfID,party_id)
            sleep(1)
                
        connection.WaitAllOngoingDisConnected(targetConfID)
        '''
         
        sleep(2)
        # Check if all parties were added and save their IDs
        num_of_parties = numOfISDNParties
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
            partyname = "Party"+str(x+1+y*numOfISDNParties)
            print "delete party: " + partyname    
            connection.DeleteParty(targetConfID,party_id_list[x])
            sleep(2)
    
        connection.DeleteConf(targetConfID)
        sleep(2)
        c.DelProfile(ProfId) 
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
targetEqName = "IsdnEQ"
eqPhone="3355"
numOfConf = 4
c.CreatePSTN_EQ(targetEqName, eqPhone,ProfId)
eqId, eqNID = c.WaitMRCreated(targetEqName)

TestMultiRateConf(c,                
                ProfId,#Profile ID
                numOfConf,
                3,
                30, # Num of retries
                False)#Disconnect from Ema

#remove the EQ Resrv
c.DelReservation(eqId, "Scripts/AwakeMrByUndef/RemoveMr.xml")

#remove the profile
c.DelProfile(ProfId)
c.Disconnect()
