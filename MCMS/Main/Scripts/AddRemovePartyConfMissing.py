#!/mcms/python/bin/python

#############################################################################
# Eror handling Script which Try to Add Remove party to/from Non Existing conf
#
# Date: 22/03/06
# By  : Yoella.

#############################################################################

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8
#-LONG_SCRIPT_TYPE
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

from McmsConnection import *
from ISDNFunctions import *

def AddRemoveH323PartyConfMissing(connection,partyFile,numRetries):
    for x in range(numRetries):
        partyname = "H323Party"+str(x+1) 
	partyid = ""+str(x+1)
	partyip =  "1.2.3." +str(2*x)
	confid = "5"
    	print "Try Adding Party " + partyname + ", with ip= " + partyip+ " To Missing Conf"
    	connection.AddParty(confid, partyname, partyip, partyFile,"Conference name or ID does not exist")
    	print "Try To Remove Party " + partyname + ", with ip= " + partyip+ " From Missing Conf"
    	connection.DeleteParty(confid, partyid,"Conference name or ID does not exist")

def AddRemoveSipPartyConfMissing(connection,partyFile,numRetries):
    for y in range(numRetries):
        partyname = "SipParty"+str(y+1) 
	partyid = "1"+str(y+1)
	partyip =  "1.2.4." +str(2*y)
    	confid = "6"
	print "Try Adding Sip Party " + partyname + ", with ip= " + partyip+ " To Missing Conf"
    	connection.AddParty(confid, partyname, partyip, partyFile,"Conference name or ID does not exist")
    	print "Try To Remove Sip Party " + partyname + ", with ip= " + partyip+ " From Missing Conf"
    	connection.DeleteParty(confid, partyid,"Conference name or ID does not exist")
        
def AddRemoveIsdnPartyConfMissing(connection,numRetries):
    for z in range(numRetries):
        partyname = "IsdnParty"+str(z+1) 
	phoneout = "123"+str(z+1)
	partyid = "2"+str(z+1)
	chnlNum="channel_12"
    	confid = "7"
	print "Try Adding ISDN Party " + partyname + ", with phoneout= " + phoneout+ " To Missing Conf"
	AddIsdnDialoutParty(connection,confid,partyname,phoneout,chnlNum, "Conference name or ID does not exist")
    	print "Try To Remove ISDN Party " + partyname + ", with phoneout= " + phoneout+ " From Missing Conf"
    	connection.DeleteParty(confid, partyid,"Conference name or ID does not exist")

##--------------------------------------- TEST ---------------------------------

c = McmsConnection()
c.Connect()

AddRemoveH323PartyConfMissing(c,
                  'Scripts/Create5ConfWith3Participants/AddVideoParty1.xml',
                  20) #num of retries

AddRemoveSipPartyConfMissing(c,
                  'Scripts/CreateConfWith3SipParticipants/AddSipParty.xml',
                  20) #num of retries

#add a new profile
#ProfId = AddIsdnProfile(c, "profile1", "768")
AddRemoveIsdnPartyConfMissing(c,20) #num of retries

c.Disconnect()
##------------------------------------------------------------------------------
