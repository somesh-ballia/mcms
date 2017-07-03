#!/mcms/python/bin/python

#############################################################################
# 
#
# Date: 05/03/06
# By  : Ron S.

#############################################################################


# For list of processes not for valgrind look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=McuMngr


from McmsConnection import *
from ISDNFunctions import *

###---------------------------------------------------------------------------------------
num_retries=20
num_of_parties=10

##--------------------------------------- Functions ---------------------------------
def CreateConf(c, confname):
    print "Creating Conference"
    c.CreateConf(confname, 'Scripts/CheckMultiTypesOfCalls/AddConf.xml')
    confid = c.WaitConfCreated(confname,num_retries) 
    return confid

def AddH323DialInParty(c, confid):
    print "Adding H323 Dial In party"
    partyname = "Party"+str(1)
    partyip =  "123.123.0." + str(1)
    c.AddParty(confid, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialInH323Party.xml")

def AddSipDialInParty(c, confid):
    print "Adding Sip Dial In party"
    partyname = "Party"+str(2)
    partyip =  "123.123.0." + str(2)
    partySipAdd = partyname + '@' + partyip
    c.AddSIPParty(confid, partyname, partyip, partySipAdd, "Scripts/CheckMultiTypesOfCalls/AddDialInSipParty.xml")

#def AddIsdnDialinPartyToRmx(c, confid):
#        print "Adding ISDN Dial In party"
#	partyname = "Party"+str(3) 
#	phoneout = "123"+str(3)
#	chnlNum="channel_12"
#	AddIsdnDialinParty(c,confid,partyname,phoneout,chnlNum)

def AddSimH323DialInParty(c, confname):
    print "Adding to SIM H323 Dial In party"
    partyname = "Party"+str(1)
    c.SimulationAddH323Party(partyname, confname)
    c.SimulationConnectH323Party(partyname)

def AddSimSipDialInParty(c, confname):
    print "Adding to SIM Sip Dial In party"
    partyname = "Party"+str(2)
    c.SimulationAddSipParty(partyname, confname)
    c.SimulationConnectSipParty(partyname)

#def AddSimIsdnDialInParty(c, confname):
#    print "Adding to SIM ISDN Dial In party"
#    partyname = "Party"+str(3)
#    phoneout = "123"+str(3)
#    chnlNum="channel_12"
#    SimulationAddIsdnParty(c, partyname,phoneout,chnlNum)
#    SimulationConnectIsdnParty(c, partyname)


def Add2DialOutH323Calls(c, confid, delay):
	print 'adding 2 dial out H323 video calls'     
	for x in range(2):
	    partyname = "Party" + str(x+3) 
	    partyip =  "1.2.3." + str(x+3)
	    c.AddParty(confid, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutH323Party.xml")
	    sleep(delay)

def Add2DialOutSipCalls(c, confid, delay):
	print 'adding 2 dial out Sip video calls'
	for x in range(2):
	    partyname = "Party" + str(x+5) 
	    partyip =  "1.2.3." + str(x+5)
	    partySipAdd = partyname + '@' + partyip
	    c.AddSIPParty(confid, partyname, partyip, partySipAdd, "Scripts/CheckMultiTypesOfCalls/AddDialOutSipParty.xml")
	    sleep(delay)

def Add2DialOutIsdnCalls(c, confid, delay):
	print 'adding 2 dial out ISDN video calls'
	for x in range(2):
	    partyname = "Party" + str(x+7) 
	    phoneout = "123"+str(x+1)
	    chnlNum="channel_12"
	    AddIsdnDialoutParty(c,confid,partyname,phoneout,chnlNum)
	    sleep(delay)

def Add1DialOutH323AudioCall(c, confid):
	print 'adding dial out H323 audio call'
	partyname = "Party" + str(9) 
	partyip =  "1.2.3." + str(9)
	c.AddParty(confid, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutH323AudioParty.xml")

def Add1DialOutSipAudioCall(c, confid):
	print 'adding dial out SIP audio call'
	partyname = "Party" + str(10) 
	partyip =  "1.2.3." + str(10)
	partySipAdd = partyname + '@' + partyip
	print "Adding Party " + partyname + ", with ip= " + partyip
	c.AddSIPParty(confid, partyname, partyip, partySipAdd, "Scripts/CheckMultiTypesOfCalls/AddDialOutSipAudioParty.xml")

def CheckAndSavePartiesAddedIds(c, party_id_list):
	c.LoadXmlFile('Scripts/CreateCPConfWith4DialInParticipants/TransConf2.xml')
	c.ModifyXml("GET","ID",confid)
	c.Send()
	ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
	if len(ongoing_party_list) < num_of_parties:
	    sys.exit("some parties are lost...")
	for index in range(num_of_parties):    
	    party_id_list.append(ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data)

def DeleteAllParties(c):
	for x in range(num_of_parties):
	    partyname = "Party"+str(x+1)
	    print "delete party: " + partyname    
	    c.DeleteParty(confid,party_id_list[x])
	    sleep(1)


##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

delayBetweenParticipants = 1
if(c.IsProcessUnderValgrind("ConfParty")):
    delayBetweenParticipants = 2


#Create Conf
confname = "Conf1"
confid = CreateConf(c, confname)

#Add Dial In H323 Party
AddH323DialInParty(c, confid)

#Add Dial In SIP Party
AddSipDialInParty(c, confid)

#Add Dial In ISDN Party
#AddIsdnDialinPartyToRmx(c, confid) - ISDN is Dial In Undefined calls only. Defined incoming calls is not relavant scenario

sleep(1)

# Add H323 party to EP Sim and connect it
AddSimH323DialInParty(c, confname)

# Add Sip party to EP Sim and connect it
AddSimSipDialInParty(c, confname)
  
# Add ISDN party to EP Sim and connect it
#AddSimIsdnDialInParty(c, confname)- ISDN is Dial In Undefined calls only. Defined incoming calls is not relavant scenario
  
print 'wait 30 seconds'
sleep(30)

# adding 2 dial out H323 video calls
Add2DialOutH323Calls(c, confid, delayBetweenParticipants)

sleep(2) 

# adding 2 dial out Sip video calls
Add2DialOutSipCalls(c, confid, delayBetweenParticipants)
    
sleep(2)    

# adding 2 dial out ISDN video calls
Add2DialOutIsdnCalls(c, confid, delayBetweenParticipants)
    
sleep(2)    

#adding 1 dial out H323 audio call.
Add1DialOutH323AudioCall(c, confid)

#adding 1 dial out SIP audio call.
Add1DialOutSipAudioCall(c, confid)

c.WaitAllPartiesWereAdded(confid,num_of_parties,num_retries)
c.WaitAllOngoingConnected(confid,num_retries)

# Check if all parties were added and save their IDs
party_id_list = []
CheckAndSavePartiesAddedIds(c, party_id_list)

# delete all parties
DeleteAllParties(c)  

# End Test
sleep(5)
c.DeleteConf(confid);
c.WaitAllConfEnd()
c.Disconnect()


##-----------------------------------------------------------------------------------------------

