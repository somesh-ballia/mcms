#!/mcms/python/bin/python
#FUNCTIONS fo Lpr Scripts
#  
# Date: 5/5/08
# By  : GuyD
#############################################################################
from McmsConnection import *
from ContentFunctions import *
import string 

#------------------------------------------------------------------------------
def AddDialInParty(connection,partyname,partyid,confName,confid,num_retries,capName="FULL CAPSET"):
    connection.SimulationAddH323Party(partyname, confName,capName)
    connection.SimulationConnectH323Party(partyname)
    #Get the party id
    currPartyID = connection.GetCurrPartyID(connection,confid,partyid,num_retries)
    if (currPartyID < 0):
       connection.Disconnect()                
       sys.exit("Error:Can not find partry id of party: "+partyname)
    print "party id="+str(currPartyID)+" found in Conf"
  
#------------------------------------------------------------------------------
def ActivateLpr(connection, partyName, lossProtection = 2, mtbf = 6, congestionCeiling = 14000, modeTime = 0):
    
  connection.LoadXmlFile('Scripts/SimLprModeChangeRequest.xml')
  connection.ModifyXml("LPR_MODE_CHANGE_REQUEST","PARTY_NAME",partyName)
  connection.ModifyXml("LPR_MODE_CHANGE_REQUEST","LOSS_PROTECTION",lossProtection)
  connection.ModifyXml("LPR_MODE_CHANGE_REQUEST","MTBF",mtbf)
  connection.ModifyXml("LPR_MODE_CHANGE_REQUEST","CONGESTION_CEILING",congestionCeiling)
  connection.ModifyXml("LPR_MODE_CHANGE_REQUEST","MODE_TIMEOUT",modeTime)
  connection.Send()  
  

#------------------------------------------------------------------------------
def DeActivateLpr(connection, partyName, congestionCeiling = 19000):
    
  connection.LoadXmlFile('Scripts/SimLprModeChangeRequest1.xml')
  connection.ModifyXml("LPR_MODE_CHANGE_REQUEST","PARTY_NAME",partyName)
  connection.ModifyXml("LPR_MODE_CHANGE_REQUEST","CONGESTION_CEILING",congestionCeiling)
  connection.Send()  
  
  
#------------------------------------------------------------------------------
def DelSimParty(connection,partyName):
    print "Deleting SIM party "+partyName+"..."
    connection.LoadXmlFile("Scripts/SimDel323Party.xml")
    connection.ModifyXml("H323_PARTY_DEL","PARTY_NAME",partyName)
    connection.Send()
    return

#------------------------------------------------------------------------------
def TestLprFunctionality(connection,num_retries, confName, numberOfParties = 3):
    #Create conf with N paries
#    confName = "VideoLprCpConf"
    connection.CreateConf(confName, 'Scripts/AddCpLprConf.xml')
    #LiveVideo
    
    confid = connection.WaitConfCreated(confName,num_retries)

    ### Add parties 
    for x in range(numberOfParties):
        partyname = confName + "_Party"+str(x+1)
        partyIp = "1.2.3."+str(x+1)
        print "Adding Party " + partyname + ", with ip= " + partyIp+ " To " +confName
        connection.AddVideoParty(confid, partyname, partyIp)
        sleep(1)    

    connection.WaitAllPartiesWereAdded(confid,numberOfParties,num_retries*2)
                 
    connection.WaitAllOngoingConnected(confid,num_retries*2)

#    AddDialInParty(connection,"Party3",2,confName,confid,num_retries)
#    connection.WaitAllOngoingConnected(confid,num_retries)

    print "Wait for IVR to complete !!!!!"
    sleep(9) # Wait for IVR connection :7=2 sec for connecting to IVR + 5 for first join Msg(3=the default thet is
             #set for the "first to join msg" + 2 spare decided by the IVR)
             # without this sleep the party is NOT yet connected to the ContentBridge
    connection.WaitAllOngoingNotInIVR(confid)
    return confid

#------------------------------------------------------------------------------
def CreateConfWithLpr(connection, num_retries, numberOfParties, fileName, CallRate = 0, confName = "VideoLprCpConf"):
    #Create conf with N paries
    if (CallRate):
        connection.CreateConfAndSetCallRate(confName, fileName, CallRate)
    else:
        connection.CreateConfWithDisplayName(confName, fileName)       
    #LiveVideo
    
    confid = connection.WaitConfCreated(confName,num_retries)

    ### Add parties 
    for x in range(numberOfParties):
        partyname = confName + "_Party"+str(x+1)
        partyIp = "1.2.3."+str(x+1)
        print "Adding Party " + partyname + ", with ip= " + partyIp+ " To " +confName
        isSip = False
        if(x == 2):
            isSip = True
        connection.AddVideoParty(confid, partyname, partyIp, isSip)
        sleep(1)    

    connection.WaitAllPartiesWereAdded(confid,numberOfParties,num_retries*2)    
    connection.WaitAllOngoingConnected(confid,num_retries*2)

    print "Wait for IVR to complete !!!!!"
    sleep(9) # Wait for IVR connection :7=2 sec for connecting to IVR + 5 for first join Msg(3=the default thet is
             #set for the "first to join msg" + 2 spare decided by the IVR)
             # without this sleep the party is NOT yet connected to the ContentBridge
    connection.WaitAllOngoingNotInIVR(confid)
    return confid

#------------------------------------------------------------------------------
def TestLprFunctionalityWithManipulatePartyIp(connection,num_retries, confName, confNum, numberOfParties = 3):
    #Create conf with N paries
#    confName = "VideoLprCpConf"
    connection.CreateConf(confName, 'Scripts/AddCpLprConf.xml')
    #LiveVideo
    confid = connection.WaitConfCreated(confName,num_retries)
    
    ### Add parties 
    for x in range(numberOfParties):
        partyname = confName + "_Party"+str(x+1)
        partyIp = "1.2.3."+str(confNum*(x+1))
        print "Adding Party " + partyname + ", with ip= " + partyIp+ " To " +confName
        connection.AddVideoParty(confid, partyname, partyIp)
        sleep(1)    

    connection.WaitAllPartiesWereAdded(confid,numberOfParties,num_retries*2) 
    connection.WaitAllOngoingConnected(confid,num_retries*2)

#    AddDialInParty(connection,"Party3",2,confName,confid,num_retries)
#    connection.WaitAllOngoingConnected(confid,num_retries)

    print "Wait for IVR to complete !!!!!"
    sleep(9) # Wait for IVR connection :7=2 sec for connecting to IVR + 5 for first join Msg(3=the default thet is
             #set for the "first to join msg" + 2 spare decided by the IVR)
             # without this sleep the party is NOT yet connected to the ContentBridge
    connection.WaitAllOngoingNotInIVR(confid)
    return confid

#------------------------------------------------------------------------------
def ActivateContentPresentationByPartyNo(connection, partyName, confId, partyNo):
    print
    print "ACTIVATE Presentation," + partyName + " is going to be the content speaker !!" #DIAL_OUT#10002 is going to be the content speaker !!!!!"
    print "==============================================================================="
    ActivatePresentation(connection, partyName, confId, partyNo)
    print

    return

