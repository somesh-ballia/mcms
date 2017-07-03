#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*PROCESSES_NOT_FOR_VALGRIND=GideonSim
#-LONG_SCRIPT_TYPE

#############################################################################
# Test Script For Presentation
# In The Test:
#  1.Send start content command to "DIAL_OUT#10002" party.
#
#  NOTES
#  =====
#  1.Party name must be valid.
#  
# Date: Jun 15, 2014
# By : uria - 
#############################################################################

from McmsConnection import *
from ContentFunctions import *
import string 

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

print
print "ACTIVATE Presentation, DIAL_OUT#10001 is going to be the content speaker !!!!!"
print "==============================================================================="
ActivatePresentation(c,"DIAL_OUT#10001")
print

c.Disconnect()

#------------------------------------------------------------------------------
