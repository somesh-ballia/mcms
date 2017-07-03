#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
# Test Script For Lpr
# In The Test:
#  1.Connect 2 parties (1st in and 2nd Out)
#  2.Connect 1 Dial in party
#  3.Activate LPR from the first party
#  4.Wait till LPR timer pops and see that rate returns to original 
#  5.Activate presentation from the first party and check it is the content speaker
#  6.Activate LPR from the first party
#  7.Close presentation by Speaker party and check That No Speaker in the conference.
#  8.Wait till LPR timer pops and see that rate returns to original 
#  9.Disconnect parties
#  10.Terminate the conference 

#  
# Date: 5/5/08
# By  : GuyD
#############################################################################
from McmsConnection import *
from ContentFunctions import *
from LprFunctions import *
import string 
#------------------------------------------------------------------------------
def TestLprFunctionality(connection,num_retries):
    #Create conf with 3 paries
    confName = "VideoLprCpConf"
    connection.CreateConf(confName, 'Scripts/AddCpLprConf.xml')
    
    confid = connection.WaitConfCreated(confName,num_retries)

    ### Add parties   
    for x in range(2):
        partyname = "Party"+str(x+1)
        partyIp = "1.2.3."+str(x+1)
        print "Adding Party " + partyname + ", with ip= " + partyIp+ " To " +confName
        connection.AddVideoParty(confid, partyname, partyIp)
        sleep(1)    

    connection.WaitAllPartiesWereAdded(confid,2,num_retries*2)
                 
    connection.WaitAllOngoingConnected(confid,num_retries*2)

    AddDialInParty(connection,"Party3",2,confName,confid,num_retries)
    connection.WaitAllOngoingConnected(confid,num_retries)

    print "Wait for IVR to complete !!!!!"
    sleep(9) # Wait for IVR connection :7=2 sec for connecting to IVR + 5 for first join Msg(3=the default thet is
             #set for the "first to join msg" + 2 spare decided by the IVR)
             # without this sleep the party is NOT yet connected to the ContentBridge
    connection.WaitAllOngoingNotInIVR(confid)
 
    print
    print "ACTIVATE LPR, Party1 is going to activate the LPR"
    print "==============================================================================="
    ActivateLpr(connection,"DIAL_OUT#10001")
    sleep(3)
  
    print
    print "ACTIVATE Presentation, Party1 is going to be the content speaker !!!!!"
    print "======================================================================"
    ActivatePresentation(connection,"DIAL_OUT#10001")
    CheckPresentationSpeaker(connection,"DIAL_OUT#10001",confid,1,num_retries*5)

    print
    print "ACTIVATE LPR, Party1 is going to activate the LPR"
    print "==============================================================================="
    ActivateLpr(connection,"DIAL_OUT#10001")           
    
    print
    print "CLOSE presentation !!!!!"
    print "========================="
    ClosePresentation(connection,"DIAL_OUT#10001")
    CheckNoPresentation(connection,confid,num_retries)
        
    print
    print "Clean The Conf"
    DelSimParty(connection,"Party3")
    #Delete Party1 id=0
    connection.DeleteParty(confid,1)
    connection.DeleteParty(confid,2)
    connection.WaitUntillPartyDeleted(confid,num_retries*2)
    connection.DeleteConf(confid)

    connection.WaitAllConfEnd()
    return

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestLprFunctionality(c,20)# retries
c.Disconnect()

#------------------------------------------------------------------------------
