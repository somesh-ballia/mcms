#!/mcms/python/bin/python
#FUNCTIONS fo HD Scripts
#  
# Date: 22/10/06
# By  : Inga
#############################################################################
from McmsConnection import *

#------------------------------------------------------------------------------

def AddHdProfile(self, profileName, transfer_rate, fileName="Scripts/HD/XML/AddHdProfile.xml", hd_resolution="None"):
    """Create new Profile.
        
    profileName - name of profile to be created.
    fileName - XML file
    """
    print "Adding new Profile..."
    self.LoadXmlFile(fileName)
    self.ModifyXml("RESERVATION","NAME", profileName)
    self.ModifyXml("RESERVATION","TRANSFER_RATE", transfer_rate)
    if hd_resolution != "None" :
	self.ModifyXml("RESERVATION","HD_RESOLUTION", hd_resolution)

    self.Send()
    ProfId = self.GetTextUnder("RESERVATION","ID")
    print "Profile, named: " + profileName + " ,ID = " + ProfId + ", is added"
    return ProfId
    
#------------------------------------------------------------------------------

def AddDialOutHDParticipants(self, conf_id, num_of_parties):

	num_retries = 10
	
	for party_num in range(1,num_of_parties+1):
		partyname = AddPartyFromNum(self,conf_id, party_num, "Scripts/HD/XML/AddHdDialOutParty.xml")
		#self.AudioSpeaker(conf_id, partyname)
		#self.VideoSpeaker(conf_id, partyname)
		sleep(2)
		
	self.WaitUntilAllPartiesConnected(conf_id,num_of_parties,num_retries)
	
#------------------------------------------------------------------------------

def AddDialInHDParticipants(self, conf_id, conf_name, num_of_parties,start_index):

    ### Add parties to EP Sim and connect them
    num_retries = 10
    partiesIdToName=dict()
    for x in range(start_index,start_index+num_of_parties):
        partyname = "sim_dial_in_"+str(x+1)
        self.SimulationAddH323Party(partyname, conf_name)
        self.SimulationConnectH323Party(partyname)
        #Get the party id
        currPartyID = self.GetCurrPartyID(conf_id,x,num_retries)
        if (currPartyID < 0):
            self.Disconnect()                
            sys.exit("Error:Can not find partry id of party: "+partyname)
        print "found party id ="+str(currPartyID)
        partiesIdToName[x-start_index] = partyname
    
    self.WaitAllOngoingConnected(conf_id,num_retries*num_of_parties)
    
    return partiesIdToName


#------------------------------------------------------------------------------
def AddPartyFromNum(self,confid, create_num, partyFile,expected_status="Status OK"):
    """Add a new party.
    
    confid - destination conference.
    partyname - party name.
    partyIp - ip address for new party
    partyFile - xml file which will be used to define the party
    """
    
    partyname = "sim_dial_out_" + str(create_num)
    partyIp = "1.1.1." + str(create_num)
    partyAliasName = "sim_ep_" + str(create_num)
    print "Adding Party..." + partyname
    self.LoadXmlFile(partyFile)
    self.ModifyXml("PARTY","NAME",partyname)
    self.ModifyXml("PARTY","IP",partyIp)
    self.ModifyXml("ALIAS","NAME",partyAliasName)
    self.ModifyXml("ADD_PARTY","ID",confid)
    self.Send(expected_status)
    return partyname
#------------------------------------------------------------------------------   
def AddHDParty(self, confid, partyname, partyip):
    """Add a new party.
        
    confid - destination conference.
    partyname - party name.
    sip - added party is a SIP party (True) or H323 (False)
    """
    self.LoadXmlFile('Scripts/HD/XML/AddHdDialOutParty.xml')
    self.ModifyXml("PARTY","NAME",partyname)
    self.ModifyXml("PARTY","IP",partyip)
    self.ModifyXml("ADD_PARTY","ID",confid)
    self.Send()   
    
#------------------------------------------------------------------------------   
def AudioSpeaker(self, conf_id, party_name):
    self.LoadXmlFile('Scripts/SpeakerChange/SimAudioSpeaker.xml')
    self.ModifyXml("AUDIO_SPEAKER","PARTY_NAME", party_name)
    self.Send()
    print "New Audio Speaker = " + party_name
        
#------------------------------------------------------------------------------
def VideoSpeaker(self, conf_id, party_name):
    self.LoadXmlFile('Scripts/SpeakerChange/SimActiveSpeaker.xml')
    self.ModifyXml("ACTIVE_SPEAKER","PARTY_NAME", party_name)
    self.Send()
    print "New Video Speaker = " + party_name
#------------------------------------------------------------------------------
def GetPartiesList(self,confid):
    """Monitor party in conference, and return it's id.
    
    confid - target conference id
    partyname - name of the party to monitor
    """
    self.LoadXmlFile('Scripts/TransConf2.xml')
    self.ModifyXml("GET","ID",confid)
    self.Send()
    ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
#    print "the num of ongoing parties are: " + str(len(ongoing_party_list))
    return ongoing_party_list
        
#------------------------------------------------------------------------------
def isForceInConferenceLevel(self,confid):
    self.LoadXmlFile('Scripts/TransConf2.xml')
    self.ModifyXml("GET","ID",confid)
    self.Send()
    force_state_force=str("forced")
    isForceInConferenceLevel = 0
    force_state = (str)(self.xmlResponse.getElementsByTagName("FORCE_STATE")[0].firstChild.data)
    if(force_state==force_state_force):
       isForceInConferenceLevel = 1
       print "\n"+"isForceInConferenceLevel: Force in Conference Level " +"\n"
    return isForceInConferenceLevel
        
#------------------------------------------------------------------------------
def ChangeSpeakers(self,conf_id,dial_in_paties_dict,change_interval,times):
		
#		self.LoadXmlFile('Scripts/TransConf2.xml')
#		self.ModifyXml("GET","ID",confid)
#		self.Send()
#		ongoing_party_list = self.xmlResponse.getElementsByTagName("ONGOING_PARTY")
#		print "the num of ongoing parties are: " + str(len(ongoing_party_list))
		
		ongoing_party_list = GetPartiesList(self,conf_id)
		change_layout_counter = 0
		
		num_connected_parties = len(ongoing_party_list)
		if (num_connected_parties <= 2):
			print "ChangeSpeakers - not enough parties"
		else:
			party_index = 2
			for switch_number in range (times+1):
				party_name = dial_in_paties_dict[party_index]
				VideoSpeaker(self,conf_id,party_name)
				AudioSpeaker(self,conf_id,party_name)
				party_index += 1
				if (party_index >= num_connected_parties):
					party_index = 0
				switch_number += 1
				sleep(change_interval)
				
		GetPartyVideoSource(self,conf_id,party_name)
        		
        		
        		
        	
        	
        	  
#            for index in range(len(ongoing_party_list)):  
#               party_name = ongoing_party_list[index].getElementsByTagName("NAME")[0].firstChild.data
#                VideoSpeaker(self,conf_id,party_name)
#                AudioSpeaker(self,conf_id,party_name)
#                change_layout_counter++
#                if (change_layout_counter >= times):
#                	break;
#                sleep(change_interval)
                
                
##                    partyid = ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data   
##                    print "the ID of the party is: " + partyid
##                    return partyid
##
#------------------------------------------------------------------------------
def GetPartiesInfo(connection,confid):
	
	connection.LoadXmlFile('Scripts/TransConf2.xml')
	connection.ModifyXml("GET","ID",confid)
	connection.Send()
	
	ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")

	t = (0,"null")
	PartiesIdNameList = list()
	for party in ongoing_parties:
		party_name = party.getElementsByTagName("NAME")[0].firstChild.data
		party_id = party.getElementsByTagName("ID")[0].firstChild.data
		t = (party_id,party_name)
		PartiesIdNameList.append(t)
	
	return PartiesIdNameList
                


           
#	print connection.xmlResponse.toprettyxml()


    


def GetPartyVideoSource(connection,confid,party_name):
	
	connection.LoadXmlFile('Scripts/TransConf2.xml')
	connection.ModifyXml("GET","ID",confid)
	connection.Send()
	
	ongoing_parties = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
	for party in ongoing_parties:
		current_party_name = party.getElementsByTagName("NAME")[0].firstChild.data
		print current_party_name
#		if (current_party_name == party_name):
		layout_type = party.getElementsByTagName("LAYOUT")[0].firstChild.data
		print layout_type
		#cells = party.getElementsByTagName("FORCE")[0].getElementsByTagName("CELL")
		force = connection.getElementsByTagNameFirstLevelOnly(party,"FORCE")[0]
		cells = force.getElementsByTagName("CELL")
		cell = cells[0]
		PartySource = cell.getElementsByTagName("SOURCE_ID")[0].firstChild.data
		print  PartySource 
		print "\n"
           
#	print connection.xmlResponse.toprettyxml()
		

#------------------------------------------------------------------------------
def CheckConfLayout(connection,conf_id,speaker_id):
 
# 	print "CheckConfLayout"
 	result = 0
	ongoing_party_list = GetPartiesList(connection,conf_id)
	for party in ongoing_party_list:
		current_party_name = party.getElementsByTagName("NAME")[0].firstChild.data
		current_party_id = int(party.getElementsByTagName("ID")[0].firstChild.data)
		#cells = party.getElementsByTagName("FORCE")[0].getElementsByTagName("CELL")
		force = connection.getElementsByTagNameFirstLevelOnly(party,"FORCE")[0]
		cells = force.getElementsByTagName("CELL")
		PartySource = int(cells[0].getElementsByTagName("SOURCE_ID")[0].firstChild.data)
		if (current_party_id != speaker_id):
			if (PartySource != speaker_id):
				print "party " + current_party_name + " see source is " + str(PartySource) + " instead of " + str(speaker_id)
				result = 1
#			else:
#				print "party " + current_party_name + " see source is " + str(PartySource)
		
	if (result == 0):
		print "CheckConfLayout - PASSED"
	else:
		print connection.xmlResponse.toprettyxml()
 		connection.Disconnect()
 		sys.exit("CheckConfLayout - FAILED")
#------------------------------------------------------------------------------
def CheckPartyLayout(connection,conf_id,party_id,source_party_id):
 
# 	print "CheckPartyLayout"
 	result = 0
	ongoing_party_list = GetPartiesList(connection,conf_id)
	for party in ongoing_party_list:
		current_party_name = party.getElementsByTagName("NAME")[0].firstChild.data
		current_party_id = int(party.getElementsByTagName("ID")[0].firstChild.data)
		#cells = party.getElementsByTagName("FORCE")[0].getElementsByTagName("CELL")
		force = connection.getElementsByTagNameFirstLevelOnly(party,"FORCE")[0]
		cells = force.getElementsByTagName("CELL")
		PartySource = int(cells[0].getElementsByTagName("SOURCE_ID")[0].firstChild.data)
		if (current_party_id == party_id):
			if (PartySource != source_party_id):
				print "party " + current_party_name + " see source is " + str(PartySource) + " instead of " + str(source_party_id)
				result = 1
                                if(isForceInConferenceLevel(connection,conf_id)):
                                   print "\n Force In Conference Level: Force in Party level ignored"
                                   result = 0
                                else: 
                                   "\n No Force In Conference Level: Force in Party level wasn't done"
#			else: 
#				print "party " + current_party_name + " see source is " + str(PartySource)
		
	if (result == 0):
		print "CheckPartyLayout - PASSED"
	else:
		print connection.xmlResponse.toprettyxml()
 		connection.Disconnect()
 		sys.exit("CheckPartyLayout - FAILED")
#------------------------------------------------------------------------------
def CheckConfSourceLayout(connection,conf_id,speaker_id,speaker_video_source_id):
 
# 	print "CheckConfSourceLayout"
 	result = 0
	ongoing_party_list = GetPartiesList(connection,conf_id)
	conf_source_found = 0
	for party in ongoing_party_list:
		current_party_name = party.getElementsByTagName("NAME")[0].firstChild.data
		current_party_id = int(party.getElementsByTagName("ID")[0].firstChild.data)
		#cells = party.getElementsByTagName("FORCE")[0].getElementsByTagName("CELL")
		force = connection.getElementsByTagNameFirstLevelOnly(party,"FORCE")[0]
		cells = force.getElementsByTagName("CELL")
		PartySource = int(cells[0].getElementsByTagName("SOURCE_ID")[0].firstChild.data)
		if (current_party_id == speaker_id):
			conf_source_found = 1
			if (PartySource != speaker_video_source_id):
				print "the speaker " + current_party_name + " see source is " + str(PartySource) + " instead of " + str(speaker_id)
				result = 1
#			else:
#				print "the speaker " + current_party_name + " see source is " + str(PartySource)
	if(conf_source_found == 0):
		print "the speaker not found: " + str(speaker_id)
		
	if (result == 0):
		print "CheckConfSourceLayout - PASSED"
	else:
		print connection.xmlResponse.toprettyxml()
 		connection.Disconnect()
 		sys.exit("CheckConfSourceLayout - FAILED")  
	
#------------------------------------------------------------------------------
def CheckConfVswLayout(connection,conf_id,conf_video_source_id,conf_source_video_source_id):
	CheckConfLayout(connection,conf_id,conf_video_source_id)
	CheckConfSourceLayout(connection,conf_id,conf_video_source_id,conf_source_video_source_id)
#------------------------------------------------------------------------------
def SetConfLevelForce(connection, confid, confLayoutType, partyToForce, cellToForce):
    print "Conference ID: "+ confid + " Changing Conf Level Force: PartyId: " + str(partyToForce) + " Forced to Cell: " + str(cellToForce)
    connection.LoadXmlFile('Scripts/ConfVideoLayout/ChangConfLayout.xml')
    connection.ModifyXml("SET_VIDEO_LAYOUT","ID",confid)
    connection.ModifyXml("FORCE","LAYOUT",confLayoutType) 
    connection.loadedXml.getElementsByTagName("FORCE_STATE")[cellToForce].firstChild.data = "forced"
    connection.loadedXml.getElementsByTagName("FORCE_ID")[cellToForce].firstChild.data = str(partyToForce)     
    connection.Send()
    return
#------------------------------------------------------------------------------
def RemoveConfLevelForce(connection, confid, confLayoutType, cellToForce):
    print "Conference ID: "+ confid + " Removing Conf Level Force from Cell: " + str(cellToForce)
    connection.LoadXmlFile('Scripts/ConfVideoLayout/ChangConfLayout.xml')
    connection.ModifyXml("SET_VIDEO_LAYOUT","ID",confid)
    connection.ModifyXml("FORCE","LAYOUT",confLayoutType) 
    connection.loadedXml.getElementsByTagName("FORCE_STATE")[cellToForce].firstChild.data = "auto"
    connection.loadedXml.getElementsByTagName("FORCE_ID")[cellToForce].firstChild.data = "-1"     
    connection.Send()
    return 
#------------------------------------------------------------------------------
def SetPartyLevelForce(connection, confid, partyid, confLayoutType, partyToForce, cellToForce):
    print "Changing Party Level Force: PartyId: " + str(partyid) + " Force Party " + str(partyToForce) + "to Cell: " + str(cellToForce)
    connection.LoadXmlFile('Scripts/PersonalLayout/ChangePersonalLayout.xml')
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","ID",confid)
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","PARTY_ID",partyid)
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","LAYOUT_TYPE","conference")
    connection.ModifyXml("FORCE","LAYOUT",confLayoutType)  
    connection.loadedXml.getElementsByTagName("FORCE_STATE")[cellToForce].firstChild.data = "forced"
    connection.loadedXml.getElementsByTagName("FORCE_ID")[cellToForce].firstChild.data = str(partyToForce)     
    connection.Send()
    if(isForceInConferenceLevel(connection,confid) == 0):
          connection.WaitPartySeesPartyInCell(confid, partyid, partyToForce, cellToForce)
    else:
          print "\n Force in conf level, ignore party level force" + "\n"
    return
#------------------------------------------------------------------------------
def RemovePartyLevelForce(connection, confid, partyid, confLayoutType, cellToForce):
    print "Remove Party Level Force: PartyId: " + str(partyid) + " From Cell: " + str(cellToForce)
    connection.LoadXmlFile('Scripts/PersonalLayout/ChangePersonalLayout.xml')
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","ID",confid)
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","PARTY_ID",partyid)
    connection.ModifyXml("SET_PARTY_VIDEO_LAYOUT_EX","LAYOUT_TYPE","conference")
    connection.ModifyXml("FORCE","LAYOUT",confLayoutType)  
    connection.loadedXml.getElementsByTagName("FORCE_STATE")[cellToForce].firstChild.data = "auto"
    connection.loadedXml.getElementsByTagName("FORCE_ID")[cellToForce].firstChild.data = "-1"     
    connection.Send()
    return
#------------------------------------------------------------------------------

def StartLectureMode(connection,confid, partyName,is_timer,timer_interval):
    print "Conference ID: "+ confid + " New Lecturer: " + partyName
    connection.LoadXmlFile('Scripts/PresentationMode/UpdateLectureMode.xml')
    connection.ModifyXml("SET_LECTURE_MODE","ID",confid)
    connection.ModifyXml("SET_LECTURE_MODE","LECTURE_NAME",partyName)  
    connection.ModifyXml("SET_LECTURE_MODE","ON","true")  
    connection.ModifyXml("SET_LECTURE_MODE","TIMER",is_timer)  
    connection.ModifyXml("SET_LECTURE_MODE","INTERVAL",timer_interval)  
    connection.ModifyXml("SET_LECTURE_MODE","AUDIO_ACTIVATED","false")
    connection.ModifyXml("SET_LECTURE_MODE","LECTURE_MODE_TYPE","lecture_mode")
    connection.Send()
    return
#------------------------------------------------------------------------------


        
				
				
				
				
	

	
	
	
	
	
	
