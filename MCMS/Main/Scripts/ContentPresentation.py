#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*PROCESSES_NOT_FOR_VALGRIND=GideonSim
#-LONG_SCRIPT_TYPE

#############################################################################
# Test Script For Presentation
# In The Test:
#  1.Connect 2 parties (1st and 2nd Out)
#  2.Activate presentation from the first party while party on IVR and check there is NO content speaker
#  3.Connect 3rd DialIn
#  4.Wait for IVR to finish
#  5.Activate presentation from the first party and check it is the content speaker
#  6.Switch presentation - Activate presentation from the 3rd party (DialIN)and 
#    check that it is the speaker
#  7.Connect 4th party(DialIn) to the conference and check the content speaker didnt changed 
#     and all parties are connected.
#  8.Delete the Seconed Party and check content Speaker and parties are connected.
#  9.Close presentation by Speaker party and check That No Speaker in the conference.
#  10.Activate presentation from 3rd and 4th party and check content speaker is the 4th
#  11.Abort presentation from EMA and check.
#  12.Activate and immidiate Release presentation from 3rd party and check there is NO content speaker
#  13.Activate presentation by 2nd party and check.
#  14.Disconnect the speaker and check
#  15.Activate presentation 4th party and check.
#  16.Delete the current speaker and check parties are connected and NO content speaker.
#  17.Activate presentation by 2nd party and check.
#  18.Add another conference with 3 DialOut parties .
#  19.Activate presentation by 2nd party (New conf) and Check
#  20.Switch pres. to 3rd party conf2.
#  21.Test the Speakers in both confs.

#  NOTES
#  =====
#  1.The test is checking the ContentToken sign in ConfLevel ONLY ,since According to EMA people(Zoe)
#    The content check box in Party Level is taken from conf Level according to party sourceID
#  2.All the parties in this test are Default parties,there is a different test for ContentHCommon

#  
# Date: 12/09/06
# By  : Yoella
# Updated 22/01/2009 - Add ISDN parties 
# By : Eitan - 
#############################################################################
from McmsConnection import *
from ContentFunctions import *
from ISDNFunctions import *
import string 
#------------------------------------------------------------------------------
def TestContentPresentation(connection,num_retries):
    #Create conf with 3 paries
    confName = "ConfTest1"
    connection.CreateConf(confName, 'Scripts/ContentPresentation/AddCpConf.xml')
    
    confid = connection.WaitConfCreated(confName,num_retries)

    AddIsdnDialoutParty(connection,confid,"Party1","10001")
    connection.WaitAllPartiesWereAdded(confid,1,num_retries*2)
    connection.WaitAllOngoingConnected(confid,num_retries*2)
    connection.WaitAllOngoingNotInIVR(confid,num_retries)	
    print

    ### Add parties   
    for x in range(2):
        partyname = "Party"+str(x+2)
        partyIp = "1.2.3."+str(x+2)
        print "Adding Party " + partyname + ", with ip= " + partyIp+ " To " +confName
        connection.AddVideoParty(confid, partyname, partyIp)
        sleep(1)    

    connection.WaitAllPartiesWereAdded(confid,3,num_retries*2)
    connection.WaitAllOngoingConnected(confid,num_retries*2)
    
    print	
    print "ACTIVATE Presentation, DIAL_OUT#10002 TRYING to be the content speaker While on IVR!!!!!"
    print "========================================================================================"
    ActivatePresentation(connection,"DIAL_OUT#10002")
    CheckNoPresentation(connection,confid,num_retries)
    print

    connection.WaitAllOngoingNotInIVR(confid,num_retries)
    print	    
    sleep(4)
			
    AddIsdnDialoutParty(connection,confid,"Party4","10004")
       	
    connection.WaitAllPartiesWereAdded(confid,4,num_retries*2)
    connection.WaitAllOngoingConnected(confid,num_retries*2)      
    print
    AddDialInParty(connection,"Party5",4,confName,confid,num_retries)
    connection.WaitAllOngoingConnected(confid,num_retries)
    	
    print "Wait for IVR to complete !!!!!"
    sleep(9) # Wait for IVR connection :7=2 sec for connecting to IVR + 5 for first join Msg(3=the default thet is
             #set for the "first to join msg" + 2 spare decided by the IVR)
             # without this sleep the party is NOT yet connected to the ContentBridge
    connection.WaitAllOngoingNotInIVR(confid,num_retries)
    sleep(2)
 
    print
    print "ACTIVATE Presentation, DIAL_OUT#10002 is going to be the content speaker !!!!!"
    print "==============================================================================="
    ActivatePresentation(connection,"DIAL_OUT#10002")
    CheckPresentationSpeaker(connection,"DIAL_OUT#10002",confid,2,num_retries*5)
  
    print
    print "SWITCH Presentation, DIAL_OUT#10001 (ISDN) is going to be the content speaker !!!!!"
    print "======================================================================"	
    ActivatePresentation(connection,"DIAL_OUT#10001")
    CheckPresentationSpeaker(connection,"DIAL_OUT#10001",confid,1,num_retries*5)	

    print
    print "SWITCH Presentation, Party5 is going to be the content speaker !!!!!"
    print "======================================================================"
    ActivatePresentation(connection,"Party5")
    CheckPresentationSpeaker(connection,"Party5",confid,5,num_retries*5)
           
    print
    AddDialInParty(connection,"Party6",5,confName,confid,num_retries)
    connection.WaitAllOngoingConnected(confid,num_retries*4)
    
    print
    print "Wait for IVR to complete !!!!!"
    sleep(2) # Wait for IVR connection :2 sec for connecting to IVR 
             # without this sleep the party is NOT yet connected to the ContentBridge   
    connection.WaitAllOngoingNotInIVR(confid,num_retries)
    sleep(2)
    
    print
    print "Check speaker after ADDING party to conf !!!!!"
    print "================================================"
    CheckPresentationSpeaker(connection,"Party5",confid,5,num_retries*5)
            
    print "Delete 2nd party"
    connection.DeleteParty(confid,3)
    print
    print "Check speaker after DELETING party from conf !!!!!"
    print "====================================================="
    CheckPresentationSpeaker(connection,"Party5",confid,5,num_retries*5)
    
    print
    print "SWITCH Presentation, DIAL_OUT#10004 (ISDN) is going to be the content speaker !!!!!"
    print "======================================================================"	
    ActivatePresentation(connection,"DIAL_OUT#10004")
    CheckPresentationSpeaker(connection,"DIAL_OUT#10004",confid,4,num_retries*5)	          

    print
    print "CLOSE presentation !!!!!"
    print "========================="
    ClosePresentation(connection,"DIAL_OUT#10004")
    CheckNoPresentation(connection,confid,num_retries)
    
    print	    
    print "Activate PARALLEL Token Requests "
    print "=================================="
    ActivatePresentation(connection,"Party5")
    ActivatePresentation(connection,"Party6")
    CheckPresentationSpeaker(connection,"Party6",confid,6,num_retries*5)

    print
    print "Abort Presentation from EMA"
    print "=========================+=="
    connection.LoadXmlFile('Scripts/ContentPresentation/AbortContentPresentation.xml')
    connection.ModifyXml("WITHDRAW_CONTENT_TOKEN","ID",confid)
    connection.Send()
    CheckNoPresentation(connection,confid,num_retries)
        
    print
    print "Activate and IMMEDIATE Release Presentation "
    print "============================================="
    ActivatePresentation(connection,"Party5")
    sleep(0.5)
    ClosePresentation(connection,"Party5")
    CheckNoPresentation(connection,confid,num_retries)
        
    print
    print "ACTIVATE Presentation, Party2 is going to be the content speaker !!!!!"
    print "========================================================================"
    partyId=2
    ActivatePresentation(connection,"DIAL_OUT#10002")
    CheckPresentationSpeaker(connection,"DIAL_OUT#10002",confid,partyId,num_retries*3)
    connection.WaitAllOngoingConnected(confid,num_retries*3)
    
    print
    print "Disconnect Presentation Speaker!!!!!"
    print "====================================="
    connection.LoadXmlFile('Scripts/ContentPresentation/TransSetConnect.xml')
    connection.ModifyXml("SET_CONNECT","ID",confid)
    connection.ModifyXml("SET_CONNECT","CONNECT","false")
    connection.ModifyXml("SET_CONNECT","PARTY_ID",partyId)
    connection.Send()
    connection.WaitPartyDisConnected(confid,partyId,num_retries)
    
    print
    print "Connect Last Presentation Speaker!!!!!"
    print "======================================"
    connection.LoadXmlFile('Scripts/ContentPresentation/TransSetConnect.xml')
    connection.ModifyXml("SET_CONNECT","ID",confid)
    connection.ModifyXml("SET_CONNECT","CONNECT","true")
    connection.ModifyXml("SET_CONNECT","PARTY_ID",partyId)
    connection.Send()
    connection.WaitAllOngoingConnected(confid,num_retries*3)
    #Now EPSim PartyName is DIAL_OUT#10007

    sleep(2)
    connection.WaitAllOngoingNotInIVR(confid,num_retries)
    sleep(2)
    
    print
    print "ACTIVATE Presentation, Party4 is going to be the content speaker !!!!!"
    print "==============================================================================="
    ActivatePresentation(connection,"Party6")
    CheckPresentationSpeaker(connection,"Party6",confid,6,num_retries*5)
    connection.WaitAllOngoingConnected(confid,num_retries*3)
    
    print
    print "DELETE Presentation speaker !!!!!"
    print "=================================="
    DelSimParty(connection,"Party6")
    connection.WaitAllOngoingConnected(confid,num_retries*2)
    sleep(2)
    #Now EPSim PartyName for Party1 is DIAL_OUT#10007 
    print
    print "ACTIVATE Presentation, Party2 is going to be the content speaker !!!!!"
    print "========================================================================"
    partyId=2
    ActivatePresentation(connection,"DIAL_OUT#10007")
    CheckPresentationSpeaker(connection,"DIAL_OUT#10007",confid,partyId,num_retries)
    connection.WaitAllOngoingConnected(confid,num_retries*2)
        
    print
    confName2 = "ConfTest2"
    connection.CreateConf(confName2, 'Scripts/ContentPresentation/AddCpConf.xml')
    
    confid2 = connection.WaitConfCreated(confName2,num_retries)

    ### Add parties   
    for x in range(4):
	partyname = "Party2"+str(x+1)
        if (x % 2 == 1):
		partyIp = "1.2.3."+str(x+1)
        	print "Adding Party " + partyname + ", with ip= " + partyIp+ " To " +confName2
        	connection.AddVideoParty(confid2, partyname, partyIp)
        else:
		partyPhone="2000"+str(x+1)
		AddIsdnDialoutParty(connection,confid2,partyname,partyPhone)
        sleep(2)
        
    connection.WaitAllPartiesWereAdded(confid2,4,num_retries*3)             
    connection.WaitAllOngoingConnected(confid2,num_retries*3)
          
    print "Wait for IVR to complete !!!!!"
    sleep(7) # Wait for IVR connection :7=2 sec for connecting to IVR + 5 for first join Msg(3=the default thet is
             #set for the "first to join msg" + 2 spare decided by the IVR)
             # without this sleep the party is NOT yet connected to the ContentBridge     
    connection.WaitAllOngoingNotInIVR(confid,num_retries)
    sleep(2)

    print
    print "ACTIVATE Presentation, DIAL_OUT#10009 is going to be the content speaker !!!!!"
    print "==============================================================================="
    ActivatePresentation(connection,"DIAL_OUT#10009")
    partyId=0
    CheckPresentationSpeaker(connection,"DIAL_OUT#10009",confid2,2,num_retries*5)
   
    print
    print "SWITCH Presentation, DIAL_OUT#10008(ISDN) is going to be the content speaker !!!!!"
    print "======================================================================"
    ActivatePresentation(connection,"DIAL_OUT#10008")
    partyId=1
    CheckPresentationSpeaker(connection,"DIAL_OUT#10008",confid2,partyId,num_retries*5)
    partyId=2
    CheckPresentationSpeaker(connection,"DIAL_OUT#10007",confid,partyId,num_retries*5)
        
    """
    print
    print "Clean The Confs"
    DelSimParty(connection,"Party3")
    #Delete Party1 id=0
    connection.DeleteParty(confid,0)
    connection.WaitUntillPartyDeleted(confid,num_retries*2)
    
    connection.DeleteParty(confid2,0)
    connection.DeleteParty(confid2,1)
    connection.DeleteParty(confid2,2)
    connection.WaitUntillPartyDeleted(confid2,num_retries*3)
    """
    connection.DeleteConf(confid)
    connection.DeleteConf(confid2)
    connection.WaitAllConfEnd()
    return

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestContentPresentation(c,50)# retries
c.Disconnect()

#------------------------------------------------------------------------------
