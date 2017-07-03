#!/mcms/python/bin/python

###########################################
#  By: Guy D.
#  Date: 8.10.07
#  Testing mixed dial in IpV4/IpV6 conference
# Re-Write date = 25/8/13
# Re-Write name = Uri A.
###########################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-LONG_SCRIPT_TYPE

from McmsConnection import *

c = McmsConnection()
c.Connect()

num_retries=20
num_of_parties=2

print "Create Conf..."

confname = "MixedConf"
c.CreateConf(confname, 'Scripts/AddConf.xml')
confid = c.WaitConfCreated(confname,num_retries)

print "Create conf with id " + str(confid) 

#Add Dial In IpV4 H323 Party
#c.LoadXmlFile("Scripts/CheckMultiTypesOfCalls/AddDialInParty.xml")
#partyname = "Party"+str(1)
#partyip =  "123.123.0." + str(1)
#print "Adding IpV4 H323 Party..."
#c.ModifyXml("PARTY","NAME",partyname)
#c.ModifyXml("PARTY","IP",partyip)
#c.ModifyXml("ADD_PARTY","ID",confid)
#c.Send()

#Add Dial In IpV6 H323 Party
#c.LoadXmlFile("Scripts/AddH323DialInPartyIpV6.xml")
#partyname = "Party"+str(2)
#partyip =  "[2001:0db8:85a3:08d3:123:123:0:" + str(2) + "]"
#print "Adding IpV6 H323 Party..."
#c.ModifyXml("PARTY","NAME",partyname)
#c.ModifyXml("PARTY","IP_V6",partyip)
#c.ModifyXml("ADD_PARTY","ID",confid)
#c.Send()

# Add H323 party to EP Sim and connect him
partyname = "Party"+str(1)
c.SimulationAddH323Party(partyname, confname)
c.SimulationConnectH323Party(partyname)
sleep(2)

# Add H323 party to EP Sim and connect him
ipVer = 1
capSetName="FULL CAPSET"
partyname = "Party"+str(2)
c.SimulationAddH323Party(partyname, confname, capSetName, ipVer)
c.SimulationConnectH323Party(partyname)
sleep(2)


c.WaitAllOngoingConnected(confid, num_retries)


# delete H323 endpoints from Simulation
partyname = "Party"+str(1)
c.SimulationDisconnectH323Party(partyname)   
c.SimulationDeleteH323Party(partyname)

sleep(1)

partyname = "Party"+str(2)
c.SimulationDisconnectH323Party(partyname)   
c.SimulationDeleteH323Party(partyname)

sleep(5)

#print "Delete Conference..."
c.DeleteConf(confid)

c.WaitAllConfEnd()

c.Disconnect()



