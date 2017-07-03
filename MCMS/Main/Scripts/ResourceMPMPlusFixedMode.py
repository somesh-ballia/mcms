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
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_60SD.xml"
#-LONG_SCRIPT_TYPE

import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

print "============================================================"
print "Checking startup and the expected configuration - it should be fixed mode with 60 SD30, and the rest 0" 
print "-----------------------------------------------------------"
if c.IsThereAnActiveAlarmOfAllocationModeChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Allocation Mode Change at the beginning of the test ")  
if c.IsThereAnActiveAlarmOfPortConfigurationChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Port Configuration Change at the beginning of the test ")  
c.CheckModes("fixed","fixed")
c.CheckAudioVideoUnits(2, 30, 1)
c.CheckAudioVideoUnits(2, 30, 2)
c.CheckEnhancedConfiguration(0, 0, 60, 0, 0)
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
c.TestResourcesReportPortsType("CIF",0,"TOTAL")
c.TestResourcesReportPortsType("CIF",0,"OCCUPIED")
c.TestResourcesReportPortsType("CIF",0,"FREE")
c.TestResourcesReportPortsType("SD",60,"TOTAL")
c.TestResourcesReportPortsType("SD",0,"OCCUPIED")
c.TestResourcesReportPortsType("SD",60,"FREE")
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
c.TestResourcesReportPortsType("SD",60,"TOTAL")
c.TestResourcesReportPortsType("SD",1,"OCCUPIED")
c.TestResourcesReportPortsType("SD",59,"FREE")
c.DeleteParty(confid,partyId) 
print "Trying to add 60 parties (SD30). They should all connect"
num_of_parties = 60
c.AddVideoParties(confid,num_of_parties)
c.WaitAllOngoingConnected(confid)
print "Indeed succeeded to connect all parties"
print "The parties that we added are all SD30, so 60 parties should take all the resources"
c.TestResourcesReportPortsType("SD",60,"TOTAL")
c.TestResourcesReportPortsType("SD",60,"OCCUPIED")
c.TestResourcesReportPortsType("SD",0,"FREE")
print "============================================================"
print "Disconnect all parties, and see that the resources are freed" 
print "-----------------------------------------------------------"  
c.ConnectDisconnectAllParties(confid, "false")
c.WaitAllOngoingDisConnected(confid)
c.TestResourcesReportPortsType("SD",60,"TOTAL")
c.TestResourcesReportPortsType("SD",0,"OCCUPIED")
c.TestResourcesReportPortsType("SD",60,"FREE")
print "============================================================"
print "Try different types of get_check enhanced configuration and see that we get the expected results" 
print "-----------------------------------------------------------"
c.GetCheckEnhancedConfiguration("CONFIG_SYSTEM_MAXIMUM", 0, 0, 0, 0, 0, 800, 160, 60, 40, 20)
c.GetCheckEnhancedConfiguration("CONFIG_CURRENT", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
c.GetCheckEnhancedConfiguration("CONFIG_CURRENT", 400, 0, 0, 0, 0, 400, 0, 0, 0, 0)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 0, 0, 0, 800, 160, 60, 40, 20)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 400, 0, 0, 0, 0, 800, 80, 30, 20, 10)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 160, 0, 0, 0, 0, 160, 0, 0, 0)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 45, 0, 0, 200, 40, 60, 10, 5)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 15, 10, 5, 200, 40, 30, 20, 10)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 200, 0, 15, 10, 5, 200, 0, 15, 10, 5)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 0, 0, 1, 760, 152, 57, 38, 20)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 1, 0, 0, 0, 0, 800, 159, 59, 39, 19)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 1, 0, 0, 786, 157, 60, 39, 19)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 100, 20, 0, 5, 5, 400, 80, 22, 20, 12)
print "============================================================"
print "Try different types of enhanced configuration that should fail" 
print "-----------------------------------------------------------"
c.SetEnhancedConfiguration(0,0,20,0,0,"This action is illegal while there are ongoing conferences")
c.DeleteConf(confid)
c.WaitConfEnd(confid)
c.SetEnhancedConfiguration(801,0,0,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(0,161,0,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(0,0,61,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(0,0,0,41,0, "Invalid port configuration")
c.SetEnhancedConfiguration(0,0,0,0,21, "Invalid port configuration")
c.SetEnhancedConfiguration(401,80,0,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(401,0,30,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(401,0,0,20,0, "Invalid port configuration")
c.SetEnhancedConfiguration(401,0,0,0,10, "Invalid port configuration")
c.SetEnhancedConfiguration(1,0,0,0,40, "Invalid port configuration")
c.SetEnhancedConfiguration(201,40,15,10,0, "Invalid port configuration")
c.SetEnhancedConfiguration(200,40,15,10,1, "Invalid port configuration")
c.SetAutoPortConfiguration(20,"Invalid operation in Fixed Resource Capacity Mode") 
print "============================================================"
print "Try to different types of enhanced configuration that should succeed + check audio video units" 
print "-----------------------------------------------------------"
c.SetEnhancedConfiguration(799,0,0,0,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(32, 0, 1)
c.CheckAudioVideoUnits(32, 0, 2)
c.SetEnhancedConfiguration(0,159,0,0,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(6, 26, 1)
c.CheckAudioVideoUnits(7, 25, 2)
c.SetEnhancedConfiguration(0,0,59,0,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(2, 30, 1)
c.CheckAudioVideoUnits(2, 30, 2)
c.SetEnhancedConfiguration(0,0,0,39,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(2, 30, 1)
c.CheckAudioVideoUnits(2, 30, 2)
c.SetEnhancedConfiguration(0,0,0,0,19, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(2, 30, 1)
c.CheckAudioVideoUnits(2, 30, 2)
c.SetEnhancedConfiguration(0,0,0,0,20)
sleep(5)
c.CheckAudioVideoUnits(2, 30, 1)
c.CheckAudioVideoUnits(2, 30, 2)
c.SetEnhancedConfiguration(0,0,0,0,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(2, 30, 1)
c.CheckAudioVideoUnits(2, 30, 2)
print "============================================================"
print "Changing the configuration to all audio (800) - still in fixed mode, throughly check" 
print "-----------------------------------------------------------"
c.SetEnhancedConfiguration(800,0,0,0,0)
sleep(5)
c.CheckAudioVideoUnits(32, 0, 1)
c.CheckAudioVideoUnits(32, 0, 2)
#when changing configuration in fixed mode, there should be no active alarm!
if c.IsThereAnActiveAlarmOfPortConfigurationChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Port Configuration Change")
print "There's NO active alarm of Port Configuration Change - as expected" 	  
c.CheckModes("fixed","fixed")
c.CheckEnhancedConfiguration(800, 0, 0, 0, 0)
c.TestResourcesReportPortsType("audio",800,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",800,"FREE")
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
print "Now adding four conferences, each one with 200 audio only parties"
c.CreateConfWithAudioOnlyParties("Conf200_1", 200)
c.CreateConfWithAudioOnlyParties("Conf200_2", 200)
c.CreateConfWithAudioOnlyParties("Conf200_3", 200)
c.CreateConfWithAudioOnlyParties("Conf200_4", 200)
c.TestResourcesReportPortsType("audio",800,"TOTAL")
c.TestResourcesReportPortsType("audio",800,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
print "============================================================"
print "Changing the configuration to auto mode" 
print "-----------------------------------------------------------"  
c.SetMode("auto")
c.CheckModes("auto","auto")
if c.IsThereAnActiveAlarmOfAllocationModeChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's still an active alarm of Allocation Mode Change")
print "There's NO active alarm of Allocation Mode Change - as expected" 	  
print "-----------------------------------------------------------"  


c.Disconnect()

