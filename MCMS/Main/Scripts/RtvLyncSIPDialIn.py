#!/mcms/python/bin/python

#-LONG_SCRIPT_TYPE
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_8
#*PROCESSES_NOT_FOR_VALGRIND=Authentication Resource Faults CertMngr GideonSim EndpointsSim Logger QAAPI MplApi CSApi EncryptionKeyServer

#############################################################################
# Test Script For RTV  
# Date: 15/03/12
# By  : Shmulik Y
#############################################################################


import os
from McmsConnection import *

def AddProfileWithRate(self, profileName, transfer_rate, video_quality = "auto", fileName="Scripts/CreateNewProfile.xml"):
        print "Adding new Profile with rate " + str(transfer_rate) + " and video quality " + video_quality
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME", profileName)
        self.ModifyXml("RESERVATION","TRANSFER_RATE", str(transfer_rate))
        self.ModifyXml("RESERVATION","VIDEO_QUALITY", video_quality)        
        self.Send()
        ProfId = self.GetTextUnder("RESERVATION","ID")
        print "Profile, named: " + profileName + " ,ID = " + ProfId + ", is added"
        return ProfId   


#------------------------------------------------------------------------------
def TestLyncDailIn(connection,num_of_parties,num_retries,capset="NONE"):
	
	#create RTV profile
	profileName = "lync_profile_01"
	transferRate = 1536
	ProfId = AddProfileWithRate(connection, profileName, transferRate)

	#create RTV conf
	confName = "rtvConf"
	print "Adding Conf " + confName + " ..."
	connection.CreateConfFromProfile(confName, ProfId)
    	confid = connection.WaitConfCreated(confName,num_retries)

	### UserAgent
	#userAgent = "UCCAPI/4.0.7577.0 OC/4.0.7577.0 (Microsoft Lync 2010)"
	userAgent = "RTCC/4.0.0.0 AV-MCU"

	### Add parties to EP Sim and connect them
	for x in range(num_of_parties):
		partyname = "Party"+str(x+1)+" ,UserAgent:"+userAgent
		if capset=="NONE":
			if x==0:
				capset = "RTV_HD_CAPSET"
			elif x==1:
				capset = "RTV_VGA_CAPSET"
			elif x==2:
				capset = "RTV_CIF_CAPSET"
			elif x==3:
				capset = "RTV_QCIF_CAPSET"
			else:
				capset = "RTV_HD_CAPSET"
			
		print capset
		connection.SimulationAddSipParty(partyname, confName, capset, userAgent)
		connection.SimulationConnectSipParty(partyname)
		
	sleep(3)    
	connection.WaitAllOngoingConnected(confid,num_retries)

	# Check if all parties were added and save their IDs
	party_id_list = [0]*num_of_parties 
	connection.LoadXmlFile('Scripts/UndefinedSipDialIn/TransConf2.xml')
	connection.ModifyXml("GET","ID",confid)
	connection.Send()
	ongoing_party_list = c.xmlResponse.getElementsByTagName("ONGOING_PARTY")
	if len(ongoing_party_list) < num_of_parties:
		errMsg= "some parties are lost, find only " +str(len(ongoing_party_list)) + " parties in conf"
		sys.exit(errMsg )
	for index in range(num_of_parties):
		party_id_list[(num_of_parties - index) - 1 ]  = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data

	for index in range(num_of_parties):
		connection.SimulationDisconnectSipParty(partyname)
		connection.SimulationDeleteSipParty(partyname)

	connection.DeleteConf(confid)
	connection.WaitAllConfEnd()
	connection.DelProfile(ProfId)
	return

#------------------------------------------------------------------------------


## ---------------------- Test --------------------------



c = McmsConnection()
c.Connect()

TestLyncDailIn(c,2, 2,"RTV_VGA_CAPSET")


c.Disconnect()


