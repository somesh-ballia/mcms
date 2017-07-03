#!/mcms/python/bin/python
# # NAME: SoftMcuConfWithOneAudioSipParty
# # OWNER: SOFT
# # GROUP: AudioParty
# # NIGHT: FALSE
# # VERSION: FALSE
# # VRESTART_AFTER: FALSE

from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()
c.DeleteAllConf()
c.SetAutoPortConfiguration(2)
from SMCUConference import *
from SoftMcuAudioSipParty import *
conf = SMCUConference()
#---Create Conference
conf.Create(c)

party = SMCUAudioParty()

try:
	ip=os.environ["EP1"]
	status =party.Add(conf, "EP1", ip, "sip")
except KeyError:
	print "Error: cannot get EP1 env variable"
	sys.exit(1)
#status =party.Add(conf, partyname, "10.253.66.31", "sip")
if status != "Statuss OK":
	ScriptAbort("Failed to add party " + partyname)
partyid = party.partyid
print "Added party ID: " + partyid

sleep(10) #let it work 10 sec
#---Delete party
conf.DeleteParty(partyid)
#---Close Conference
conf.Disconnect()
