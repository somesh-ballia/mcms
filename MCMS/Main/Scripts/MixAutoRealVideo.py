#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#-LONG_SCRIPT_TYPE
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

from McmsConnection import *

c = McmsConnection()
c.Connect()
c.SimpleXmlConfPartyTest('Scripts/AddVideoCpConf.xml',
                         'Scripts/SipAddVideoParty1.xml',
                         3,
                         60,
                         'TRUE',
                         'Scripts/AddVideoParty1.xml')
                         
c.Disconnect()


