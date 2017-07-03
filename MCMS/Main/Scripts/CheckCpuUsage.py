#!/mcms/python/bin/python

#############################################################################
# This Script do:
# Date: 15/03/06
# By  : Yuri R.
#############################################################################


from McmsConnection import *
from datetime import date
import os
import sys


ExpectedCpu = { 
"Logger" : 1990,
"McuMngr" : 9,
"CSMngr" : 5,
"ConfParty" : 1120,
"Cards" : 5,
"Resource" :21,
"SipProxy" : 3,
"Ice" : 3,
"DNSAgent" : 3,
"Gatekeeper" : 30,
"QAAPI" : 3,
"CDR" : 72,
"EncryptionKeyServer" : 146,
"Authentication" : 4,
"Configurator" : 3,
"McmsDaemon"  : 6,
"MplApi" : 290,
"CSApi" :  431,
"ApacheModule"  : 20,
"GideonSim" : 572,
"Faults" :  3,
"EndpointsSim"  : 740,
"Installer" : 4,
"IPMCInterface" : 2
}

  

os.system("Scripts/RunTest.sh " + "Scripts/Video40Party5Times.py")

sleep(10) 

outfile = open("/tmp/CheckCpuUsage.txt",'w')

outfile.write("Cpu Usage Report :\n")
outfile.write('Process'.ljust(32)+'Actual'.ljust(10)+'Expected'.ljust(10)+'%Diff\n')

print "Cpu Usage on Startup + 5 * VideoConfWith40Participants.py:"
print ('Process'.ljust(32)+'Actual'.ljust(10)+'Expected'.ljust(10)+'%Diff')

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
  
header = "Command : ps"
headerLen = len(header)
line = "Cucu_lulu"
while (line != ""):
    line = logger.readline()
    if(line[0:headerLen] == header):
        lineName = logger.readline()
        lineCpu  = logger.readline()
        
        listName = lineName.split(" ")
        process  = listName[2]
        
        listCpu = lineCpu.split(" ")
        totalCpu = listCpu[7]
        
        numTotalCpu = int(totalCpu)
        numExpectedCpu = ExpectedCpu[process]
        
        diff =  numTotalCpu - numExpectedCpu
        DiffCpu = (diff * 100) / numExpectedCpu
        
        if ( (abs(diff) > 2) and (abs(DiffCpu) > 4) ):
            sDiffCpu = str(DiffCpu)+'%'
        else:
            sDiffCpu = ''
      
        outfile.write(process.ljust(32)+str(numTotalCpu).ljust(10)+str(ExpectedCpu[process]).ljust(10)+sDiffCpu+'\n')
#        outfile.write(process.ljust(32) + " : " + totalCpu)
           
        print(process.ljust(32)+str(numTotalCpu).ljust(10)+str(ExpectedCpu[process]).ljust(10)+sDiffCpu)
#        print process.ljust(32) + " : " + totalCpu


#Command : ps
#Command Answer:***** Logger *****
#PID: 11917  User: 7  System: 7  Total: 15
    
logger.close()
outfile.close()

    
