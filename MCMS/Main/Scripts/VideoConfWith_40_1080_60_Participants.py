#!/mcms/python/bin/python

#-LONG_SCRIPT_TYPE

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_10

# For list of processes not for valgrind look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=MplApi EndpointsSim GideonSim CSApi Logger Resource CDR Faults McuMngr CSMngr ConfParty EncryptionKeyServer


from McmsConnection import *
from ISDNFunctions import *
import os
import sys
command_line = 'Bin/McuCmd set ConfParty DELAY_BETWEEN_DIAL_OUT_PARTY 0'
os.system(command_line)
sleep(1)
c = McmsConnection()
c.Connect()
isCop = "FALSE"
delayBetweenParticipants = 1
#if(c.IsProcessUnderValgrind("ConfParty")):
#    delayBetweenParticipants = 4
numberOfParties = 10
#if isCop == "TRUE":
#    numberOfParties = 40   # instead of 30 + 10 isdn. no isdn in cop.
#numberOfIsdnParties = 10
timeout = 7

#command_line = "Bin/McuCmd set ConfParty MAX_CP_RESOLUTION CIF" + '\n'
#os.system(command_line)

confid = c.SimpleXmlConfPartyTest_1080_60('Scripts/AddVideoCpConf_1080_60.xml',
                         'Scripts/AddVideoParty_1080_60.xml',
                         numberOfParties,
                         40,
                         'FALSE',            
                         'NONE',
                         timeout,              
                         'FALSE') 

# def SimpleXmlConfPartyTest_1080_60(self,confFile,partyFile,num_of_parties,num_retries,deleteConf="TRUE",secondPartyFile="NONE",delayBetweenParticipants=0,connectPartiesOneByOne="FALSE"):
#TestDialOutISDN(c, confid, numberOfIsdnParties, timeout, "TRUE", delayBetweenParticipants)		# no ISDN in COP version

#command_line = "Bin/McuCmd set ConfParty MAX_CP_RESOLUTION SD30" + '\n'
#os.system(command_line)


c.Disconnect()
