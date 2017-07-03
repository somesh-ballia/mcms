#!/mcms/python/bin/python


#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_12


import os
from McmsConnection import *
from AlertUtils import *


def Sleep(seconds):
    print "Sleep " + str(seconds) + " seconds"
    sleep(seconds)

# 1) add faults without flushing

numAlert = 5
alertUtils = AlertUtils()



command = "Bin/McuCmd add_aa_no_flush cdr " + str(numAlert) + " YES"
os.system(command)

Sleep(10)


faultCheck = alertUtils.IsFaultExist("TEST_ERROR", numAlert)
if(faultCheck == False):
    print "Faults were not added"
    exit(1)

numAAInMcuMngr = alertUtils.HowManyAlertsInMcuMngr("TEST_ERROR")
if(numAAInMcuMngr <> 0):
    print "Alerts were found in McuMngr"
    exit(2)



# 2) perform flush

command = "Bin/McuCmd aa_flush cdr"
os.system(command)

Sleep(10)

aaCheck = alertUtils.IsAlertExistInMcuMngr("TEST_ERROR", numAlert)
if(aaCheck == False):
    print "Not all " + str(numAlert) + " Alerts were found in McuMngr"
    exit(3)


print "Every thing went GREAT"
