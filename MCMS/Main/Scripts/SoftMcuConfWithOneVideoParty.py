#!/mcms/python/bin/python

# # NAME: SoftMcuConfWithOneVideoParty
# # OWNER: SOFT
# # GROUP: VideoParty
# # NIGHT: FALSE
# # VERSION: FALSE
# # VRESTART_AFTER: FALSE

from McmsConnection import *

c = McmsConnection()
c.Connect()

from SMCUConference import *
from SoftMcuVideoParty import *
conf = SMCUConference()
#---Create Conference
conf.Create(c, "VideoConf", "12345")

party = SMCUVideoParty()
partyname = "Party1"
#status =party.Add(conf, partyname, "10.253.66.35", "sip")
status, connected =party.TryAddFromEP1env(conf, "EP1", partyname, "sip")
if (status != "Status OK" or connected != 1):
	conf.Disconnect()
	c.Disconnect()
	ScriptAbort("Failed to add party " + partyname)
partyid = party.partyid
print "Added party ID: " + partyid

sleep(15) #let it work awhille
#---Delete party
conf.DeleteParty(partyid)
#---Close Conference
conf.Disconnect()
c.Disconnect()
