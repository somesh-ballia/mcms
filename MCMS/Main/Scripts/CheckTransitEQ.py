#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_3

#############################################################################
# Test Script which test the Creation of Transit(Default) Eq 
#############################################################################

from McmsConnection import *

def TestTransitEq(connection,num_retries):
    #add a new profile
    ProfId = connection.AddProfile("profile1")
    #Add a new EQ     
    """Create new EQ reservation.
        
    eqName - name of EQ to be created.
    ProfId - Id of profile used by EQ.
    fileName - XML file
    """
    
    eqName1 = "eq1"
    eqName2 = "eq2"
    eqName3 = "1111"
    
    #Add Entry Queue 1 
    print "Adding a new Entry Queue " + eqName1 + " Reservation..."
    connection.LoadXmlFile("Scripts/CheckTransitEQ/AddEQ.xml")
    connection.ModifyXml("RESERVATION","NAME",eqName1)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",ProfId)
    connection.Send()
    
    #Wait untill eq was added
    eqId, eqNID = connection.WaitMRCreated(eqName1,num_retries)
    sleep(3)
 
    #Set eq1 to be the default eq
    print "Set the Entry Queue " + eqName1 + " To be Transit EQ..."
    connection.LoadXmlFile("Scripts/CheckTransitEQ/SetTransitEQ.xml")
    connection.ModifyXml("SET_DEFAULT_EQ","NAME",eqName1)
    connection.Send()  
    sleep(300)
 
    #Try to call in with wrang eq name with sip party 
    num_parties = 1
    ConnectXSimulationSipParties2(connection,eqName2,num_parties,num_retries,5)    
    eqConfId5 = connection.WaitUntillEQorMRAwakes(eqName1,num_parties,num_retries,True)       
    sleep(3)
 
    """ 
    #delete the eq1 from list
    print "Delete the Default Entry Queue " + eqName1 + " From reservation..."
    connection.DelReservation(eqId,"Scripts/CheckTransitEQ/RemoveEQ.xml")
    sleep(80)
    """
    
    
    """ 
    print "Cancel the Defult Entry Queue " + eqName1
    connection.LoadXmlFile("Scripts/CheckTransitEQ/CancelTransitEQ.xml")
    connection.ModifyXml("CANCEL_DEFAULT_EQ","NAME",eqName1)
    connection.Send()  
    sleep(80)
    """
    
     #delete the eq1 conf
    connection.DeleteConf(eqConfId5)
    sleep(3)
  

    #Try to call in with wrang eq name  
    num_parties = 1
    ConnectXSimulationParties(connection,eqName2,num_parties,num_retries)    
    eqConfId = connection.WaitUntillEQorMRAwakes(eqName1,num_parties,num_retries,True)       
    sleep(3)


    #delete the eq1 conf
    connection.DeleteConf(eqConfId)
    sleep(3)
    
    #delete the eq1 from list
    connection.DelReservation(eqId,"Scripts/CheckTransitEQ/RemoveEQ.xml")
    sleep(3)
    
    #Add Entry Queue 2
    print "Adding a new Entry Queue " + eqName2 + " Reservation..."
    connection.LoadXmlFile("Scripts/CheckTransitEQ/AddEQ.xml")
    connection.ModifyXml("RESERVATION","NAME",eqName2)
    connection.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",ProfId)
    connection.Send()
   
     #Wait untill eq was added
    eqId2, eqNID2 = connection.WaitMRCreated(eqName2,num_retries)
    sleep(3)
 
    #Set eq2 to be the default eq
    print "Set the Entry Queue " + eqName2 + " To be Transit EQ..."
    connection.LoadXmlFile("Scripts/CheckTransitEQ/SetTransitEQ.xml")
    connection.ModifyXml("SET_DEFAULT_EQ","NAME",eqName2)
    connection.Send()  
    sleep(3)
    
    #Try to call in with empty eq name  
    num_parties = 1
    ConnectXSimulationParties2(connection,eqName3,num_parties,num_retries,2)    
    eqConfId2 = connection.WaitUntillEQorMRAwakes(eqName2,num_parties,num_retries,True)       
    sleep(3)
 
    #delete the eq2 conf
    connection.DeleteConf(eqConfId2)
    sleep(3)
   

    #Cancel the Defualt EQ
    print "Cancel the Defult Entry Queue " + eqName2
    connection.LoadXmlFile("Scripts/CheckTransitEQ/CancelTransitEQ.xml")
    connection.ModifyXml("CANCEL_DEFAULT_EQ","NAME",eqName2)
    connection.Send()  
    sleep(3)
 
    #Try to call in with empty eq name  
#    num_parties = 1
#    ConnectXSimulationParties2(connection,eqName3,num_parties,num_retries,3)    
#    eqConfId3 = connection.WaitUntillEQorMRAwakes(eqName2,num_parties,num_retries,True)       
#    sleep(3)
   
    # Create a new undefined party and add it to the EPSim
    
    partyName = "EQParty"
    print "Add new undefined party :" + partyName
    connection.SimulationAddH323Party(partyName, eqName2)
    
     #connect the undefined party
    connection.SimulationConnectH323Party(partyName)
    
    #Wait untill Eq was awake and the Ad-Hoc conf will be created
    numOfParties = 1
    eqConfId4 = connection.WaitUntillEQorMRAwakes(eqName2,numOfParties,num_retries,True)
    sleep(3)
    
    
    connection.SimulationDeleteH323Party(partyName)
    
    #delete the eq conf
  #  connection.DeleteConf(eqConfId) 
#    connection.DeleteConf(eqConfId2)
#    connection.DeleteConf(eqConfId3)   
    connection.DeleteConf(eqConfId4)  
    connection.WaitAllConfEnd(num_retries)
    
    #remove the EQ Resrv
    print "Remove the EQ reservation..."
 #   connection.SendXmlFile("Scripts/CheckTransitEQ/RemoveEQ.xml")
    
    #remove the EQ Resrv
    connection.DelReservation(eqId2, "Scripts/AwakeMrByUndef/RemoveMr.xml")
    
    #remove the profile
    connection.DelProfile(ProfId, "Scripts/AdHocConfWithUndefParty1/RemoveNewProfile.xml")
    
    return

#------------------------------------------------------------------------------
def ConnectXSimulationSipParties2(connection,eqName,num_parties,num_retries,y):
    # Create a new undefined party and add it to the EPSim
    for x in range(num_parties):
        partyName = "Party" + str(x+y)
        connection.SimulationAddSipParty(partyName, eqName)
        #connect the undefined party
        connection.SimulationConnectSipParty(partyName)
        
    sleep(3)
    return 

#------------------------------------------------------------------------------
def ConnectXSimulationParties(connection,eqName,num_parties,num_retries):
    # Create a new undefined party and add it to the EPSim
    for x in range(num_parties):
        partyName = "Party" + str(x+1)
        connection.SimulationAddH323Party(partyName, eqName)
        #connect the undefined party
        connection.SimulationConnectH323Party(partyName)
        
    sleep(3)
    return 
#------------------------------------------------------------------------------
def ConnectXSimulationParties2(connection,eqName,num_parties,num_retries,y):
    # Create a new undefined party and add it to the EPSim
    for x in range(num_parties):
        partyName = "Party" + str(x+y)
        connection.SimulationAddH323Party(partyName, eqName)
        #connect the undefined party
        connection.SimulationConnectH323Party(partyName)
        
    sleep(3)
    return 

#------------------------------------------------------------------------------
def DtmfXSimulationParties(connection,adHocConfNumericId,num_parties):
     
    #Send the DTMF which represent the new Ad-hoc conf
    print "Sending DTMF: " + adHocConfNumericId + "to Conf"
    for x in range(num_parties):
        partyName = "Party" + str(x+1)
        connection.SimulationH323PartyDTMF(partyName, adHocConfNumericId)
    return
#------------------------------------------------------------------------------
def RemoveXSimulationParties(connection,num_parties):
     
    #delete the Sim Parties
    for x in range(num_parties):
        partyName = "Party" + str(x+1)
        connection.SimulationDeleteH323Party(partyName)
           
    return

        
## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestTransitEq(c,
             40)# retries

c.Disconnect()


