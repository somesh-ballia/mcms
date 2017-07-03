#!/mcms/python/bin/python

import sys
import httplib, urllib
import xml.dom.minidom
import re
from time import *

from xml.dom.minidom import parse, parseString, Document

from McmsConnection import *

class ResourceUtilities ( McmsConnection ):
#------------------------------------------------------------------------------
    def CreateDailyRepeatedRes(self, confName, ProfId, occur_num, minutes_delta=30, fileName="Scripts/AddRemoveReservation/AddRepeatedRes.xml", meetMePerConf = "false", phoneNumber=""):
        """Create new Repeated reservation.
        
        confName  - name of repeated Res to be created.
        ProfId - Id of profile used in reservation.
        fileName - XML file
        minutes_delta - how much minutes from now till the first occurrence (can be 0 - the first occurrence will start in a 
        """
        print "Adding a repeated reservation - " + confName + " Daily Occurrences - " + str(occur_num)
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME",confName)
        self.ModifyXml("RESERVATION","DISPLAY_NAME",confName)
        self.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",str(ProfId))
        self.ModifyXml("REPEATED_EX","OCCUR_NUM",str(occur_num))
        self.ModifyXml("MEET_ME_PER_CONF","ON",meetMePerConf)
        self.ModifyXml("MEET_ME_PER_CONF","PHONE1",phoneNumber)
        t = datetime.utcnow( ) 
        if minutes_delta!=0 :
       	    deltat = timedelta(0,0,0,0,minutes_delta,0,0)
       	    t = t + deltat      
        iso_time = t.strftime("%Y-%m-%dT%H:%M:%S")
        print "the start time is: " + iso_time
        self.ModifyXml("RESERVATION","START_TIME",iso_time)
		
		#when adding repeated reservations, it might take a long time
		#especially if there are a lot of reservations
        self.Send("Status OK", 60)
        
        repeated_id = self.GetTextUnder("RESERVATION","REPEATED_ID")
        print "Repeated reservation, named: " + confName + ", repeated_id = " + repeated_id + " is added"
        return repeated_id       
                
#------------------------------------------------------------------------------
    def DelRepeatedReservation(self, id, fileName="Scripts/AddRemoveReservation/DeleteRepeatedRes.xml"):
        """Delete repeated reservation.
        id - Id of reservation to be deleted.
        fileName - XML file
        """
        print "Remove repeated reservation, Id:" + str(id)
        self.LoadXmlFile(fileName)
        self.ModifyXml("CANCEL_REPEATED","REPEATED_ID",id)
        
        self.Send("In progress")
        
#------------------------------------------------------------------------------
    def CreateAndThenDeleteRepeated(self, profileId, num_of_occurences):
        
        repeated_id = self.CreateDailyRepeatedRes("repeated_test", profileId, num_of_occurences)
        if (num_of_occurences > 1000):
            print "Sleep for 20 sec"
            sleep(20)
        else:
            print "Sleep for 5 sec"
            sleep(5)
      
        self.Connect()
        res_number = self.GetNumOfReservationsInList()
        print "Number of reservations in list, (after adding the repeated reservations) is " + str(res_number)
        if res_number != num_of_occurences  :
            print "Error: Not all reservations were found in the list, found "+ str(res_number)+" Reservations"
            self.Disconnect()                
            sys.exit("Not all reservations were created")
   
        self.DelRepeatedReservation(repeated_id)

        if (num_of_occurences > 1000):
            print "Sleep for 20 sec"
            sleep(20)
        else:
            print "Sleep for 5 sec"
            sleep(5)
       
        res_number = self.GetNumOfReservationsInList()
        print "Number of reservations in list, (after removing the repeated reservations) is " + str(res_number)
        if res_number != 0 :
            print "Error: Not all reservations were deleted, found "+ str(res_number)+" Reservations"
            self.Disconnect()                
            sys.exit("Not all reservations were deleted")  

#------------------------------------------------------------------------------
    def GetNumOfReservationsInList(self):
       self.SendXmlFile('Scripts/AddRemoveReservation/TransResList.xml',"Status OK") 
       res_list = self.xmlResponse.getElementsByTagName("RES_SUMMARY")
       return len(res_list)
#------------------------------------------------------------------------------
    def GetNumberOfMRs(self): 
        self.SendXmlFile('Scripts/GetMRList.xml',"Status OK")
        mr_list = self.xmlResponse.getElementsByTagName("MEETING_ROOM_SUMMARY")
        return len(mr_list)     
#------------------------------------------------------------------------------
    def CheckNumOfReservationsInList(self,expected_num):
       num = self.GetNumOfReservationsInList()
       if(expected_num != num):
	   		self.Disconnect()                
	   		sys.exit("Unexpected number of reservations " + str(num) + " Expected: " + str(expected_num))
       print "There are now indeed " + str(expected_num) + " reservations in the list "		
#------------------------------------------------------------------------------
    def CheckNumOfMRsInList(self,expected_num):
       num = self.GetNumberOfMRs()
       if(expected_num != num):
	   		self.Disconnect()                
	   		sys.exit("Unexpected number of MRs " + str(num) + " Expected: " + str(expected_num))
       print "There are now indeed " + str(expected_num) + " MRs in the list "		       		
#------------------------------------------------------------------------------
    def GetNumOfConferencesInList(self):
       self.SendXmlFile('Scripts/TransConfList.xml',"Status OK") 
       conf_list = self.xmlResponse.getElementsByTagName("CONF_SUMMARY")
       return len(conf_list)
#------------------------------------------------------------------------------
    def GetNumConnectedParties(self,confid):
       strConfId = str(confid)
       self.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
       ongoing_conf_list = self.xmlResponse.getElementsByTagName("CONF_SUMMARY")
       for index in range(len(ongoing_conf_list)):  
           if(ongoing_conf_list[index].getElementsByTagName("ID")[0].firstChild.data == strConfId):
              if (ongoing_conf_list[index].getElementsByTagName("NUM_CONNECTED_PARTIES")[0].firstChild.data != ""):
                 return int(ongoing_conf_list[index].getElementsByTagName("NUM_CONNECTED_PARTIES")[0].firstChild.data)
              else :
                 print "Conference found but not number of ongoing parties for "+ strConfId +" Conference"
                 self.Disconnect()                
                 sys.exit("Conference found but not number of ongoing parties")  
       print "Conference NOT found for "+ strConfId +" Conference"
       self.Disconnect()                
       sys.exit("Conference NOT found") 
#------------------------------------------------------------------------------
    def GetResStatus(self,res_id):
        self.SendXmlFile('Scripts/AddRemoveReservation/TransResList.xml',"Status OK")
        res_list = self.xmlResponse.getElementsByTagName("RES_SUMMARY")
        for index in range(len(res_list)):  
          if(res_id == res_list[index].getElementsByTagName("ID")[0].firstChild.data):
              status = res_list[index].getElementsByTagName("RES_STATUS")[0].firstChild.data
              print "The status of the reservation " + res_list[index].getElementsByTagName("NAME")[0].firstChild.data + " is now: " + status
              return status
        self.Disconnect()                
        sys.exit("Reservation NOT found for "+ res_id)          
#------------------------------------------------------------------------------
    def CheckResStatus(self,res_id,expected_status):
        status = self.GetResStatus(res_id)
        if(status != expected_status):
            self.Disconnect()    
            sys.exit("Incorrect status was found for reservation " + res_id)
#------------------------------------------------------------------------------
    def UpdateReservation(self, name, id, numericid, profileId, start_time, min_audio = 0, min_video = 0, expected_result="Status OK", meetMePerConf = "false", phoneNumber=""):
        print "Updating reservation " + name + " numericid: " + numericid + " min_audio: " + str(min_audio) + " min_video: " + str(min_video) + " meetMePerConf: " + meetMePerConf + " phoneNumber: " + phoneNumber
        self.LoadXmlFile('Scripts/AddRemoveReservation/UpdateRes.xml')
        self.ModifyXml("RESERVATION","NAME",name)
        self.ModifyXml("RESERVATION","DISPLAY_NAME",name)
        self.ModifyXml("RESERVATION","ID",id)
        self.ModifyXml("RESERVATION","NUMERIC_ID",numericid)
        self.ModifyXml("RESERVATION","AD_HOC_PROFILE_ID",profileId) 
        iso_time = start_time.strftime("%Y-%m-%dT%H:%M:%S")
        self.ModifyXml("RESERVATION","START_TIME",iso_time)
        self.ModifyXml("MEET_ME_PER_CONF","MIN_NUM_OF_PARTIES",str(min_video))
        self.ModifyXml("MEET_ME_PER_CONF","MIN_NUM_OF_AUDIO_PARTIES",str(min_audio))
        self.ModifyXml("MEET_ME_PER_CONF","ON",meetMePerConf)
        self.ModifyXml("MEET_ME_PER_CONF","PHONE1",phoneNumber)

        self.Send(expected_result)
        if expected_result != "Status OK":
	    	print "UpdateReservation returned with expected status of " + expected_result    
#------------------------------------------------------------------------------
    def UpdateProfile(self, name, id, transfer_rate, video_quality, expected_result="Status OK"):
        print "Updating Profile with rate " + str(transfer_rate) + " and video quality " + video_quality
        self.LoadXmlFile('Scripts/UpdateProfile.xml')
        self.ModifyXml("RESERVATION","NAME",name)
        self.ModifyXml("RESERVATION","ID",id)
        self.ModifyXml("RESERVATION","TRANSFER_RATE", str(transfer_rate))
        self.ModifyXml("RESERVATION","VIDEO_QUALITY", video_quality)        
        self.Send(expected_result)
        if expected_result != "Status OK":
	    	print "UpdateProfile returned with expected status of " + expected_result                      	    	                  
#------------------------------------------------------------------------------
    def GetCurrentMode(self, expected_result="Status OK") : 
	    self.SendXmlFile("Scripts/ResourceAllocationModeAndConfiguration/GetAllocationMode.xml", expected_result)
	    if(expected_result=="Status OK"):
 	    	return self.GetTextUnder("ALLOCATION_MODE","ALLOCATION_MODE_CURRENT")
	    else:
	    	return ""	
#------------------------------------------------------------------------
    def GetFutureMode(self, expected_result="Status OK") : 
	    self.SendXmlFile("Scripts/ResourceAllocationModeAndConfiguration/GetAllocationMode.xml", expected_result)
	    if(expected_result=="Status OK"):
	    	return self.GetTextUnder("ALLOCATION_MODE","ALLOCATION_MODE_FUTURE")
	    else:
	    	return ""	
#------------------------------------------------------------------------
    def SetMode(self,mode, expected_result="Status OK") :
	    #in case we expect status OK, first delete all conferences
	    if(expected_result=="Status OK"):
	    	self.DeleteAllConf()
	    	self.WaitAllConfEnd(100) 
	    print "Setting mode to " + mode
	    self.LoadXmlFile("Scripts/ResourceAllocationModeAndConfiguration/SetAllocationMode.xml")
	    self.ModifyXml("SET_ALLOCATION_MODE","SELECTED_ALLOCATION_MODE",mode)
	    self.Send(expected_result)
	    print "Indeed received expected status of " + expected_result
	    #sleeping to allow reconfiguration and everything to finish
	    sleep(5)
#------------------------------------------------------------------------
    def CheckModes(self,expected_current, expected_future, expected_result="Status OK") :
		currentmode = self.GetCurrentMode(expected_result)
		futuremode = self.GetFutureMode(expected_result)
		if(expected_result!="Status OK"):
		    print "GET_ALLOCATION_MODE indeed returned with status " + expected_result
		    return
		if futuremode != expected_future:
			print "Unexpected future mode " + futuremode + " Expected: " + expected_future 
	   		self.Disconnect()                
	   		sys.exit("Unexpected future mode")
		if currentmode != expected_current:
			print "Unexpected current mode " + currentmode + " Expected: " + expected_current 
	   		self.Disconnect()                
	   		sys.exit("Unexpected current mode")
	   	print "Modes are as expected! Current mode: " + expected_current + " Future mode: " + expected_future
#------------------------------------------------------------------------
    def ActiveAlarmExists(self,active_alarm_string):
		self.SendXmlFile("Scripts/GetActiveAlarms.xml")
		fault_elm_list = self.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
		for index in range(len(fault_elm_list)):
			fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
			#print fault_desc
			if fault_desc == active_alarm_string:
				return 1
		return 0
#------------------------------------------------------------------------
    def IsThereAnActiveAlarmOfAllocationModeChange(self):
		return self.ActiveAlarmExists("ALLOCATION_MODE_CHANGED")
#------------------------------------------------------------------------
    def IsThereAnActiveAlarmOfPortConfigurationChange(self):
		return self.ActiveAlarmExists("PORT_CONFIGURATION_CHANGED")
#------------------------------------------------------------------------
    def IsThereAnActiveAlarmOfInsufficientResources(self):
		return self.ActiveAlarmExists("INSUFFICIENT_RESOURCES")		
#------------------------------------------------------------------------
    def ResetPortConfiguration(self, expected_num_audio, expected_num_video, expected_result="Status OK") : 
	    self.SendXmlFile("Scripts/ResourceAllocationModeAndConfiguration/ResetPortConfiguration.xml", expected_result)
	    selected_configuration_index = self.GetTextUnder("AUDIO_VIDEO_CONFIG_LIST","SELECTED_ID")
	    print "Selected configuration index is: " + selected_configuration_index
	    configuration_list = self.xmlResponse.getElementsByTagName("AUDIO_VIDEO_CONFIG")
	    #print "len(configuration_list): " + str(len(configuration_list))
	    for index in range(len(configuration_list)):
	    	id_desc = configuration_list[index].getElementsByTagName("ID")[0].firstChild.data
	    	if id_desc == selected_configuration_index:
	    		audio = int(configuration_list[index].getElementsByTagName("NUM_AUDIO_PORTS")[0].firstChild.data)
	    		video = int(configuration_list[index].getElementsByTagName("NUM_VIDEO_PORTS")[0].firstChild.data)
	    		if audio != expected_num_audio:
	    			print "Unexpected audio " + str(audio) + " expected " + str(expected_num_audio)
	    			self.Disconnect()
	    			sys.exit("Unexpected audio configuration")
	    		if video != expected_num_video:
	    			print "Unexpected video " + str(video) + " expected " + str(expected_num_video)
	    			self.Disconnect()
	    			sys.exit("Unexpected video configuration")
	    		print "Selected configuration as expected: audio " + str(audio) + " video " + str(video)
	    		return
	    self.Disconnect()
	    sys.exit("ResetPortConfiguration configuration not found")	
#------------------------------------------------------------------------
    def SetAutoPortConfiguration(self,index_of_configuration, expected_result="Status OK", sleep_at_end = 1):
	    if(expected_result=="Status OK"):
	    	self.DeleteAllConf()
	    	self.WaitAllConfEnd(100) 
	    self.LoadXmlFile("Scripts/ResourceAllocationModeAndConfiguration/SetAutoPortConfiguration.xml")
	    self.ModifyXml("SET_PORT_CONFIGURATION","SELECTED_ID",index_of_configuration)
	    self.Send(expected_result)
	    if expected_result != "Status OK":
	    	print "SetAutoPortConfiguration returned with expected status of " + expected_result
	    	return
	    if(sleep_at_end==1):	
	    	sleep(5)
#------------------------------------------------------------------------
    def CheckAudioVideoAutoConfiguration(self,expected_audio, expected_video, expected_result="Status OK"):
		self.SendXmlFile("Scripts/ResourceAllocationModeAndConfiguration/GetFixedPortConfiguration.xml", expected_result)
		if expected_result != "Status OK":
			print "CheckAudioVideoAutoConfiguration returned with expected status of " + expected_result
		selected_configuration_index = self.GetTextUnder("AUDIO_VIDEO_CONFIG_LIST","SELECTED_ID")
		print "Selected configuration index is: " + selected_configuration_index
		configuration_list = self.xmlResponse.getElementsByTagName("AUDIO_VIDEO_CONFIG")
		#print "len(configuration_list): " + str(len(configuration_list))
		audio = 0
		video = 0
		found_configuration = 0
		for index in range(len(configuration_list)):
			id_desc = configuration_list[index].getElementsByTagName("ID")[0].firstChild.data
			if id_desc == selected_configuration_index:
				audio = int(configuration_list[index].getElementsByTagName("NUM_AUDIO_PORTS")[0].firstChild.data)
				video = int(configuration_list[index].getElementsByTagName("NUM_VIDEO_PORTS")[0].firstChild.data)
				print "Selected configuration is: audio " + str(audio) + " video " + str(video) 
				found_configuration = 1
				break
		if found_configuration == 0:
		   	self.Disconnect()                
		   	sys.exit("The configuration was not found")
		if audio != expected_audio:   		
			print "Unexpected audio configuration " + str(audio) + " Expected: " + str(expected_audio) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected audio configuration")	
		if video != expected_video:   		
			print "Unexpected video configuration " + str(video) + " Expected: " + str(expected_video) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected video configuration")	
	   	print "Audio/video configuration is as expected! Video: " + str(expected_video) + " Audio: " + str(expected_audio)
#------------------------------------------------------------------------
    def CheckEnhancedConfiguration(self,expected_audio, expected_CIF, expected_SD, expected_HD720, expected_HD1080, expected_result="Status OK"):
		self.SendXmlFile("Scripts/ResourceAllocationModeAndConfiguration/GetEnhancedPortConfiguration.xml", expected_result)	
		if expected_result != "Status OK":
			print "GetEnhancedConfiguration returned with expected status of " + expected_result		
		audio = int(self.GetTextUnder("AUDIO_CONFIG","CONFIG_CURRENT"))
		CIF = int(self.GetTextUnder("CIF_CONFIG","CONFIG_CURRENT"))
		SD = int(self.GetTextUnder("SD_CONFIG","CONFIG_CURRENT"))
		HD720 = int(self.GetTextUnder("HD720_CONFIG","CONFIG_CURRENT"))
		HD1080 = int(self.GetTextUnder("HD1080_CONFIG","CONFIG_CURRENT"))
		if audio != expected_audio:   		
			print "Unexpected audio configuration " + str(audio) + " Expected: " + str(expected_audio) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected audio configuration")	
		if CIF != expected_CIF:   		
			print "Unexpected CIF configuration " + str(CIF) + " Expected: " + str(expected_CIF) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected CIF configuration")	
		if SD != expected_SD:   		
			print "Unexpected SD configuration " + str(SD) + " Expected: " + str(expected_SD) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected SD configuration")	
		if HD720 != expected_HD720:   		
			print "Unexpected HD720 configuration " + str(HD720) + " Expected: " + str(expected_HD720) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected HD720 configuration")	
		if HD1080 != expected_HD1080:   		
			print "Unexpected HD1080 configuration " + str(HD1080) + " Expected: " + str(expected_HD1080) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected HD1080 configuration")	
	   	print "Enhanced configuration is as expected! Audio: " + str(expected_audio) + " CIF: " + str(expected_CIF) + " SD: " + str(expected_SD) + " HD720: " + str(expected_HD720) + " HD1080: " + str(expected_HD1080) 		
#------------------------------------------------------------------------
    def SetEnhancedConfiguration(self,audio, CIF, SD, HD720, HD1080, expected_result="Status OK"):
		print "***Setting enhanced configuration of Audio: " + str(audio) + " CIF: " + str(CIF) + " SD: " + str(SD) + " HD720: " + str(HD720) + " HD1080: " + str(HD1080)  
		self.LoadXmlFile("Scripts/ResourceAllocationModeAndConfiguration/SetFixedPortConfiguration.xml")		
		self.ModifyXml("SET_ENHANCED_PORT_CONFIGURATION","AUDIO_NUM_PORTS_CONFIG",audio)
		self.ModifyXml("SET_ENHANCED_PORT_CONFIGURATION","CIF_NUM_PORTS_CONFIG",CIF)
		self.ModifyXml("SET_ENHANCED_PORT_CONFIGURATION","SD_NUM_PORTS_CONFIG",SD)
		self.ModifyXml("SET_ENHANCED_PORT_CONFIGURATION","HD720_NUM_PORTS_CONFIG",HD720)
		self.ModifyXml("SET_ENHANCED_PORT_CONFIGURATION","HD1080_NUM_PORTS_CONFIG",HD1080)
		self.Send(expected_result)
		if expected_result != "Status OK":
			print "SetEnhancedConfiguration returned with expected status of " + expected_result 
#------------------------------------------------------------------------
    def GetCheckEnhancedConfiguration(self,type,audio, CIF, SD, HD720, HD1080,expected_audio, expected_CIF, expected_SD, expected_HD720, expected_HD1080, expected_result="Status OK"):
    	#type: CONFIG_SYSTEM_MAXIMUM, CONFIG_CURRENT, CONFIG_OPTIONAL_MAXIMUM
		print "Get check configuration for " + type + ". Audio: " + str(audio) + " CIF: " + str(CIF) + " SD: " + str(SD) + " HD720: " + str(HD720) + " HD1080: " + str(HD1080)   			
		self.LoadXmlFile("Scripts/ResourceAllocationModeAndConfiguration/GetCheckEnhancedConfiguration.xml")		
		self.ModifyXml("GET_CHECK_ENHANCED_PORT_CONFIGURATION","AUDIO_NUM_PORTS_CONFIG",audio)
		self.ModifyXml("GET_CHECK_ENHANCED_PORT_CONFIGURATION","CIF_NUM_PORTS_CONFIG",CIF)
		self.ModifyXml("GET_CHECK_ENHANCED_PORT_CONFIGURATION","SD_NUM_PORTS_CONFIG",SD)
		self.ModifyXml("GET_CHECK_ENHANCED_PORT_CONFIGURATION","HD720_NUM_PORTS_CONFIG",HD720)
		self.ModifyXml("GET_CHECK_ENHANCED_PORT_CONFIGURATION","HD1080_NUM_PORTS_CONFIG",HD1080)
		self.Send(expected_result)
		if expected_result != "Status OK":
			print "SetEnhancedConfiguration returned with expected status of " + expected_result
			return
		audio_received = int(self.GetTextUnder("AUDIO_CONFIG",type))
		CIF_received = int(self.GetTextUnder("CIF_CONFIG",type))
		SD_received = int(self.GetTextUnder("SD_CONFIG",type))
		HD720_received = int(self.GetTextUnder("HD720_CONFIG",type))
		HD1080_received = int(self.GetTextUnder("HD1080_CONFIG",type))
		if audio_received != expected_audio:   		
			print "Unexpected audio configuration " + str(audio_received) + " Expected: " + str(expected_audio) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected audio configuration")	
		if CIF_received != expected_CIF:   		
			print "Unexpected CIF configuration " + str(CIF_received) + " Expected: " + str(expected_CIF) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected CIF configuration")	
		if SD_received != expected_SD:   		
			print "Unexpected SD configuration " + str(SD_received) + " Expected: " + str(expected_SD) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected SD configuration")	
		if HD720_received != expected_HD720:   		
			print "Unexpected HD720 configuration " + str(HD720_received) + " Expected: " + str(expected_HD720) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected HD720 configuration")	
		if HD1080_received != expected_HD1080:   		
			print "Unexpected HD1080 configuration " + str(HD1080_received) + " Expected: " + str(expected_HD1080) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected HD1080 configuration")	
	   	print "Get check configuration is as expected! Audio: " + str(expected_audio) + " CIF: " + str(expected_CIF) + " SD: " + str(expected_SD) + " HD720: " + str(expected_HD720) + " HD1080: " + str(expected_HD1080) 		
				   		
#------------------------------------------------------------------------
    def GetIdOfAVideoUnit(self,boardId):
	 	visualBoardId = self.NormalBoardIdToVisualBoardId(boardId)   
		self.LoadXmlFile('Scripts/ResourceDetailParty/CardDetailReq.xml')
		self.ModifyXml("GET","BOARD_NUMBER",visualBoardId)
		self.Send()
		unit_summary = self.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
		unit_len =len(unit_summary)
	
		for i in range(unit_len):
			if unit_summary[i].getElementsByTagName("UNIT_TYPE")[0].firstChild.data == "video":
				return unit_summary[i].getElementsByTagName("UNIT_NUMBER")[0].firstChild.data
		
		self.Disconnect()                
		sys.exit("No video unit found on board " + str(boardId))
#------------------------------------------------------------------------
    def GetUnitType(self,boardId,unitId):
		visualBoardId = self.NormalBoardIdToVisualBoardId(boardId)    
		self.LoadXmlFile('Scripts/ResourceDetailParty/CardDetailReq.xml')
		self.ModifyXml("GET","BOARD_NUMBER",visualBoardId)
		self.Send()
		unit_summary = self.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
		unit_len =len(unit_summary)
	
		for i in range(unit_len):
			if unit_summary[i].getElementsByTagName("UNIT_NUMBER")[0].firstChild.data == str(unitId):
				return unit_summary[i].getElementsByTagName("UNIT_TYPE")[0].firstChild.data
		
		self.Disconnect()                
		sys.exit("Unit " + str(unitId) + " not found on board " + str(boardId) + " that has " + str(unit_len) + " units")
#------------------------------------------------------------------------
    def GetStatusOfUnit(self,boardId,unitId,subBoardId = 1):
		visualBoardId = self.NormalBoardIdToVisualBoardId(boardId,subBoardId)    
		self.LoadXmlFile('Scripts/ResourceDetailParty/CardDetailReq.xml')
		self.ModifyXml("GET","BOARD_NUMBER",visualBoardId)
		self.ModifyXml("GET","SUB_BOARD_ID",subBoardId)
		self.Send()
		unit_summary = self.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
		unit_len =len(unit_summary)
	
		for i in range(unit_len):
			if unit_summary[i].getElementsByTagName("UNIT_NUMBER")[0].firstChild.data == str(unitId):
				return unit_summary[i].getElementsByTagName("DESCRIPTION")[0].firstChild.data
		
		self.Disconnect()                
		sys.exit("Unit " + str(unitId) + " not found on board " + str(boardId) + " Subboard " + str(subBoardId) + " that has " + str(unit_len) + " units")
#------------------------------------------------------------------------	
    def WaitStatusOfUnit(self,boardId,unitId,status,subBoardId = 1):
        print "Waiting for the unit " + str(unitId) + " on board " + str(boardId) + " Subboard " + str(subBoardId) + " to have status of " + status
        num_retries = 20
        for retry in range(num_retries+1):
	        current_status = self.GetStatusOfUnit(boardId,unitId,subBoardId)
	        if(status in current_status):
	            print " Done!"
	            return
	        if (retry == num_retries):
	            print self.xmlResponse.toprettyxml()
	            self.Disconnect()                
	            sys.exit("Status of unit is still not as expected. It is : " + current_status)
	        sys.stdout.write(".")
	        sys.stdout.flush()
	        sleep(1)
#------------------------------------------------------------------------	
    def WaitUnitDisabled(self,boardId,unitId,subBoardId = 1):
        self.WaitStatusOfUnit(boardId,unitId,"Disabled",subBoardId)
#------------------------------------------------------------------------	
    def WaitUnitEnabled(self,boardId,unitId,subBoardId = 1):
        self.WaitStatusOfUnit(boardId,unitId,"Available",subBoardId)	
#------------------------------------------------------------------------
    def CheckStatusOfUnit(self,boardId,unitId, expected_status,subBoardId = 1) :
        status = self.GetStatusOfUnit(boardId,unitId,subBoardId)
        if status != expected_status:   		
        	print "Board " +  str(boardId) + " Subboard " + str(subBoardId) + " unit " + str(unitId) + ": Unexpected status " + str(status) + " Expected: " + str(expected_status) 
           	self.Disconnect()                
           	sys.exit("Unexpected status")	
        print "Board " +  str(boardId) + " Subboard " + str(subBoardId) + " unit " + str(unitId) + ": Indeed status is " + str(status)
#------------------------------------------------------------------------
    def CheckUnitAvailable(self,boardId,unitId,subBoardId = 1) :
        self.CheckStatusOfUnit(boardId,unitId,"Available;",subBoardId)
#------------------------------------------------------------------------
    def CheckUnitActive(self,boardId,unitId,subBoardId = 1) :
        self.CheckStatusOfUnit(boardId,unitId,"Active;",subBoardId)        				 		
#------------------------------------------------------------------------
    def GetUtulisationOfUnit(self,boardId,unitId):
		visualBoardId = self.NormalBoardIdToVisualBoardId(boardId)
		self.LoadXmlFile('Scripts/ResourceDetailParty/CardDetailReq.xml')
		self.ModifyXml("GET","BOARD_NUMBER",visualBoardId)
		self.Send()
		unit_summary = self.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
		unit_len =len(unit_summary)
	
		for i in range(unit_len):
			if unit_summary[i].getElementsByTagName("UNIT_NUMBER")[0].firstChild.data == str(unitId):
				return int(unit_summary[i].getElementsByTagName("UTILIZATION")[0].firstChild.data)
		
		self.Disconnect()                
		sys.exit("Unit " + str(unitId) + " not found on board " + str(boardId) + " that has " + str(unit_len) + " units")		
#------------------------------------------------------------------------
    def CheckUtilisationOfUnit(self,boardId,unitId, expected_utilization):
        utilization = self.GetUtulisationOfUnit(boardId,unitId)
        if utilization != expected_utilization:   		
        	print "Board " +  str(boardId) + " unit " + str(unitId) + ": Unexpected utilization " + str(utilization) + " Expected: " + str(expected_utilization) 
           	self.Disconnect()                
           	sys.exit("Unexpected utilization")	
        print "Board " +  str(boardId) + " unit " + str(unitId) + ": Indeed utilization is " + str(utilization)
#------------------------------------------------------------------------
    def CheckAudioVideoUnits(self,expected_audio, expected_video,boardId):
		visualBoardId = self.NormalBoardIdToVisualBoardId(boardId)    
		self.LoadXmlFile('Scripts/ResourceDetailParty/CardDetailReq.xml')
		self.ModifyXml("GET","BOARD_NUMBER",visualBoardId)
		self.Send()
		unit_summary = self.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
		unit_len =len(unit_summary)
	
		audio_units = 0
		video_units = 0
		for i in range(unit_len):
			if unit_summary[i].getElementsByTagName("UNIT_TYPE")[0].firstChild.data == "video":
				video_units = video_units + 1
			if unit_summary[i].getElementsByTagName("UNIT_TYPE")[0].firstChild.data == "smart":
				audio_units = audio_units + 1
		if audio_units != expected_audio:   		
			print "Board " +  str(boardId) + ": Unexpected audio configuration " + str(audio_units) + " Expected: " + str(expected_audio) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected number of audio units")	
		if video_units != expected_video:   		
			print "Board " +  str(boardId) + ": Unexpected video configuration " + str(video_units) + " Expected: " + str(expected_video) 
		   	self.Disconnect()                
		   	sys.exit("Unexpected number of video units")	
	   	print "Board " +  str(boardId) + ": Audio/video units are as expected! Video units: " + str(expected_video) + " Audio units: " + str(expected_audio)
#------------------------------------------------------------------------
    def CountTotaloccupied(self,boardId,unit_type):
    	#unit type: either "video" or "smart"
    	visualBoardId = self.NormalBoardIdToVisualBoardId(boardId)
    	self.LoadXmlFile('Scripts/ResourceDetailParty/CardDetailReq.xml')
    	self.ModifyXml("GET","BOARD_NUMBER",visualBoardId)
    	self.Send()
    	unit_summary = self.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
    	unit_len =len(unit_summary)
    	
    	sum = 0
    	for i in range(unit_len):
    	        if unit_summary[i].getElementsByTagName("UNIT_TYPE")[0].firstChild.data == unit_type:
    	           util = int(unit_summary[i].getElementsByTagName("UTILIZATION")[0].firstChild.data)
    	           if (util < 1001) :
    	               sum = sum + util
    	return sum               
#------------------------------------------------------------------------
    def CreateConfWithAudioOnlyParties(self, confname, num_parties):	
		self.CreateConf(confname)
		confid = self.WaitConfCreated(confname)   
		for x in range(num_parties):
			partyname = "Audio"+str(x+1)
			partyip =  "1.2.3." + str(x+1)
			self.AddParty(confid, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutH323AudioParty.xml")
			#every 20 participants, wait for everyone to connect, or else there's a problem in ConfParty in night tests
			if((x+1) % 20 == 0):
			    #os.system("ls -l /proc/`pgrep ConfParty`/fd")
			    self.WaitAllPartiesWereAdded(confid,x+1,30)
			    self.WaitAllOngoingConnected(confid)
		self.WaitAllPartiesWereAdded(confid,num_parties,10)
		self.WaitAllOngoingConnected(confid)
		print "Indeed succeeded to connect all audio parties to conference " + confname 
		return confid
#------------------------------------------------------------------------
    def CreateConfFromProfileAndDialInVideoParties(self, confname, profId, partyNamePrefix, num_of_parties, capName="FULL CAPSET"):    
        self.CreateConfFromProfile(confname,profId)
        confid = self.WaitConfCreated(confname)  
        for x in range(num_of_parties):
            partyname = partyNamePrefix + str(x+1)
            self.SimulationAddSipParty(partyname, confname, capName)
            self.SimulationConnectSipParty(partyname)
        self.WaitAllOngoingConnected(confid)
        return confid    
#------------------------------------------------------------------------
    def CreateConfFromProfileWithVideoParties(self, confname, profId, num_parties):	
        self.CreateConfFromProfile(confname,profId)
        confid = self.WaitConfCreated(confname)  
        self.AddVideoParties(confid,num_parties)
        return confid	
#------------------------------------------------------------------------
    def AddVideoParties(self, confid, num_of_parties):	
        for x in range(num_of_parties):
            partyname = "VideoParty"+str(x+1)
            partyip =  "1.2.3." + str(x+1)
            self.AddParty(confid, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutH323Party.xml")
            #every 20 participants, wait for everyone to be added, or else there's a problem in ConfParty in night tests
            if((x+1) % 20 == 0):
                 self.WaitAllPartiesWereAdded(confid,x+1,10)
        self.WaitAllPartiesWereAdded(confid,num_of_parties,10)
#--------------------------------------------------------------------------------
# testedResourcePortType for mixed or auto mode = "audio" , "video", 
# testedResourcePortType for fixed mode = "audio" , "CIF", "SD", "HD720", "HD1080"
# expected result - integer
# key = "TOTAL" , "OCCUPIED","FREE", "RESERVED"
    def TestResourcesReportPortsType(self,testedResourcePortType,expectedResult,key):

	self.SendXmlFile("Scripts/ResourceFullCapacity/ResourceMonitorCarmel.xml")
	rsrc_report_rmx_list = self.xmlResponse.getElementsByTagName("RSRC_REPORT_RMX")
	for i in range(len(rsrc_report_rmx_list)):
	    resourceType = self.GetTextUnder("RSRC_REPORT_RMX","RSRC_REPORT_ITEM",i)
	    if resourceType!="":
	        if resourceType==testedResourcePortType:
		    result = self.GetTextUnder("RSRC_REPORT_RMX",key,i)
		    if (expectedResult!=int(result)):
			print "Failed test for Resource Port TYPE = " + resourceType + ". Result = " + str(result) + ", expected result = " + str(expectedResult)
                        print self.xmlResponse.toprettyxml(encoding="utf-8")
                        sys.exit("Unexpected resource report")
                        self.Disconnect()
	print "Verified that " + testedResourcePortType + " in resource report is " + str(expectedResult) + " for " + key                   

#--------------------------------------------------------------------------------
# ResourcePortType for mixed or auto mode = "audio" , "video", 
# ResourcePortType for fixed mode = "audio" , "CIF", "SD", "HD720", "HD1080"
# key = "TOTAL" , "OCCUPIED","FREE", "RESERVED"
    def GetNumResources(self,ResourcePortType,key):

	self.SendXmlFile("Scripts/ResourceFullCapacity/ResourceMonitorCarmel.xml")
	rsrc_report_rmx_list = self.xmlResponse.getElementsByTagName("RSRC_REPORT_RMX")
	for i in range(len(rsrc_report_rmx_list)):
	    resourceType = self.GetTextUnder("RSRC_REPORT_RMX","RSRC_REPORT_ITEM",i)
	    if resourceType==ResourcePortType:
		    result = self.GetTextUnder("RSRC_REPORT_RMX",key,i)
		    return int(result)
							
#------------------------------------------------------------------------------
    def TestFreeCarmelVideoParties(self,num_of_resource):

        self.SendXmlFile("Scripts/ResourceFullCapacity/ResourceMonitorCarmel.xml")
	party_type = "video"
    	my_string = ""
    	rsrc_report_rmx_list = self.xmlResponse.getElementsByTagName("RSRC_REPORT_RMX")
	#print rsrc_report_rmx_list
	for index in range(len(rsrc_report_rmx_list)):
            #print rsrc_report_rmx_list[index]
            #rsrc_report_rmx = rsrc_report_rmx_list[index].getElementsByTagName("RSRC_REPORT_RMX")
            #print rsrc_report_rmx
            print "list len is:" +str(len(rsrc_report_rmx_list)) + " index is: " + str(index) + " item is: " + self.GetTextUnder("RSRC_REPORT_RMX","RSRC_REPORT_ITEM",index) + " free: " + self.GetTextUnder("RSRC_REPORT_RMX","FREE",index)
            if(party_type == self.GetTextUnder("RSRC_REPORT_RMX","RSRC_REPORT_ITEM",index)):
                my_string = self.GetTextUnder("RSRC_REPORT_RMX","FREE",index)
                break
    

        if my_string !=  str(num_of_resource):
            print self.xmlResponse.toprettyxml()
            sys.exit("error, free resources are: " + my_string + " and not " + str(num_of_resource) + " as expected")
            
#------------------------------------------------------------------------------
    def TestFreeCarmelParties(self,num_of_resource):
        self.TestResourcesReportPortsType("video",num_of_resource,"FREE")
#------------------------------------------------------------------------------           
    def TestReservedCarmelParties(self,num_of_resource):
        self.TestResourcesReportPortsType("video",num_of_resource,"RESERVED")             
#------------------------------------------------------------------------------           
    def TestOccupiedCarmelParties(self,num_of_resource):
        self.TestResourcesReportPortsType("video",num_of_resource,"OCCUPIED")
#------------------------------------------------------------------------------
    def GetFreeCarmelParties(self):
        return self.GetNumResources("video","FREE")
#------------------------------------------------------------------------------
    def GetOccupiedCarmelParties(self):
        return self.GetNumResources("video","OCCUPIED")
#------------------------------------------------------------------------------
    def GetReservedCarmelParties(self):
        return self.GetNumResources("video","RESERVED")    
#------------------------------------------------------------------------------
    def GetTotalCarmelParties(self):
        result = self.GetNumResources("video","TOTAL")
        print "GetTotalCarmelParties = " + str(result)
        return int(result)
#------------------------------------------------------------------------------
    def SimRemoveCard(self,boardId,subBoardId=1):  
    	print "Removing board " + str(boardId) + " SubBoard: " + str(subBoardId)   
        self.LoadXmlFile('Scripts/SimRemoveCardEvent.xml')
        self.ModifyXml("REMOVE_CARD_EVENT", "BOARD_ID", boardId)
        self.ModifyXml("REMOVE_CARD_EVENT", "SUB_BOARD_ID", subBoardId)    
        self.Send()    
#------------------------------------------------------------------------------
    def SimInsertCard(self,cardType,boardId, subBoardId=1):
    	#cardtype: taken from eCardType in CardsStructs.h (for example: eMpmPlus_80 = 9, eMfa_26 = 3)
    	print "Inserting board " + str(boardId) + " SubBoard: " + str(subBoardId) + " CardType: " +  str(cardType)
        self.LoadXmlFile('Scripts/SimInsertCardEvent.xml')
        self.ModifyXml("INSERT_CARD_EVENT", "BOARD_ID", boardId)
        self.ModifyXml("INSERT_CARD_EVENT", "SUB_BOARD_ID", subBoardId)    
        self.AddXML("INSERT_CARD_EVENT", "CARD_TYPE", str(cardType))    
        self.Send()   
#------------------------------------------------------------------------------
    def SimDisableUnit(self,boardId,unitId):
        print "Disable unit " + str(unitId) + " on board " + str(boardId)
        self.SimSetUnitStatus(boardId,unitId,3)
#------------------------------------------------------------------------------
    def SimEnableUnit(self,boardId,unitId):
        print "Enable unit " + str(unitId) + " on board " + str(boardId)
        self.SimSetUnitStatus(boardId,unitId,0)
        sleep(1)
#------------------------------------------------------------------------------
    def SimSetUnitStatus(self,boardId,unitId,status):
        self.LoadXmlFile('Scripts/keep_allive/SetUnitStatus.xml')
        self.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","BOARD_ID",boardId)
        self.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","UNIT_ID",unitId)
        self.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","STATUS",status)
        self.Send()
#------------------------------------------------------------------------------
    def SimDisableCard(self,boardId):
		visualBoardId = self.NormalBoardIdToVisualBoardId(boardId)
		self.LoadXmlFile('Scripts/ResourceDetailParty/CardDetailReq.xml')
		self.ModifyXml("GET","BOARD_NUMBER",visualBoardId)
		self.Send()
		unit_summary = self.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
		unit_len =len(unit_summary)
		for i in range(unit_len):
		    self.SimDisableUnit(boardId,unit_summary[i].getElementsByTagName("UNIT_NUMBER")[0].firstChild.data)
#------------------------------------------------------------------------------
    def SimEnableCard(self,boardId):
		visualBoardId = self.NormalBoardIdToVisualBoardId(boardId)
		self.LoadXmlFile('Scripts/ResourceDetailParty/CardDetailReq.xml')
		self.ModifyXml("GET","BOARD_NUMBER",visualBoardId)
		self.Send()
		unit_summary = self.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
		unit_len =len(unit_summary)
		for i in range(unit_len):
		    self.SimEnableUnit(boardId,unit_summary[i].getElementsByTagName("UNIT_NUMBER")[0].firstChild.data)
#------------------------------------------------------------------------------
    def NormalBoardIdToVisualBoardId(self,boardId,subBoardId = 1):	
    	if self.IsAmos:        
		if subBoardId == 1:
		    #for audio/video boards it's 2, 3, 4, 5: so it's board + 1
			return boardId;#NOTE that in RMX it will = boardId+1
		#for RTM it's 13, 14, 15, 16: so it's board + 12	
		return boardId+12	
        return boardId	    
#------------------------------------------------------------------------------
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
    def TryAddProfileWithRate(self, profileName, transfer_rate, video_quality = "auto", fileName="Scripts/CreateNewProfile.xml", num_retries=60):
        print "Adding new Profile with rate " + str(transfer_rate) + " and video quality " + video_quality
        self.LoadXmlFile(fileName)
        self.ModifyXml("RESERVATION","NAME", profileName)
        self.ModifyXml("RESERVATION","TRANSFER_RATE", str(transfer_rate))
        self.ModifyXml("RESERVATION","VIDEO_QUALITY", video_quality)        
        self.Send("") #no ScriptAbort
	success = False
	ProfId = -1
	for retry in range(num_retries):

        	self.SendXmlFile('Scripts/GetProfileList.xml')
        	profile_list = self.xmlResponse.getElementsByTagName("PROFILE_SUMMARY")
        	for index in range(len(profile_list)):
            		currentProfName = profile_list[index].getElementsByTagName("NAME")[0].firstChild.data
            		currentProfId = profile_list[index].getElementsByTagName("ID")[0].firstChild.data
            		if profileName == currentProfName:
                		print "Profile " + profileName + " added to profile list with id " + str(currentProfId)
                		success = True
				ProfId = currentProfId
                		break
        	if success == True:
            		break
        return ProfId  
#------------------------------------------------------------------------------
    def WaitUntilStartupEnd(self):
        print "Wait until startup ends"
        title = "Startup failed!"
        for x in range(150):
            self.SendXmlFile('Scripts/GetMcuState.xml')
            state = self.GetTextUnder("MCU_STATE","DESCRIPTION")
            print "Mcu status:" + state
            if state != "Startup":
                print "End Startup"
                return
            sleep(2)
        sys.exit("Startup didn't finish after 5 minutes!!!")	
#------------------------------------------------------------------------------
    def IsMPMMode(self):
        #this function can be called only immediately after connect, because then we have the data
        card_mode = self.GetTextUnder("LOGIN","SYSTEM_CARDS_MODE")
        print "received card mode as: " + card_mode
        if(card_mode == "mpm_plus"):
            return 0
        if(card_mode == "mpm"):
            return 1
        sys.exit("Unknown CardMode")	
#------------------------------------------------------------------------------
    def SetResolutionSlider(self, max_resolution = "hd1080p60", config_type = "balanced", dic = {}):
        self.LoadXmlFile('Scripts/SetResolutionSlider.xml')
        self.ModifyXml("SET_RESOLUTIONS_PARAMS", "CP_MAX_RESOLUTION", max_resolution)
        self.ModifyXml("SET_RESOLUTIONS_PARAMS", "CONFIGURATION_TYPE", config_type)
        
        if config_type != "manual":
            print "----- SetResolutionSlider -----" + config_type
            #print self.loadedXml.toprettyxml(encoding="utf-8")
            self.Send()
            return
        
        i = 0
        for x in dic["SET_SHARPNESS_RESOLUTIONS"]:
            self.loadedXml.getElementsByTagName("SET_RESOLUTIONS_PARAMS")[0] \
                .getElementsByTagName("SET_SHARPNESS_RESOLUTIONS")[0] \
                .getElementsByTagName("RESOLUTION_SLIDER_SHORT_PARAMS_LIST")[0] \
                .getElementsByTagName("RESOLUTION_SLIDER_SHORT_PARAMS")[i] \
                .getElementsByTagName("MINIMAL_RESOLUTION_RATE")[0] \
                .firstChild.data = x
            i = i +1
        
        i = 0
        for x in dic["SET_MOTION_RESOLUTIONS"]:
            self.loadedXml.getElementsByTagName("SET_RESOLUTIONS_PARAMS")[0] \
                .getElementsByTagName("SET_MOTION_RESOLUTIONS")[0] \
                .getElementsByTagName("RESOLUTION_SLIDER_SHORT_PARAMS_LIST")[0] \
                .getElementsByTagName("RESOLUTION_SLIDER_SHORT_PARAMS")[i] \
                .getElementsByTagName("MINIMAL_RESOLUTION_RATE")[0] \
                .firstChild.data = x
            i = i + 1
                
        i = 0
        for x in dic["SET_SHARPNESS_HIGH_PROFILE_RESOLUTIONS"]:
            self.loadedXml.getElementsByTagName("SET_RESOLUTIONS_PARAMS")[0] \
                .getElementsByTagName("SET_SHARPNESS_HIGH_PROFILE_RESOLUTIONS")[0] \
                .getElementsByTagName("RESOLUTION_SLIDER_SHORT_PARAMS_LIST")[0] \
                .getElementsByTagName("RESOLUTION_SLIDER_SHORT_PARAMS")[i] \
                .getElementsByTagName("MINIMAL_RESOLUTION_RATE")[0] \
                .firstChild.data = x
            i = i + 1
                
        i = 0
        for x in dic["SET_MOTION_HIGH_PROFILE_RESOLUTIONS"]:
            self.loadedXml.getElementsByTagName("SET_RESOLUTIONS_PARAMS")[0] \
                .getElementsByTagName("SET_MOTION_HIGH_PROFILE_RESOLUTIONS")[0] \
                .getElementsByTagName("RESOLUTION_SLIDER_SHORT_PARAMS_LIST")[0] \
                .getElementsByTagName("RESOLUTION_SLIDER_SHORT_PARAMS")[i] \
                .getElementsByTagName("MINIMAL_RESOLUTION_RATE")[0] \
                .firstChild.data = x
            i = i + 1
        
        print "----- SetResolutionSlider -----" + config_type
        #print self.loadedXml.toprettyxml(encoding="utf-8")
        self.Send()
        return
             
                 
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------           
#------------------------------------------------------------------------------           
#----------------------END CALSS ResourceUtilities-----------------------------           
#------------------------------------------------------------------------------           
#------------------------------------------------------------------------------           
#------------------------------------------------------------------------------           

#------------------------------------------------------------------------------           
def SimpleNonTerminateXConfWithYParticipaints(connection,numOfConfs,numOfParties,
                                  confFile,
                                  partyFile,
                                  numRetries):
    confIdArray = [0]*numOfConfs
    for confNum in range(numOfConfs):
        confname = "Conf"+str(confNum+1)
        connection.CreateConf(confname, confFile)

        print "Wait untill Conf number " + str(confNum+1) + " will be createtd...",
        confid = connection.WaitConfCreated(confname,numRetries)
        confIdArray[confNum] = confid

        connection.LoadXmlFile(partyFile)        
        for x in range(numOfParties):
            partyname = "Party" + str(x+1) + str(confIdArray[confNum])
            partyip =  "1.2."+ str(confNum+1) + "." + str(x+1)
            connection.AddParty(confIdArray[confNum], partyname, partyip, partyFile)
            
        connection.WaitAllPartiesWereAdded(confIdArray[confNum],numOfParties,numRetries)
        connection.WaitAllOngoingConnected(confIdArray[confNum],numRetries)

    ############# for our use here no need to delete !!! ###########
    #for deletedConfNum in range(numOfConfs):
        #print "Delete Conference..."
     #   connection.DeleteConf(confIdArray[deletedConfNum])
         
    #print "Wait until no conferences"
    #connection.WaitAllConfEnd(50)
    
    return

#############################################################################################
#------------------------------------------------------------------------------
def AddConf(connection,confName,profileID,num_retries):
    connection.CreateConfFromProfile(confName, profileID, 'Scripts/ResourceMultMoveUndef/CreateNewConf.xml')
    confid = connection.WaitConfCreated(confName,num_retries)
    return confid
#------------------------------------------------------------------------------           
def FindConfInConfList(connection,confList,targetName):
    confId = ""
    for i in range(len(confList)):
        confName = confList[i].getElementsByTagName("NAME")[0].firstChild.data
        confId = confList[i].getElementsByTagName("ID")[0].firstChild.data
        if confName == targetName:
            return targetName,confId
    return targetName+"Error",confId
#------------------------------------------------------------------------------
def WaitUntilPartyConnected(connection,confid,num_of_parties,num_retries):
    print "Wait untill party will move to target conf...",
    connection.LoadXmlFile('Scripts/ResourceMultMoveUndef/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    
    for retry in range(num_retries+1):
        connection.Send()
        ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
        if (len(ongoing_party_list) ==  num_of_parties):
            partyId  = ongoing_party_list[0].getElementsByTagName("ID")[0].firstChild.data
            partyName = ongoing_party_list[0].getElementsByTagName("NAME")[0].firstChild.data
            #print "Party "+ partyName + " with id: " + str(partyId) +" Was moved to the target conf" 
            print "The number of parties = "+ str(num_of_parties) + " were moved to the target conf" 
            break
        if (retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            sys.exit("Can not find moved Party in the target conf")
        sys.stdout.write(".")
        sys.stdout.flush()
    return 

#------------------------------------------------------------------------------
def GetConfNumericId(connection,targetConfID):
    connection.LoadXmlFile('Scripts/ResourceMultMoveUndef/TransConf2.xml')
    connection.ModifyXml("GET","ID",targetConfID)
    connection.Send()
    numericId = connection.GetTextUnder("RESERVATION","NUMERIC_ID")
    return numericId

#------------------------------------------------------------------------------

def GetMasterAudioCntrlrBoardId():
## The output of ShowMasterAC :
## Command : ShowMasterAC   Process: ***** Resource *****
## Master Audio Controller is on board : n

	MasterBoardId = os.popen('Bin/McuCmd @ShowMasterAC Resource | tail -n1').read().split(" : ")[1].rstrip("\n")

	return MasterBoardId

#------------------------------------------------------------------------------
def GetSlaveAudioCntrlrBoardId():
## The output of ShowSlaveAC :
## Command : ShowSlaveAC   Process: ***** Resource *****
## Slave Audio Controller is on board : n

	SlaveBoardId = os.popen('Bin/McuCmd @ShowSlaveAC Resource | tail -n1').read().split(" : ")[1].rstrip("\n")

	return SlaveBoardId

#------------------------------------------------------------------------------
def GetCapacity( boardId, subBoardId, unitId ):
## The output of ShowCap :
## Command : ShowCap   Process: ***** Resource *****
## Total capacity is : 0 (Parties/Promil)

	command = "Bin/McuCmd @ShowCap Resource " + str(boardId) + " " + str(subBoardId) + " " + str(unitId) +  "| tail -n1"
	output = os.popen( command ).read()
	r = re.compile('\d+') # search for decimal number
	capacity = r.findall(output)[0]
	                  
	return string.atoi(capacity)

##--------------------------------------- TEST ---------------------------------
if __name__ == '__main__':
	c = McmsConnection()
	c.Connect()
	
	print GetCapacity(1,1, 0xFFFF)
			
	c.Disconnect()
##------------------------------------------------------------------------------
	
