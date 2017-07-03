#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

import os
import sys




def IsFaultExist(faultName):
    command = 'cat Faults/* | grep ' + faultName + ' > ' + GlobalOutputFileName
    os.system(command)
    outputFileSize = os.path.getsize(GlobalOutputFileName)

    commandLine = "rm " + GlobalOutputFileName
#    os.system(commandLine)
    
    if(0 == outputFileSize):
        return False
    return True


    


#----------------------------------------------------------------------
# start test
#----------------------------------------------------------------------


GlobalOutputFileName = "tmpTerminalCommandOutput.yura"
faultName = 'WILD_RESET_IND'

if(True == IsFaultExist(faultName)):
    print '1 Fault " + faultName + " exist'
    exit(1)


IndFileName = "States/McmsDaemonNeatlyFileInd"
if(False == os.path.exists(IndFileName)):
    print '2 File ' + IndFileName + "does not exist"
    exit(2)


command = "Scripts/Destroy.sh"
os.system(command)

if(True == os.path.exists(IndFileName)):
    print '3 File ' + IndFileName + "exist"
    exit(3)


os.environ["CLEAN_STATES"]="NO"

command = "Scripts/Startup.sh"
os.system(command)

# there is [cl] inside startup.
os.system(command)


if(False == IsFaultExist(faultName)):
    print '4 Fault " + faultName + " does not exist'
    exit(1)

if(False == os.path.exists(IndFileName)):
    print '2 File ' + IndFileName + "does not exist"
    exit(2)

