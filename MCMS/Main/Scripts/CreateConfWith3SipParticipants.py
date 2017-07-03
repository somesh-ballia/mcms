#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
#############################################################################
# Test Script which Creating a Conf with 3 Sip participants
#
# Date: 4/01/05
# By  : Ron S.

#############################################################################

from McmsConnection import *

c = McmsConnection()
c.Connect()
print "Starting test: adding 3 SIP participants ..."
c.SimpleXmlConfPartyTest('Scripts/AddVoipConf.xml',
                         'Scripts/SipAddVideoParty1.xml',
                         3,
                         60)
c.Disconnect()
