#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#############################################################################
# Test Script For Lpr
# In The Test:
# 1. Create a Conference and connect 3 parties (1st in and 2nd Out)
# 2. Activate content from one of them.
# 3. Activate LPR from another.
# 4. Stop the LPR.
# 5. Stop the content.
# 6. End the Conference
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



## ---------------------- Test --------------------------
#
# Currently test must be reset simulation before any test. 
#
c = McmsConnection()
c.Connect()

# create conferences.
num_retries = 20
print "Create LPR conference with 3 parties"
confId = CreateConfWithLpr(c, num_retries, 4, 'Scripts/AddCpLprConf1.xml', 10240)

delayTime = 3

sleep(delayTime)
print
print "ACTIVATE LPR, DIAL_OUT#10001 is going to receive LPR indication"
print "==============================================================================="
ActivateLpr(c,"DIAL_OUT#10001", 2, 8, 8960)
print

sleep(delayTime)
print
print "ACTIVATE Presentation, DIAL_OUT#10002 is going to be the content speaker !!!!!"
print "==============================================================================="
ActivatePresentation(c,"DIAL_OUT#10002")
print

sleep(delayTime + 1)
print
print "ACTIVATE LPR, DIAL_OUT#10001 is going to receive LPR indication"
print "==============================================================================="
ActivateLpr(c,"DIAL_OUT#10001", 3, 8, 5760)
print

sleep(delayTime)
print
print "ACTIVATE LPR, DIAL_OUT#10002 is going to receive LPR indication"
print "==============================================================================="
ActivateLpr(c,"DIAL_OUT#10002", 3, 8, 8320)
print

sleep(delayTime)
print
print "ACTIVATE LPR, DIAL_OUT#10001 is going to receive LPR indication"
print "==============================================================================="
ActivateLpr(c,"DIAL_OUT#10001", 3, 8, 8100)
print

sleep(delayTime)
print
print "ACTIVATE LPR, DIAL_OUT#10003 (SIP) is going to receive LPR indication"
print "==============================================================================="
ActivateLpr(c,"DIAL_OUT#10003", 3, 8, 6800)
print

sleep(delayTime)
print
print "Stop LPR"
print "==============================================================================="
DeActivateLpr(c,"DIAL_OUT#10001", 8960)
print

sleep(delayTime)
print
print "Stop LPR"
print "==============================================================================="
DeActivateLpr(c,"DIAL_OUT#10002", 8960)
print

sleep(delayTime)
print
print "Stop LPR"
print "==============================================================================="
DeActivateLpr(c,"DIAL_OUT#10003", 8960)
print

sleep(delayTime)
print
print "Stop Content"
print "==============================================================================="
ClosePresentation(c,"DIAL_OUT#10002")
print

sleep(delayTime + 1)
print
print "ACTIVATE Presentation - second time, DIAL_OUT#10002 is going to be the content speaker !!!!!"
print "==============================================================================="
ActivatePresentation(c,"DIAL_OUT#10002")
print

sleep(delayTime + 1)
c.DeleteConf(confId)
c.Disconnect()

#------------------------------------------------------------------------------
