#!/mcms/python/bin/python

#############################################################################
#Script which tests fixed allocation mode and configuration: 
#-Reading the configuration file
#-Changing the slider without reset (and checking that the units were reconfigured)
#-Testing full video capacity + full audio capacity
#-"get_check" enhanced configuration 
#-and much more...
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_5
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_20_YES_v100.0.cfs"
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK_PARTIALLY20.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_20Video.xml"

import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

print "============================================================"
print "Checking startup and the expected configuration - it should be auto mode with 20 video" 
print "-----------------------------------------------------------"
c.CheckModes("auto","auto")
c.CheckAudioVideoAutoConfiguration(0, 20)
c.CheckAudioVideoUnits(2, 7, 1)
c.TestResourcesReportPortsType("video",20,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",20,"FREE")
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
print "============================================================"
print "Checking that the configuration indeed works" 
print "-----------------------------------------------------------"
confname = "Conf1"
c.CreateConf(confname)
confid = c.WaitConfCreated(confname)  
print "Adding dial-out audio only party. It shouldn't connect"
partyname = "AudioParty"
partyip =  "1.2.3.0"
c.AddParty(confid, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutH323AudioParty.xml")
c.WaitAllPartiesWereAdded(confid,1,10)
partyId = c.GetPartyId(confid, partyname)
sleep(2)
status = c.GetPartyStatus(confid,partyId)
print "Status of party is " + status
if (status != "disconnected") & (status != "idle"):
   	c.Disconnect()                
   	sys.exit("Unexpected status of audio only party")
c.DeleteParty(confid,partyId) 
print "Trying to add 7 parties (SD30). They should all connect"
num_of_parties = 7
c.AddVideoParties(confid,num_of_parties)
c.WaitAllOngoingConnected(confid)
print "Indeed succeeded to connect all parties"
print "The parties that we added are all SD30, so 7 parties should take 19 resources (7*8/3)"
c.TestResourcesReportPortsType("video",20,"TOTAL")
c.TestResourcesReportPortsType("video",19,"OCCUPIED")
c.TestResourcesReportPortsType("video",1,"FREE")
print "============================================================"
print "Changing the configuration to fixed mode" 
print "-----------------------------------------------------------"  
c.SetMode("fixed")
if c.IsThereAnActiveAlarmOfAllocationModeChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Allocation Mode Change")  
if c.IsThereAnActiveAlarmOfPortConfigurationChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Port Configuration Change")  
c.CheckModes("fixed","fixed") 	
c.CheckEnhancedConfiguration(0, 0, 7, 0, 0)
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
c.TestResourcesReportPortsType("CIF",0,"TOTAL")
c.TestResourcesReportPortsType("CIF",0,"OCCUPIED")
c.TestResourcesReportPortsType("CIF",0,"FREE")
c.TestResourcesReportPortsType("SD",7,"TOTAL")
c.TestResourcesReportPortsType("SD",0,"OCCUPIED")
c.TestResourcesReportPortsType("SD",7,"FREE")
c.TestResourcesReportPortsType("HD720",0,"TOTAL")
c.TestResourcesReportPortsType("HD720",0,"OCCUPIED")
c.TestResourcesReportPortsType("HD720",0,"FREE")
c.TestResourcesReportPortsType("HD1080",0,"TOTAL")
c.TestResourcesReportPortsType("HD1080",0,"OCCUPIED")
c.TestResourcesReportPortsType("HD1080",0,"FREE")  
print "============================================================"
print "Checking that the configuration indeed works" 
print "-----------------------------------------------------------"
confname = "Conf1"
c.CreateConf(confname)
confid = c.WaitConfCreated(confname)  
print "Adding dial-out audio only party. It should connect, but be counted as one SD30"
partyname = "AudioParty"
partyip =  "1.2.3.0"
c.AddParty(confid, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutH323AudioParty.xml")
c.WaitAllPartiesWereAdded(confid,1,10)
partyId = c.GetPartyId(confid, partyname)
c.WaitAllOngoingConnected(confid)
c.TestResourcesReportPortsType("SD",7,"TOTAL")
c.TestResourcesReportPortsType("SD",1,"OCCUPIED")
c.TestResourcesReportPortsType("SD",6,"FREE")
c.DeleteParty(confid,partyId) 
print "Trying to add 7 parties (SD30). They should all connect"
num_of_parties = 7
c.AddVideoParties(confid,num_of_parties)
c.WaitAllOngoingConnected(confid)
print "Indeed succeeded to connect all parties"
print "The parties that we added are all SD30, so 7 parties should take all the resources"
c.TestResourcesReportPortsType("SD",7,"TOTAL")
c.TestResourcesReportPortsType("SD",7,"OCCUPIED")
c.TestResourcesReportPortsType("SD",0,"FREE")
print "============================================================"
print "Disconnect all parties, and see that the resources are freed" 
print "-----------------------------------------------------------"  
c.ConnectDisconnectAllParties(confid, "false")
c.WaitAllOngoingDisConnected(confid)
c.TestResourcesReportPortsType("SD",7,"TOTAL")
c.TestResourcesReportPortsType("SD",0,"OCCUPIED")
c.TestResourcesReportPortsType("SD",7,"FREE")
print "============================================================"
print "Try different types of get_check enhanced configuration and see that we get the expected results" 
print "-----------------------------------------------------------"
c.GetCheckEnhancedConfiguration("CONFIG_SYSTEM_MAXIMUM", 0, 0, 0, 0, 0, 100, 20, 7, 5, 2)
c.GetCheckEnhancedConfiguration("CONFIG_CURRENT", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
c.GetCheckEnhancedConfiguration("CONFIG_CURRENT", 50, 0, 0, 0, 0, 50, 0, 0, 0, 0)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 7, 0, 0, 6, 1, 7, 0, 0)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 0, 0, 0, 100, 20, 7, 5, 2)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 50, 0, 0, 0, 0, 100, 10, 3, 2, 1)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 20, 0, 0, 0, 0, 20, 0, 0, 0)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 5, 0, 0, 33, 6, 7, 1, 0)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 4, 0, 1, 0, 60, 16, 4, 4, 1)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 40, 0, 1, 1, 0, 66, 5, 3, 2, 0)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 0, 0, 1, 60, 12, 4, 3, 2)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 1, 0, 0, 0, 0, 100, 19, 7, 4, 2)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 1, 0, 0, 86, 17, 7, 4, 2)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 20, 4, 0, 1, 0, 60, 12, 3, 3, 1)
print "============================================================"
print "Try different types of enhanced configuration that should fail" 
print "-----------------------------------------------------------"
c.SetEnhancedConfiguration(0,0,5,0,0,"This action is illegal while there are ongoing conferences")
c.DeleteConf(confid)
c.WaitConfEnd(confid)
c.SetEnhancedConfiguration(101,0,0,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(0,21,0,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(0,0,8,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(0,0,0,6,0, "Invalid port configuration")
c.SetEnhancedConfiguration(0,0,0,0,3, "Invalid port configuration")
c.SetEnhancedConfiguration(51,10,0,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(51,0,4,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(51,0,0,3,0, "Invalid port configuration")
c.SetEnhancedConfiguration(61,0,0,0,1, "Invalid port configuration")
c.SetEnhancedConfiguration(21,0,0,0,2, "Invalid port configuration")
c.SetEnhancedConfiguration(26,8,0,2,0, "Invalid port configuration")
c.SetEnhancedConfiguration(100,20,7,5,2, "Invalid port configuration")
print "============================================================"
print "Try to different types of enhanced configuration that should succeed + check audio video units" 
print "-----------------------------------------------------------"
c.SetEnhancedConfiguration(99,0,0,0,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(9, 0, 1)
c.SetEnhancedConfiguration(0,19,0,0,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(2, 7, 1)
c.SetEnhancedConfiguration(0,0,6,0,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(1, 8, 1)
c.SetEnhancedConfiguration(0,0,0,4,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(1, 8, 1)
c.SetEnhancedConfiguration(0,0,0,0,2, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(1, 8, 1)
c.SetEnhancedConfiguration(20,0,0,0,2)
sleep(5)
c.CheckAudioVideoUnits(2, 7, 1)
c.SetEnhancedConfiguration(0,0,0,0,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(1, 8, 1)
print "============================================================"
print "Changing the configuration to all audio (100) - still in fixed mode, throughly check" 
print "-----------------------------------------------------------"
c.SetEnhancedConfiguration(100,0,0,0,0)
sleep(5)
c.CheckAudioVideoUnits(9, 0, 1)
print "There's NO active alarm of Port Configuration Change - as expected" 	  
c.CheckModes("fixed","fixed")
c.CheckEnhancedConfiguration(100, 0, 0, 0, 0)
c.TestResourcesReportPortsType("audio",100,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",100,"FREE")
c.TestResourcesReportPortsType("CIF",0,"TOTAL")
c.TestResourcesReportPortsType("CIF",0,"OCCUPIED")
c.TestResourcesReportPortsType("CIF",0,"FREE")
c.TestResourcesReportPortsType("SD",0,"TOTAL")
c.TestResourcesReportPortsType("SD",0,"OCCUPIED")
c.TestResourcesReportPortsType("SD",0,"FREE")
c.TestResourcesReportPortsType("HD720",0,"TOTAL")
c.TestResourcesReportPortsType("HD720",0,"OCCUPIED")
c.TestResourcesReportPortsType("HD720",0,"FREE")
c.TestResourcesReportPortsType("HD1080",0,"TOTAL")
c.TestResourcesReportPortsType("HD1080",0,"OCCUPIED")
c.TestResourcesReportPortsType("HD1080",0,"FREE")
confname = "Conf1"
c.CreateConf(confname)
confid = c.WaitConfCreated(confname)  
print "Adding dial-out video party. It shouldn't connect"
partyname = "VideoParty"
partyip =  "1.2.3.1"
c.AddParty(confid, partyname, partyip, "Scripts/CheckMultiTypesOfCalls/AddDialOutH323Party.xml")
c.WaitAllPartiesWereAdded(confid,1,10)
partyId = c.GetPartyId(confid, partyname)
sleep(2)
status = c.GetPartyStatus(confid,partyId)
print "Status of party is " + status
if (status != "disconnected") & (status != "idle"):
   	c.Disconnect()                
   	sys.exit("Unexpected status of video party")	
c.DeleteConf(confid)
print "Now adding a conference with 100 audio only parties"
c.CreateConfWithAudioOnlyParties("Conf100", 100)
c.TestResourcesReportPortsType("audio",100,"TOTAL")
c.TestResourcesReportPortsType("audio",100,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")

c.Disconnect()

