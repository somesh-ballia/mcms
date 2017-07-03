#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_9

from MuteUtil import *
from McmsConnection import *
num_retries=20
num_of_parties=1
          
#------------------------------------------------------------------------------

#Create CP conf with 3 parties
c = McmsConnection()
c.Connect()

confName = "Conf1"
c.CreateConf(confName, "Scripts/AddCpConf.xml")
confid = c.WaitConfCreated(confName,num_retries)

# Add Dial In Parties
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    partyip =  "123.123.0." + str(x+1)
    c.AddParty(confid, partyname, partyip, "Scripts/CreateConfWith4DialInSipParties/AddDialInSipParty.xml")

c.WaitAllPartiesWereAdded(confid,num_of_parties,num_retries)

# Check if all parties were added and save their IDs
party_id_list = []
c.LoadXmlFile('Scripts/CreateConfWith4DialInSipParties/TransConf2.xml')
c.ModifyXml("GET","ID",confid)
c.Send()
ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
if len(ongoing_party_list) < num_of_parties:
    sys.exit("some parties are lost...")
for index in range(num_of_parties):    
    party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)

# Add parties to EP Sim and connect them
for x in range(num_of_parties):
    partyname = "Party"+str(x+1)
    c.SimulationAddSipParty(partyname, confName)
    c.SimulationConnectSipParty(partyname)
  
c.WaitAllOngoingConnected(confid,num_retries)

mutedParty = 0;
#WaitAllButSelfSeesPartyInLayout(c, confid, mutedParty)
print
print "Mute Audio by Port in Conf ..."
SipMutePartyAudioPortTest(c, confid, mutedParty, 0)
sleep(10)

mutedParty = 0;
#WaitAllButSelfSeesPartyInLayout(c, confid, mutedParty)
print
print "Unmute Audio by Port in Conf ..."
SipMutePartyAudioPortTest(c, confid, mutedParty, 0xc020)
sleep(10)

print    
print "Start Deleting Conference..."
c.DeleteConf(confid)   
c.WaitAllConfEnd()

c.Disconnect()

