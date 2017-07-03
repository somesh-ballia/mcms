#!/usr/bin/python

from McmsConnection import *
from string import *
from ConfPkgUtil import *
from SMCUConference import *

class SMCUAudioParty:
    """This is a video party class."""
    conference = None
    protocol = None
    partyid = -1

#------------------------------------------------------------------------------    
    def TryAddFromEP1env(self,conf, EP_EnvVar, partyname="Party1", protocol ="sip"):
    	status = "Statuss ERR"
    	try:
		ip=os.environ[EP_EnvVar]
		status =self.Add(conf, partyname, ip, protocol)

    	except KeyError:
		print "Error: cannot get " + EP_EnvVar + " env variable"

    	return status 
#------------------------------------------------------------------------------    
    def Add(self, conf, partyname = "Party1", partyip =  "1.0.0.0", protocol ="sip"):
	if conf != None:
		self.conference = conf
	else:
		print "SMCUAudioParty::The conference object must be provided"
		return "Statuss ERR"
	confid = conf.internalConfId
	self.protocol = protocol
	connection = conf.connection
	
	print " -> Add a sip Audio only user"
	partyFile = "Scripts/AddAudioOnlySipParty.xml"
	AddSIPPartyWithInfo(connection,confid, partyname, partyip, partyname+"@"+partyip, partyFile)

	print "SMCUAudioParty::Status of sending AddAudioOnlySipParty.xml" + connection.last_status
	self.partyid = connection.GetPartyId(confid,partyname)

	connection.WaitAllOngoingConnected(confid,20)
	connection.WaitAllOngoingNotInIVR(confid)
	return "Statuss OK"

  
