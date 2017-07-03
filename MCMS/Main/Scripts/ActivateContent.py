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
#  9.Close presentation by Speaker party and check That No Speaker in the conference.

#  NOTES
#  =====
#  1.The test is checking the ContentToken sign in ConfLevel ONLY ,since According to EMA people(Zoe)
#    The content check box in Party Level is taken from conf Level according to party sourceID
#  2.All the parties in this test are Default parties,there is a different test for ContentHCommon

#  
# Date: Jun 15, 2014
# By : uria - 
#############################################################################
from McmsConnection import *
from ContentFunctions import *
from ISDNFunctions import *
import string 

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

print
print "ACTIVATE Presentation, DIAL_OUT#10002 is going to be the content speaker !!!!!"
print "==============================================================================="
ActivatePresentation(c,"DIAL_OUT#10002")
print

c.Disconnect()

#------------------------------------------------------------------------------
