#!/mcms/python/bin/python

from McmsConnection import *
from SMCUConference import *

class SMCUVideoParty:
    """This is a video party class."""
    conference = None
    protocol = None
    partyid = -1

#------------------------------------------------------------------------------    
    def TryAddFromEP1env(self,conf, EP_EnvVar, partyname="Party1", protocol ="h323", videoprot = "h264", brate = "384"):
    	status = "Error"
	connected = 0
    	try:
		ip=os.environ[EP_EnvVar]
		status =self.Add(conf, partyname, ip, protocol, videoprot, brate)
		if status == "Status OK":
		   connected += 1 

    	except KeyError:
		print "Error: cannot get " + EP_EnvVar + " env variable"
		status = "Status OK"

    	return status, connected 

#------------------------------------------------------------------------------    
    def Add(self, conf, partyname = "Party1", partyip =  "1.0.0.0", protocol ="h323", videoprot = "h264", brate= "384"):
	if conf != None:
		self.conference = conf
	else:
		print "SMCUVideoParty::The conference object must be provided"
		return "Status ERR"
	confid = conf.internalConfId
	self.protocol = protocol
	connection = conf.connection
	delayBetweenParticipants = 0
	partyFile = 'Scripts/AddVideoParty1.xml'	
	#----connect party
	connection.LoadXmlFile(partyFile)
	print "SMCUVideoParty::Adding Party ("+partyname+", partyip = " + partyip +")"
	connection.ModifyXml("PARTY","NAME",partyname)
	connection.ModifyXml("PARTY","IP",partyip)
	connection.ModifyXml("ADD_PARTY","ID",confid)
	connection.ModifyXml("PARTY", "INTERFACE", protocol)
	#connection.ModifyXml("PARTY","VIDEO_PROTOCOL", videoprot)
	#connection.ModifyXml("PARTY","VIDEO_PROTOCOL","h264")
	connection.ModifyXml("PARTY","VIDEO_BIT_RATE",brate)
	connection.Send()
	print "SMCUVideoParty::Status of sending AddVideoParty1.xml" + connection.last_status
	sleep(1)
	self.partyid = connection.GetPartyId(confid,partyname)

	connection.WaitAllOngoingConnected(confid,30)
	connection.WaitAllOngoingNotInIVR(confid)
	return "Status OK"


