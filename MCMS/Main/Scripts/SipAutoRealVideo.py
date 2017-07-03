#!/mcms/python/bin/python

#-LONG_SCRIPT_TYPE

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_21
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

from McmsConnection import *

c = McmsConnection()
c.Connect()
c.SimpleXmlConfPartyTest('Scripts/AddVideoCpConf.xml',
                         'Scripts/SipAddVideoParty1.xml',
                         3,
                         60)
c.Disconnect()


