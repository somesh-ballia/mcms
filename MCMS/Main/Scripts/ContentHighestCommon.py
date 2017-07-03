#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
# Test Script For Presentation (HighestCommon)
# In The Test:
#  1.Connect 2 DailOut parties RATE 384
#  2.Connect DailIn RATE 128
#  3.Activate presentation from the first party and check it is the content speaker
#    (This is a very simple test when monitoring bug we have to check the content rate in the party monitoring)
#  4.Switch presentation - Activate presentation from the 3rd party (DialIn)and 
#    check that it is the speaker
#  5.Delete the 3rd party
#  6.Activate Presentation from the 2nd party
#  7.Connect 3rd DailIn RATE 128 (Check that 2nd party is the speaker)
#  8.Connect DailIn WITHOUT content caps and check all are connected.
#  9.Switch presentation - Activate presentation from the 3rd party (DialIn)and 
#    check that it is the speaker
#  10.Close Presentation and check there is NO content speaker.
##############################################################################  
# Date: 26/09/06
# By  : Yoella
#############################################################################
from McmsConnection import *
from ContentFunctions import *
import string 
#------------------------------------------------------------------------------
def TestContentHighestCommon(connection,num_retries):
    #Create conf with 3 paries
    confName = "ConfTest"
    connection.CreateConf(confName, 'Scripts/ContentPresentation/AddCpConf.xml')
    
    confid = connection.WaitConfCreated(confName,num_retries)

    ### Add parties   
    for x in range(2):
        partyname = "Party"+str(x+1)
        partyIp = "1.2.3."+str(x+1)
        print "Adding Party " + partyname + ", with ip= " + partyIp+ " To " +confName
        connection.AddVideoParty(confid, partyname, partyIp)
        sleep(2)
        
    connection.WaitAllPartiesWereAdded(confid,2,num_retries*2)
    print "Wait for IVR to complete !!!!!"
    sleep(7) # Wait for IVR connection :7=2 sec for connecting to IVR + 5 for first join Msg(3=the default thet is
             #set for the "first to join msg" + 2 spare decided by the IVR)
             # without this sleep the party is NOT yet connected to the ContentBridge 
    connection.WaitAllOngoingNotInIVR(confid)
    sleep(2)

    connection.LoadXmlFile('Scripts/ContentPresentation/SimAddCapSet.xml')
    connection.ModifyXml("ADD_CAP_SET","NAME","CapSet128")
    connection.ModifyXml("ADD_CAP_SET","CALL_RATE","128")
    connection.Send()  
    AddDialInParty(connection,"Party3",2,confName,confid,num_retries,"CapSet128")
            
    connection.WaitAllOngoingConnected(confid,num_retries*3)
    print "Wait for IVR to complete !!!!!"
    sleep(2) # Wait for IVR connection :2 sec for connecting to IVR 
    connection.WaitAllOngoingNotInIVR(confid)
    sleep(2)
   
    print
    print "ACTIVATE Presentation, DIAL_OUT#10001 is going to be the content speaker !!!!!"
    print "==============================================================================="
    ActivatePresentation(connection,"DIAL_OUT#10001")
    CheckPresentationSpeaker(connection,"DIAL_OUT#10001",confid,1,num_retries*5)
       
    print
    print "SWITCH Presentation, Party3 is going to be the content speaker !!!!!"
    print "======================================================================"
    ActivatePresentation(connection,"Party3")
    CheckPresentationSpeaker(connection,"Party3",confid,3,num_retries*5)
               
    print
    print "Delete 3rd party"
    connection.DeleteParty(confid,3)
    DelSimParty(connection,"Party3")
    print "Check No speaker !!!!!"
    print "====================================================="
    CheckNoPresentation(connection,confid,num_retries)
    
    print
    print "ACTIVATE Presentation, DIAL_OUT#10002 is going to be the content speaker !!!!!"
    print "==============================================================================="
    ActivatePresentation(connection,"DIAL_OUT#10002")
    CheckPresentationSpeaker(connection,"DIAL_OUT#10002",confid,2,num_retries*5)

    AddDialInParty(connection,"Party4",2,confName,confid,num_retries,"CapSet128")   
    connection.WaitAllOngoingConnected(confid,num_retries*3)
    connection.WaitAllOngoingNotInIVR(confid)    
    sleep(2)
    
    print
    print "Check speaker after ADDING CapSet128 party to conf !!!!!"
    print "================================================"
    CheckPresentationSpeaker(connection,"Party2",confid,2,num_retries*5)
            
    print
    print "Add party with NO H239 caps"
    connection.LoadXmlFile('Scripts/ContentPresentation/SimAddCapSet.xml')
    connection.ModifyXml("ADD_CAP_SET","NAME","CapSetNoH239")
    connection.Send()  
    AddDialInParty(connection,"Party5",3,confName,confid,num_retries,"CapSetNoH239")
            
    connection.WaitAllOngoingConnected(confid,num_retries*3)
    connection.WaitAllOngoingNotInIVR(confid)    
    sleep(2)
    
    print "Check speaker after ADDING party with NO H239 caps to conf !!!!!"
    print "================================================"
    CheckPresentationSpeaker(connection,"Party2",confid,2,num_retries*5)
        
    print
    print "SWITCH Presentation, Party4 is going to be the content speaker !!!!!"
    print "======================================================================"
    ActivatePresentation(connection,"Party4")
    #sleep(1)
    CheckPresentationSpeaker(connection,"Party4",confid,4,num_retries*5)
        
    print
    print "CLOSE presentation !!!!!"
    print "========================="
    ClosePresentation(connection,"Party4")
    CheckNoPresentation(connection,confid,num_retries)
            
    print
    print "Clean The Conf"
    DelSimParty(connection,"Party4")
    DelSimParty(connection,"Party5")
    
    #Delete Party1,2 id=0,1
    connection.DeleteParty(confid,1)
    connection.DeleteParty(confid,2)
        
    connection.WaitUntillPartyDeleted(confid,num_retries*2)
    
    connection.DeleteConf(confid)
    connection.WaitAllConfEnd()
    return


## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestContentHighestCommon(c,20)# retries
c.Disconnect()

#------------------------------------------------------------------------------
