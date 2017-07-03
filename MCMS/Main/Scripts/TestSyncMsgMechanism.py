#!/mcms/python/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_14

import os
from McmsConnection import *
import sys

#test 1 - CDR Process receives unsuitable response message 

commandLine = "Bin/McuCmd send_sync_msg CDR 5 Logger 10"
os.system(commandLine)

sleep(1)

commandLine = "Bin/McuCmd send_sync_msg CDR 10 Faults 12"
os.system(commandLine)

sleep(19)

#test 2 - CDR Process receives unsuitable response message 

commandLine = "Bin/McuCmd send_sync_msg CDR 3 Logger 7"
os.system(commandLine)

sleep(2)

commandLine = "Bin/McuCmd send_sync_msg CDR 10 Faults 8"
os.system(commandLine)

sleep(15)

commandLine = "Scripts/Flush.sh"
os.system(commandLine)

sleep(2)

LogFilePrefix = "Log_SN"
LogFileSubfix = ".txt"
outputLoggerFileList = "LoggerFileList.txt"

commandLine = "ls -t LogFiles/ | grep " + LogFilePrefix + " | grep " + LogFileSubfix + " > " + outputLoggerFileList
os.system(commandLine)

commandOutput = open(outputLoggerFileList)
LastLoggerFileName = commandOutput.readline()

commandOutput.close()
commandLine = "rm " + outputLoggerFileList
os.system(commandLine)

LineSplit = LastLoggerFileName.split("\n")
LastLoggerFileNameWithoutNewLine = LineSplit[0]
fullname = "LogFiles/" + LastLoggerFileNameWithoutNewLine
logger = open(fullname)

Discard1 = "CTaskApi::SendMessageSync : Process CDR discards the received response message with seq num of 1"
Discard2 = "CTaskApi::SendMessageSync : Process CDR discards the received response message with seq num of 2"
Discard3 = "CTaskApi::SendMessageSync : Process CDR discards the received response message with seq num of 3"

Timer1 = "Process CDR Task Manager waits for rsp message:  send seq num 2 opcode 10 Timeout:"
Timer2 = "Process CDR Task Manager waits for rsp message:  send seq num 3 opcode 10 Timeout:"
Timer3 = "Process CDR Task Manager waits for rsp message:  send seq num 4 opcode 10 Timeout:"

FoundDiscard1 = "NO"
FoundDiscard2 = "NO"
FoundDiscard3 = "NO"

FoundTimer1 = "NO"
FoundTimer2 = "NO"
FoundTimer3 = "NO"

DiscardLen = len(Discard1)
TimerLen = len(Timer1)

line = "Cucu_lulu"
while (line != ""):
     line = logger.readline()
     
     if(line[0:DiscardLen] == Discard1):
		 FoundDiscard1 = "YES"
		 continue
     else:
         if(line[0:DiscardLen] == Discard2):
            FoundDiscard2 = "YES"
            continue
         else:
            if(line[0:DiscardLen] == Discard3):
                FoundDiscard3 = "YES"
                continue
            else:
               if(line[0:TimerLen] == Timer1):
                  listName = line.split(" ")
                  newTimerString  = listName[16]
                  Timer  = int(newTimerString)
                  if( (Timer > 400) and (Timer < 600) ):
                     FoundTimer1 = "YES"
                  continue
               else:
                  if(line[0:TimerLen] == Timer2):
                     listName = line.split(" ")
                     newTimerString  = listName[16]
                     Timer  = int(newTimerString)
                     if( (Timer > 200) and (Timer < 400) ):
                        FoundTimer2 = "YES"
                     continue
                  else:
                      if(line[0:TimerLen] == Timer3):
                         listName = line.split(" ")
                         newTimerString  = listName[16]
                         Timer  = int(newTimerString)
                         if( (Timer > 500) and (Timer < 700) ):
                            FoundTimer3 = "YES"
                         continue
                     

if( (FoundDiscard1 == "YES") and (FoundDiscard2 == "YES") and (FoundDiscard3 == "YES") and (FoundTimer1 == "YES") and (FoundTimer2 == "YES") and (FoundTimer3 == "YES") ):
     print "TestSyncMsgMechanism: SUCCESS"
     sys.exit(0)
else:
     print "TestSyncMsgMechanism: FAILED"		     
     sys.exit(1)
        
     
