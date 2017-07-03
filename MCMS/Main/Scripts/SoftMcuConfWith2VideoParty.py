#!/mcms/python/bin/python

# # NAME: SoftMcuConfWith2VideoParty
# # OWNER: SOFT
# # GROUP: VideoParty
# # NIGHT: FALSE
# # VERSION: TRUE
# # VRESTART_AFTER: FALSE

from McmsConnection import *

c = McmsConnection()
c.Connect()

from SMCUConference import *
from SoftMcuVideoParty import *
c.DeleteAllConf(1)
conf = SMCUConference()
#---Create Conference
conf.Create(c)
#--first party
party1 = SMCUVideoParty()
partyname = "EP1"
#status =party1.Add(conf, partyname, "10.253.66.35", "h323")
status, connected =party1.TryAddFromEP1env(conf, "EP1", partyname, "h323", "h264", "768")
if status != "Status OK":
	ScriptAbort("Failed to add party " + partyname)
partyid1 = party1.partyid
if connected != 1:
	print "EP1 IP seems as not defined, the test is not complete..."
else:
	print "Added party ID: " + str(partyid1)
	sleep(1)
totalParties = connected

#---second party
party2 = SMCUVideoParty()
partyname = "EP2"
#status =party2.Add(conf, partyname, "10.253.66.31", "h323")
status, connected =party2.TryAddFromEP1env(conf, "EP2", partyname, "h323", "h264", "768")
if status != "Status OK":
	ScriptAbort("Failed to add party " + partyname)
partyid2 = party2.partyid
if connected != 1:
	print "EP2 IP seems as not defined, the test is not complete..."
else:
	print "Added party ID: " + str(partyid2)

totalParties += connected
if totalParties > 0:
	c.WaitAllPartiesWereAdded(conf.internalConfId,totalParties,20)
	sleep(30) #let it work 30 sec

#---Delete parties
if partyid1 != -1:
	conf.DeleteParty(partyid1)
	sleep(2)
if partyid2 != -1:
	conf.DeleteParty(partyid2)

conf.connection.DeleteAllConf()
conf.connection.Disconnect()
