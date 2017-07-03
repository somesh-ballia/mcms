#!/mcms/python/bin/python

#############################################################################
# Test Script which Creating a Conf with 3 Sip URI participants
#
# Date: 4/01/05
# By  : Ron S.

#############################################################################

from McmsConnection import *

c = McmsConnection()
c.Connect()
print "Starting test: adding 3 SIP URI participants ..."
c.SimpleXmlConfPartyTest('Scripts/AddVoipConf.xml',
                         'Scripts/CreateConfWith3SipURIParties/AddSipURIParty.xml',
                         3,
                         60)
c.Disconnect()
