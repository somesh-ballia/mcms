#!/mcms/python/bin/python

import os
from McmsConnection import *
from UsersUtils import *




class AlertUtils(McmsConnection):
#---------------------------------------------------------------------------
#   Public functions
#---------------------------------------------------------------------------
    
    def AddAlert(self, alertNum):
        command = "Bin/McuCmd add_aa cdr " + str(alertNum) + " YES"
        os.system(command)



  
    def IsAlertExist(self, processName, taskName,  wantedAlert, expectedNum = 1 ):
        print "AlertUtils::IsAlertExist : " + processName + "::" + taskName +  ", Expected Active Alarm : " + wantedAlert + "(" + str(expectedNum) + ")"
        
        outputFileName = "tmpTerminalCommandOutput.yura"
        commandLine = "Bin/McuCmd @show_aa " + processName  + "> " + outputFileName
        os.system(commandLine)
        
        commandOutput = open(outputFileName);

        wantedActiveAlarmCnt = self.ParseShowAAOutput(commandOutput, taskName, wantedAlert)
        
        commandOutput.close()
        commandLine = "rm " + outputFileName
        os.system(commandLine)
        
        bResult = True
        if(wantedActiveAlarmCnt <> expectedNum):
            bResult = False
            
        message = "AlertUtils::IsAlertExist : "
        if(bResult):
            message = message + "Successed"
        else:
            message = message + "Failed"
        print message
        
        return bResult

    def HowManyAlertsInMcuMngr(self, wantedAlert):
        print "AlertUtils::HowManyAlertsInMcuMngr:  Expected Active Alarm : " + wantedAlert


        outputFileName = "tmpTerminalCommandOutput.yura"
        commandLine = "Bin/McuCmd @all_aa McuMngr > " + outputFileName
        os.system(commandLine)
        
        commandOutput = open(outputFileName);
        
        actualActiveAlarmCnt = self.ParseAllAAOutput(commandOutput, wantedAlert)
        
        commandOutput.close()
        commandLine = "rm " + outputFileName
        os.system(commandLine)

        return actualActiveAlarmCnt
        
    def IsAlertExistInMcuMngr(self, wantedAlert, expectedNum = 1 ):
        print "AlertUtils::IsAlertExistInMcuMngr:  Expected Active Alarm : " + wantedAlert + "(" + str(expectedNum) + ")"

        actualActiveAlarmCnt = self.HowManyAlertsInMcuMngr(wantedAlert)
        print str(actualActiveAlarmCnt) + " alerts found"
        
        bResult = True
        if(actualActiveAlarmCnt <> expectedNum):
            bResult = False
        
        message = "AlertUtils::IsAlertExist : "
        if(bResult):
            message = message + "Successed"
        else:
            message = message + "Failed"
        print message
        
        return bResult

    def IsFaultExist(self, wantedFault, expectedNum = 1 ):
        print "IsFaultExist::IsFaultExist:  Expected Fault : " + wantedFault + "(" + str(expectedNum) + ")"

        outputFileName = "tmpTerminalCommandOutput.yura"
        command = "cat Faults/FaultsShort* | grep " + wantedFault + " | wc -l > " + outputFileName
        os.system(command)
        
        actualFaultCnt = 0
        commandOutput = open(outputFileName);
        line = commandOutput.readline()
        if(line != ""):
            actualFaultCnt = int(line)
            
        commandOutput.close()

        command = "rm " + outputFileName
        os.system(command)
        
        
        ret = False
        message = "IsFaultExist : "
        if(actualFaultCnt == expectedNum):
            ret = True
            message = message + " OK"
        else:
            message = message + " Failed"
            
        print message
        
        return ret
            
#---------------------------------------------------------------------------
#   Private functions
#---------------------------------------------------------------------------
      
    def ParseShowAAOutput(self, commandOutput, taskName, wantedActiveAlarm):

## the output of the show_aa
## ---------------------------------------
## [yurir@f6-lnxdev09 Main]$ ca show_aa cdr
## Command : show_aa   Process: ***** CDR *****
## ErrorHandlerTask task : Ready

## Manager task : Ready
## CDR:TEST_ERROR : 6 2 3047 9 0 - - - Test 1 - - -
## CDR:TEST_ERROR : 6 2 3047 9 0 - - - Test 2 - - -
## CDR:TEST_ERROR : 6 2 3047 9 0 - - - Test 3 - - -
## CDR:TEST_ERROR : 6 2 3047 9 0 - - - Test 4 - - -
## CDR:TEST_ERROR : 6 2 3047 9 0 - - - Test 5 - - -

## Monitor task : Ready


## [yurir@f6-lnxdev09 Main]$

        # ----- finds the wanted task.
        taskNameLen = len(taskName)
        line = "Cucu_lulu"
        while (line != ""):
            line = commandOutput.readline()
            if(line[0:taskNameLen] == taskName):
                break
            
        wantedActiveAlarmCnt = 0
        while (line != ""):
            line = commandOutput.readline()
            
            if(len(line) == 1):
                break
            
            line = line.replace(" : ", ":")
            line = line.replace(" :", ":")
            lineSplit = line.split(":")
        
            activeAlarm = lineSplit[1]
            if(wantedActiveAlarm == activeAlarm):
                wantedActiveAlarmCnt = wantedActiveAlarmCnt + 1

        return wantedActiveAlarmCnt

    def ParseAllAAOutput(self, commandOutput, wantedActiveAlarm):

## CDR:TEST_ERROR : Used for testing the Active Alarms mechanism : 6 2 5010 10 0 - - -Error in Test 1 - - -
## CDR:TEST_ERROR : Used for testing the Active Alarms mechanism : 6 2 5010 10 0 - - -Error in Test 2 - - -
        
        wantedActiveAlarmCnt = 0
        line = "cucu_lulu"
        while (line != ""):
            line = commandOutput.readline()

            line = line.replace(" : ", ":")
            line = line.replace(" :", ":")
            line = line.replace(": ", ":")
            lineSplit = line.split(":")
            
            if(len(lineSplit) < 2):
                continue
            
            activeAlarm = lineSplit[1]
            
            if(wantedActiveAlarm == activeAlarm):
                wantedActiveAlarmCnt = wantedActiveAlarmCnt + 1
            
        return wantedActiveAlarmCnt
    
