#!/mcms/python/bin/python

#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer McuMngr Configurator BackupRestore

#Updated 8/2/2006
#By Yoella

from McmsConnection import *
from BasicUndefinedDialIn import *

#------------------------------------------------------------------------------
def TestUndefinedDailIn(connection,num_of_parties,num_retries):
    BasicUndefinedDialIn(connection,num_of_parties,num_retries,"discoByEMA")

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestUndefinedDailIn(c,
                    3, # num of parties
                    20)# retries                

c.Disconnect()


