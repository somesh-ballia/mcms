#!/mcms/python/bin/python
# # NAME: SoftMcuConfWithOneAudioSipParty
# # OWNER: SOFT
# # GROUP: AudioParty
# # NIGHT: FALSE
# # VERSION: TRUE
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

########## party1 ###########
party1 = SMCUAudioParty()

try:
	ip1=os.environ["EP1"]
	status =party1.Add(conf, "EP1", ip1, "sip")
except KeyError:
	print "Error: cannot get EP1 env variable"
	sys.exit(1)

if status != "Statuss OK":
	ScriptAbort("Failed to add party EP1" + partyname)
partyid1 = party1.partyid
print "Added party ID: " + partyid1

########## party2 ###########
party2 = SMCUAudioParty()
partyid2=-1
try:
	ip2=os.environ["EP2"]
	status =party2.Add(conf, "EP2", ip2, "sip")
except KeyError:
	print "EP2 is not set"
	sys.exit(0)
if status != "Statuss OK":
	ScriptAbort("Failed to add party EP2" + partyname)
else:
	partyid2 = party2.partyid

print "Added party ID: " + partyid2

sleep(10) #let it work 10 sec
#---Delete parties
conf.DeleteParty(partyid1)
if partyid2 != -1:
	conf.DeleteParty(partyid2)
#---Close Conference
conf.Disconnect()
