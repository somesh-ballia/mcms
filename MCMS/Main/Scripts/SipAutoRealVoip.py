#!/mcms/python/bin/python

#-LONG_SCRIPT_TYPE

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

from McmsConnection import *

c = McmsConnection()
c.Connect()

c.SimpleXmlConfPartyTest('Scripts/AddVoipConf.xml',
                         'Scripts/SipAddVoipParty1.xml',
                         3,
                         60)
c.Disconnect()



