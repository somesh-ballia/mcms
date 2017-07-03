#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

from McmsConnection import *

c = McmsConnection()
c.Connect()

c.SimpleXmlConfPartyTest('Scripts/AddVoipConf.xml',
                         'Scripts/AddVoipParty1.xml',
                         3,
                         60,
                         'TRUE',
                         'Scripts/SipAddVoipParty1.xml')
c.Disconnect()



