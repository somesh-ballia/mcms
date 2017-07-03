#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind


import os
import sys

from AlertUtils import *
from SysCfgUtils import *



def CheckHistoryFile(isShouldExist, numOfReset):
    print "CheckHistoryFile " + "is should exist:" + str(isShouldExist) + ", num of resets: " + str(numOfReset)

    # wanted    real   result
    #   0        0       return True
    #   0        1       return False
    #   1        0       return False
    #   1        1       continue

    isExist = IsFileExist(GlobalResetHistoryFile)
    if(False == isShouldExist):
        if(False == isExist):  # row 1
            return True
        else:
            print "0        1       return False"
            return False       # row 2
    else:
        if(False == isExist):  # row 3
            print "1        0       return False"
            return False
#        else:
                               # row 4
    
    
    historyFileName = GlobalResetHistoryFile 
    startupTitle = "Startup: "
    startupTitleLen = len(startupTitle)
    
    file = open(historyFileName);
    line = "Cucu_lulu"
    numOfLines = 0
    while (line != ""):
        line = file.readline()
        if(line[0:startupTitleLen] == startupTitle):
            numOfLines = numOfLines + 1

    file.close()

    ret = False
    if(numOfLines == numOfReset):
        ret = True

    return ret


def StopWatchDog():
    print "StopWatchDog"
    command = "Bin/McuCmd kill CSMngr"
    os.system(command)


def DisableEnableCleanStates(val):
    print "DisableEnableCleanStates " + str(val)
    if(True == val):
        os.environ["CLEAN_STATES"]="YES"
    else:
        os.environ["CLEAN_STATES"]="NO"
 


def StartSystem():
    print "StartSystem"

    print "---------------------------------------------------------"
    print ""
    print ""
    
    command = "Scripts/Startup.sh"
    os.system(command)

    print "---------------------------------------------------------"
    print ""
    print ""


def ShutdownSystem(isAppropriateWay):
    print "ShutdownSystem appropriate = " + str(isAppropriateWay)
    command = "Scripts/"
    if(True == isAppropriateWay):
        command = command + "Destroy.sh"
    else:
        command = command + "Cleanup.sh"
    os.system(command)


def CheckIsSystemDown():
    print "CheckIsSystemDown"
    commandLine = "ps -e | grep McmsDaemon"  + "> " + GlobalOutputFileName
    os.system(commandLine)

    outputFileSize = os.path.getsize(GlobalOutputFileName)
    
    commandLine = "rm " + GlobalOutputFileName
    os.system(commandLine)

    if(0 != outputFileSize):
        return False
    return True


def Sleep(numOfSeconds):
    minutes = numOfSeconds / 60
    seconds = numOfSeconds % 60
    
    print "Sleep " + str(numOfSeconds) + "; " + str(minutes) + ":" + str(seconds)
    sleep(numOfSeconds)


def IsFileExist(fileName):
    return os.path.exists(fileName)

def IsFaultExist(faultName):
    command = 'cat Faults/* | grep ' + faultName + ' > ' + GlobalOutputFileName
    os.system(command)
    outputFileSize = os.path.getsize(GlobalOutputFileName)

    commandLine = "rm " + GlobalOutputFileName
    os.system(commandLine)
    
    if(0 == outputFileSize):
        return False
    return True
    
    
def IsSystemInSafeMode():

    # 1) history file should be deleted
    res = IsFileExist(GlobalResetHistoryFile)
    if(True == res):
        print "Reset history file  [" + GlobalResetHistoryFile + "] exist, but it should not"
        return False
    
    # 2) alarm should exist
    alertsUtils = AlertUtils()
    bAlertCheck = alertsUtils.IsAlertExist("McmsDaemon", "Manager", "SYSTEM_IN_SAFE_MODE", 1)
    if(False == bAlertCheck):
        print "Alarm SYSTEM_IN_SAFE_MODE was not found"
        return False

    # 2.5) fault should exist
    bFaultCheck = IsFaultExist('SYSTEM_IN_SAFE_MODE')
    if(False == bFaultCheck):
        print 'Fault SYSTEM_IN_SAFE_MODE was not found'
        return False
    
    # 3) no reset should be done
    MakeCardsSendResetReqToDaemon()

    Sleep(5)

    alertsUtils = AlertUtils()
    bAlertCheck = alertsUtils.IsAlertExist("McmsDaemon", "Manager", "RESET_MCU_INTERNAL", 1)
    if(False == bAlertCheck):
        print "Alert RESET_MCU_INTERNAL should appear, but it was is not"
        return False


    # 4) no reset should be done
    KillMcmsProcess("CSMngr")
    
    Sleep(20)
    
    bAlertCheck = alertsUtils.IsAlertExist("McmsDaemon", "Manager", "PROCESS_TERMINATED", 1)
    if(False == bAlertCheck):
        print "Alert PROCESS_TERMINATED should appear, but it was is not"
        return False
        
    bAlertCheck = alertsUtils.IsAlertExist("McmsDaemon", "Manager", "RESET_MCU_INTERNAL", 2)
    if(False == bAlertCheck):
        print "Alert RESET_MCU_INTERNAL should appear twice, but it was is not"
        return False
    
    return True


def MakeCardsSendResetReqToDaemon():
    print "MakeCardsSendResetReqToDaemon"
    command = "Bin/McuCmd reset_to_daemon Cards"
    os.system(command)


# 100 ticks = 1 second
def SetDeleteResetHistoryTimeout(strTimeoutInTicks):
    print "SetDeleteResetHistoryTimeout"
    sysCfgUtils = SyscfgUtilities()
    sysCfgUtils.ConnectSpecificUser("SUPPORT", "SUPPORT")
    sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_DEBUG", "RESET_HISTORY_TIME_INTERVAL", "2000", "debug")
    sysCfgUtils.SendSetCfg()


def KillMcmsProcess(mcmsProcess):
    print "KillMcmsProcess " + mcmsProcess
    command = "Bin/McuCmd kill " + mcmsProcess
    os.system(command)

    
#----------------------------------------------------------------------
# start test
#----------------------------------------------------------------------



GlobalResetHistoryFile = "States/McmsDaemonResetHistory.txt"
GlobalOutputFileName = "tmpTerminalCommandOutput.yura"


# First part
#---------------------
# it checks the following:
# 1) addition of startup record at each startup
# 2) enter Safe mode after 5 times
# 3) alert of safe mode appears
# 3) no reset is done in seafe mode

print ""
print ""
print "---------------------------------------------------------"
print "First part"
print "1) 5 startups for entering safe mode"
print "---------------------------------------------------------"
print "" 

DisableEnableCleanStates(False)

# i -[0,4]
for i in range(5):
    
    res = CheckHistoryFile(True, i + 1)
    if (False == res):
        print "reset history was not as expected"
        exit(1)

    ShutdownSystem(False)
#    Sleep(5)
    
    res = CheckIsSystemDown()
    if(False == res):
        print "System should be down but it is not on iteration " + str(i)
        exit(2)
 
    StartSystem()



res = IsSystemInSafeMode()
if(False == res):
    print "System is not in safe mode, but it should be"
    exit(3)



# Second part
#---------------------
# it checks the following:
# 1) Normal shoutdown via Scripts/Destroy.sh (terminal command)
# 2) history should be removed.
# 3) after startup new history should be created.

print ""
print ""
print "---------------------------------------------------------"
print "Second part"
print "1) Normal shutdown"
print "2) Normal startup"
print "3) history should be removed after 5 minutes timeout."
print "---------------------------------------------------------"
print ""

SetDeleteResetHistoryTimeout("2000")
DisableEnableCleanStates(False)


ShutdownSystem(True)
Sleep(5)

res = IsFileExist(GlobalResetHistoryFile)
if (True == res):
    print "reset history file exist, but it should not"
    exit(6)

StartSystem()

res = CheckHistoryFile(True, 1)
if (False == res):
    print "reset history was not as expected"
    exit(7)

Sleep(20)

res = CheckHistoryFile(False, 0)




# print a successfull message

print "----------------------------------------------------------"
print ""
print " All 2 parts were completed successfully"
print " 1) enter safe mode after 5 \"bad\" resets"
print "    a) check that no resets are done in safe mode - reset request, mcms WD" 
print " 2) Normal shutdown + Normal startup + Remove history after timer is gone"
print ""
print " the endless reset feature works OK"
print "----------------------------------------------------------"
print ""

