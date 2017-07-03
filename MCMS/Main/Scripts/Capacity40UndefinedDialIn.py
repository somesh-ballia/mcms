#!/mcms/python/bin/python

#-LONG_SCRIPT_TYPE

#Updated 8/2/2006
#By Yoella

# For list of processes not for valgrind look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=Logger GideonSim MplApi CSApi EndpointsSim

import os
from McmsConnection import *
from BasicUndefinedDialIn import *

#------------------------------------------------------------------------------
def TestCapacity40UndefinedDailIn(connection,num_of_parties,num_retries):
    BasicUndefinedDialIn(connection,num_of_parties,num_retries,"discoByParty")

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

command_line = "Bin/McuCmd set ConfParty MAX_CP_RESOLUTION CIF" + '\n'
os.system(command_line)

TestCapacity40UndefinedDailIn(c,
                    40, # num of parties
                    20)# retries
c.Disconnect()


