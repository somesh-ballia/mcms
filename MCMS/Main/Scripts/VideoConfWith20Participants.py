#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9

# For list of processes not for valgrind look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=MplApi EndpointsSim GideonSim

from McmsConnection import *

c = McmsConnection()
c.Connect()
delayBetweenParticipants = 0
if(c.IsProcessUnderValgrind("ConfParty")):
    delayBetweenParticipants = 3
c.SimpleXmlConfPartyTest('Scripts/AddVideoCpConf.xml',
                         'Scripts/AddVideoParty1.xml',
                         20,
                         50,"TRUE","NONE",delayBetweenParticipants)
c.Disconnect()
