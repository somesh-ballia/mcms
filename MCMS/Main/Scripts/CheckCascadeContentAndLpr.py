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
#  
#AddCpLprConf.py
#Created on: Jun 15, 2014
#     Author: uria
#############################################################################

from McmsConnection import *
from ContentFunctions import *
from LprFunctions import *
import string 
import os


#------------------------------------------------------------------------------
def CascadeBetweenTheConferences(connection, confId, destAlias):
    #Create cascade party.
    #Cascade IP in simulation is hard coded to "1.1.2.2", without that IP the cascade won't work.
    connection.AddCascadeVideoParty(confId, "LinkOut", "1.1.2.2", destAlias)    


## ---------------------- Test --------------------------
def TestCascadeContentAndLpr(c, XmlFileName, num_retries):
    # create two conferences with zero parties.
    # create conference A.
    print "Create LPR conference with 0 parties"
    confAId = CreateConfWithLpr(c, num_retries, 0, XmlFileName, 10240, "VideoLprCpConf1")
    print
    
    # create conference B.
    print "Create LPR conference with 0 parties"
    confBId = CreateConfWithLpr(c, num_retries, 0, XmlFileName, 10240, "VideoLprCpConf2")
    print
    
    #cascade between the two conf - create dial out and cascade between the two conf
    sleep(10)
    print "Create Cascade link from conf A to conf B"
    CascadeBetweenTheConferences(c, confAId, "VideoLprCpConf2")
    print
    
    # add dial out parties to each conf
    sleep(20)
    print
    print "Add dial out parties"
    numberOfParties = 1
    c.AddVideoParty(confAId, "Out1", "0.0.0.1")
    c.AddVideoParty(confBId, "Out2", "0.0.0.2")
    c.WaitAllOngoingDialOutConnected(confAId,numberOfParties,num_retries*2)    
    c.WaitAllOngoingDialOutConnected(confBId,numberOfParties,num_retries*2)    
    #replace the above with function
    c.AddVideoParty(confAId, "Out3", "0.0.1.1")
    c.AddVideoParty(confBId, "Out4", "0.0.2.2")
    
    sleep(10)
    print
    print "ACTIVATE LPR, DIAL_OUT#10002 is going to receive LPR indication"
    print "==============================================================================="
    ActivateLpr(c,"DIAL_OUT#10002", 3, 8, 9600)
    print
    
    sleep(10)
    print
    print "ACTIVATE LPR, DIAL_OUT#10002 is going to receive LPR indication"
    print "==============================================================================="
    ActivateLpr(c,"DIAL_OUT#10002", 3, 8, 8960)
    print

    sleep(10)
    print
    print "ACTIVATE LPR, DIAL_OUT#10002 is going to receive LPR indication"
    print "==============================================================================="
    ActivateLpr(c,"DIAL_OUT#10002", 3, 8, 7680)
    print
    
    sleep(10)
    print
    print "ACTIVATE LPR, DIAL_OUT#10002 is going to receive LPR indication"
    print "==============================================================================="
    ActivateLpr(c,"DIAL_OUT#10002", 3, 8, 7040)
    print
    
    sleep(10)
    print
    print "ACTIVATE LPR, DIAL_OUT#10002 is going to receive LPR indication"
    print "==============================================================================="
    ActivateLpr(c,"DIAL_OUT#10002", 3, 6000, 7040)
    print
    
    sleep(20)
    print
    print "ACTIVATE Presentation, DIAL_OUT#10001 is going to be the content speaker !!!!!"
    print "==============================================================================="
    ActivatePresentation(c,"DIAL_OUT#10001")
    print
    
    sleep(20)
    print
    print "ACTIVATE LPR, DIAL_OUT#10002 is going to receive LPR indication"
    print "==============================================================================="
    ActivateLpr(c,"DIAL_OUT#10002", 10, 8, 7390)
    print
    
    sleep(10)
    print
    print "ACTIVATE LPR, DIAL_OUT#10002 is going to receive LPR indication"
    print "==============================================================================="
    ActivateLpr(c,"DIAL_OUT#10002", 3, 8, 7040)
    print
    
    sleep(20)
    print
    print "Stop Presentation for DIAL_OUT#10001"
    print "==============================================================================="
    ClosePresentation(c,"DIAL_OUT#10001")
    print
    
    sleep(20)
    print
    print "Send another LPR, DIAL_OUT#10002 is going to receive LPR indication"
    print "==============================================================================="
    ActivateLpr(c,"DIAL_OUT#10002", 0, 0, 8960)
    print
    
    sleep(20)

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

#TestCascadeContentAndLpr(c,'Scripts/AddCpLprConfForH263ContentCascade.xml', 20)# retries
TestCascadeContentAndLpr(c,'Scripts/AddCpLprConfForH264ContentCascade.xml', 20)# retries
c.Disconnect()

#------------------------------------------------------------------------------
