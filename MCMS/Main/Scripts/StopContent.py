#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*PROCESSES_NOT_FOR_VALGRIND=GideonSim
#-LONG_SCRIPT_TYPE

#############################################################################
# Test Script For Presentation
# In The Test:
#  1.Send stop content command to "DIAL_OUT#10002" party.
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
from ISDNFunctions import *
import string 

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

print
print "Stop Content"
print "==============================================================================="
ClosePresentation(c,"DIAL_OUT#10002")
print

c.Disconnect()

#------------------------------------------------------------------------------
