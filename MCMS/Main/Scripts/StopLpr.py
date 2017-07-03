#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*PROCESSES_NOT_FOR_VALGRIND=GideonSim
#-LONG_SCRIPT_TYPE

#############################################################################
# Test Script For LPR
# In The Test:
#  1.Send Stop LPR (return to initial value) command to "DIAL_OUT#10001" party.
#
#  NOTES
#  =====
#  1.Party name must be valid.
#  
# Date: Jun 15, 2014
# By : uria - 
#############################################################################

from McmsConnection import *
from LprFunctions import *
import string 

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

print
print "Stop LPR"
print "==============================================================================="
DeActivateLpr(c,"DIAL_OUT#10002", 8960)
print

c.Disconnect()

#------------------------------------------------------------------------------
