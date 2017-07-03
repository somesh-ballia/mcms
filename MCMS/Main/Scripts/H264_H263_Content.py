#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
# Test Script For Presentation
# Test Auto content protocol - Active content from H323 party with H264 content
# connect ISDN party that cause the content to stop 
# Activate the content again and don't stop the content
# Date: 5/11/08
# By  : Inga
#############################################################################
from McmsConnection import *
from ContentFunctions import *
import string 


#------------------------------------------------------------------------------
def Add263FixProfile(self, profileName, transfer_rate,presentation_protocol, fileName="Scripts/H263Content/Add263ContentProfile.xml"):
    """Create new Profile.
        
    profileName - name of profile to be created.
    fileName - XML file
    """
    print "Adding new Profile..."
 #   self.LoadXmlFile('Scripts/HD/XML/AddHdProfile.xml')
    self.LoadXmlFile("Scripts/H263Content/Add263ContentProfile.xml")
    self.ModifyXml("RESERVATION","NAME", profileName)
    self.ModifyXml("RESERVATION","TRANSFER_RATE", transfer_rate)
    self.ModifyXml("RESERVATION","ENTERPRISE_PROTOCOL", presentation_protocol)
    self.Send()
    ProfId = self.GetTextUnder("RESERVATION","ID")
    print "Profile, named: " + profileName + " ,ID = " + ProfId + ", is added"
    return ProfId
    

#-----------------------------------------------------------------------------
def AddISDN_DialoutParty(connection,confId,partyName,phone):
    print "Adding ISDN dilaout party: "+partyName+ ", from EMA with phone: " + phone
    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    connection.ModifyXml("PARTY","NAME",partyName)
    connection.ModifyXml("ADD_PARTY","ID",confId)
    connection.ModifyXml("PHONE_LIST","PHONE1",phone)
    connection.Send()

#------------------------------------------------------------------------------
def AddDialInParty(connection,partyname,partyid,confName,confid,num_retries,capName="FULL CAPSET"):
    connection.SimulationAddH323Party(partyname, confName,capName)
    connection.SimulationConnectH323Party(partyname)
    #Get the party id
    currPartyID = connection.GetCurrPartyID(connection, confid,partyid,num_retries)
    if (currPartyID < 0):
       connection.Disconnect()                
       sys.exit("Error:Can not find partry id of party: "+partyname)
    print "party id="+str(currPartyID)+" found in Conf"
  
#------------------------------------------------------------------------------

def TestContentProtocol(connection,num_retries):
    
    print
    print "          TestAutoContentProtocol                                              "
    print "==============================================================================="
    
    
#Create conf with 1 party
    confName = "Auto_ConfTest"
    connection.CreateConf(confName, 'Scripts/ContentPresentation/AddCpConf.xml')
    
    confid = connection.WaitConfCreated(confName,num_retries)
    
    partyname = "H264Party_1"
    partyIp = "1.2.3.4"
    print "Adding Party " + partyname + ", with ip= " + partyIp+ " To " +confName
    connection.AddVideoParty(confid, partyname, partyIp)
    sleep(1)    

    connection.WaitAllPartiesWereAdded(confid,1,num_retries*2)
    
    connection.WaitAllOngoingConnected(confid,num_retries*2)
        
    print "Wait for IVR to complete !!!!!"
    sleep(9) # Wait for IVR connection :7=2 sec for connecting to IVR + 5 for first join Msg(3=the default thet is
    
    connection.WaitAllOngoingNotInIVR(confid,num_retries)
    sleep(2)       

    print
    print "ACTIVATE Presentation, H264Party_1 is going to be the content speaker !!!!!"
    print "==============================================================================="
    ActivatePresentation(connection,"DIAL_OUT#10001")
    CheckPresentationSpeaker(connection,"DIAL_OUT#10001",confid,1,num_retries*5)
    
    print "Add dial in H264 party !!!!!"
    AddDialInParty(connection,"H264Party_2",1,confName,confid,num_retries)
    connection.WaitAllOngoingConnected(confid,num_retries)      
    
    print
    print "Wait for IVR to complete !!!!!"
    sleep(2) # Wait for IVR connection :2 sec for connecting to IVR 
             # without this sleep the party is NOT yet connected to the ContentBridge   
    connection.WaitAllOngoingNotInIVR(confid,num_retries)
    sleep(2) 
    
    CheckPresentationSpeaker(connection,"DIAL_OUT#10001",confid,1,num_retries*5)
     
    print
    print "Add ISDN party to conf !!!!!"
    print "==============================================================================="
    # Add Isdn party
    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    partynameIsdn = "IsdnParty "
    phone ="3333"
    AddISDN_DialoutParty(connection,confid,partynameIsdn,phone) 
     
    connection.WaitAllOngoingConnected(confid,num_retries)      
    
    print
    print "Wait for IVR to complete !!!!!"
    sleep(2) # Wait for IVR connection :2 sec for connecting to IVR 
             # without this sleep the party is NOT yet connected to the ContentBridge   
    connection.WaitAllOngoingNotInIVR(confid,num_retries)
    sleep(2) 
    
    # Presentation should stop because of protocol change.
    CheckNoPresentation(connection,confid,num_retries)
   
    
    print
    print "ACTIVATE Presentation again , H264Party is going to be the content speaker !!!!!"
    print "==============================================================================="
    ActivatePresentation(connection,"DIAL_OUT#10001")
    CheckPresentationSpeaker(connection,"DIAL_OUT#10001",confid,1,num_retries*5)
    
    print
    print "SWITCH Presentation, Party is going to be the content speaker !!!!!"
    print "======================================================================"
    ActivatePresentation(connection,"H264Party_2")
    CheckPresentationSpeaker(connection,"H264Party_2",confid,2,num_retries*5)
       
    
    print "Disconnect and delete Isdn party"
    print "======================================================================"
    party_id = connection.GetPartyId(confid, partynameIsdn)
    connection.DisconnectParty(confid,party_id)
    connection.DeleteParty(confid,party_id)
    sleep(1)
    
    print
    print "Check speaker after DELETING ISDN party from conf !!!!!"
    print "====================================================="
    CheckPresentationSpeaker(connection,"H264Party_2",confid,2,num_retries*5)
     
    print
    print "CLOSE presentation !!!!!"
    print "========================="
    ClosePresentation(connection,"H264Party_2")
    CheckNoPresentation(connection,confid,num_retries) 
     
    
    print
    print "Add ISDN party to conf !!!!!"
    print "==========================================="
    # Add Isdn party
    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    partynameIsdn = "IsdnParty1 "
    phone ="3334"
    AddISDN_DialoutParty(connection,confid,partynameIsdn,phone) 
     
    connection.WaitAllOngoingConnected(confid,num_retries)      
    
    print
    print "Wait for IVR to complete !!!!!"
    sleep(2) # Wait for IVR connection :2 sec for connecting to IVR 
             # without this sleep the party is NOT yet connected to the ContentBridge   
    connection.WaitAllOngoingNotInIVR(confid,num_retries)
    sleep(2) 
    
         
    print "Disconnect 323 dial out party"
    print "=============================================="
    c.SimulationDisconnectH323Party("H264Party_1")
      
    print "Delete 323 dial in party"
    print "=============================================="
    connection.DisconnectParty(confid,1)
    connection.DeleteParty(confid,1)
    sleep(1) 
 
    print "Delete Isdn party"
    party_id = connection.GetPartyId(confid, partynameIsdn)
    connection.DisconnectParty(confid,party_id)
    connection.DeleteParty(confid,party_id)
    sleep(1)
     
    connection.DeleteConf(confid)
    connection.WaitAllConfEnd()
    


#-----------------------------------------------------------------------------

#def TestH263FixContentProtocol(connection,num_retries):

    print
    print
    print "          TestH263FixContentProtocol                                              "
    print "==============================================================================="
    
   #Create Profile
    num_retries = 10
    conf_name = "H263Fix_conf"
    presentation_protocol = "h.263"
    speaker_id_interval = 0

    # start conference
    prof_id = Add263FixProfile(connection,"H263Fix_profile","1920","h.263", "Scripts/H263Content/Add263ContentProfile.xml")
    connection.CreateConfFromProfile(conf_name, prof_id)
    confid = connection.WaitConfCreated(conf_name,num_retries)

    # connect participants
    partyname = "Party_1"
    partyIp = "1.2.3.5"
    print "Adding Party " + partyname + ", with ip= " + partyIp+ " To " +conf_name
    connection.AddVideoParty(confid, partyname, partyIp)
    sleep(1)    

    connection.WaitAllPartiesWereAdded(confid,1,num_retries*2)
    
    connection.WaitAllOngoingConnected(confid,num_retries*2)
        
    print "Wait for IVR to complete !!!!!"
    sleep(9) # Wait for IVR connection :7=2 sec for connecting to IVR + 5 for first join Msg(3=the default thet is
     
    connection.WaitAllOngoingNotInIVR(confid,num_retries)
    sleep(2)       

    print
    print "ACTIVATE Presentation, Party_1 is going to be the content speaker !!!!!"
    print "==============================================================================="
    ActivatePresentation(connection,"DIAL_OUT#10005")
    CheckPresentationSpeaker(connection,"DIAL_OUT#10005",confid,1,num_retries*5)
   
    print
    print "Add dial in party" 
    AddDialInParty(connection,"Party_2",1,conf_name,confid,num_retries)
    connection.WaitAllOngoingConnected(confid,num_retries)      
    
    print "Wait for IVR to complete !!!!!"
    sleep(2) # Wait for IVR connection :2 sec for connecting to IVR 
             # without this sleep the party is NOT yet connected to the ContentBridge   
    connection.WaitAllOngoingNotInIVR(confid,num_retries)
    sleep(2) 
    
    print
    print "Check presentation .... !!!!!"
    print "====================================================="
    CheckPresentationSpeaker(connection,"DIAL_OUT#10005",confid,1,num_retries*5)
 
    print
    print "Add ISDN party to conf !!!!!"
    print "==============================================================================="
   
   # Add Isdn party
    connection.LoadXmlFile("Scripts/ISDN_Party.xml")
    partynameIsdn = "IsdnParty "
    phone ="3333"
    AddISDN_DialoutParty(connection,confid,partynameIsdn,phone) 
     
    connection.WaitAllOngoingConnected(confid,num_retries)      
    
    print
    print "Wait for IVR to complete !!!!!"
    sleep(2) # Wait for IVR connection :2 sec for connecting to IVR 
            # without this sleep the party is NOT yet connected to the ContentBridge   
    connection.WaitAllOngoingNotInIVR(confid,num_retries)
    sleep(2) 
    
    # Presentation should stop because of protocol change.
    CheckPresentationSpeaker(connection,"DIAL_OUT#10005",confid,1,num_retries*5)
 
     
    connection.DeleteConf(confid)
    connection.WaitAllConfEnd()
    return
     
   
    
## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestContentProtocol(c,50)# retries
#TestH263FixContentProtocol(c,50)# retries

c.Disconnect()

#------------------------------------------------------------------------------
