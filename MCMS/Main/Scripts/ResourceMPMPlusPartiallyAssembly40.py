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
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_40_YES_v100.0.cfs"
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK_PARTIALLY40.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_40Video.xml"

import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

print "============================================================"
print "Checking startup and the expected configuration - it should be auto mode with 40 video" 
print "-----------------------------------------------------------"
c.CheckModes("auto","auto")
c.CheckAudioVideoAutoConfiguration(0, 40)
c.CheckAudioVideoUnits(3, 13, 1)
c.TestResourcesReportPortsType("video",40,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",40,"FREE")
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
print "============================================================"
print "Checking that the co2nfiguration indeed works" 
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
print "Trying to add 15 parties (SD30). They should all connect, but will require some reconfiguration of units"
num_of_parties = 15
c.AddVideoParties(confid,num_of_parties)
sleep(10)
#Some of the parties will require reconnecting, because some of the units had to be reconfigured. Retry several times
c.ReconnectDisconnectedParties(confid)
sleep(5)
c.ReconnectDisconnectedParties(confid)
sleep(5)
c.WaitAllOngoingConnected(confid)
print "Indeed succeeded to connect all parties"
print "The parties that we added are all SD30, so 15 parties should take all the resources"
c.TestResourcesReportPortsType("video",40,"TOTAL")
c.TestResourcesReportPortsType("video",40,"OCCUPIED")
c.TestResourcesReportPortsType("video",0,"FREE")
c.CheckAudioVideoUnits(1, 15, 1)
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
c.CheckEnhancedConfiguration(0, 0, 15, 0, 0)
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
c.TestResourcesReportPortsType("CIF",0,"TOTAL")
c.TestResourcesReportPortsType("CIF",0,"OCCUPIED")
c.TestResourcesReportPortsType("CIF",0,"FREE")
c.TestResourcesReportPortsType("SD",15,"TOTAL")
c.TestResourcesReportPortsType("SD",0,"OCCUPIED")
c.TestResourcesReportPortsType("SD",15,"FREE")
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
c.TestResourcesReportPortsType("SD",15,"TOTAL")
c.TestResourcesReportPortsType("SD",1,"OCCUPIED")
c.TestResourcesReportPortsType("SD",14,"FREE")
c.DeleteParty(confid,partyId) 
print "Trying to add 15 parties (SD30). They should all connect"
num_of_parties = 15
c.AddVideoParties(confid,num_of_parties)
c.WaitAllOngoingConnected(confid)
print "Indeed succeeded to connect all parties"
print "The parties that we added are all SD30, so 15 parties should take all the resources"
c.TestResourcesReportPortsType("SD",15,"TOTAL")
c.TestResourcesReportPortsType("SD",15,"OCCUPIED")
c.TestResourcesReportPortsType("SD",0,"FREE")
print "============================================================"
print "Disconnect all parties, and see that the resources are freed" 
print "-----------------------------------------------------------"  
c.ConnectDisconnectAllParties(confid, "false")
c.WaitAllOngoingDisConnected(confid)
c.TestResourcesReportPortsType("SD",15,"TOTAL")
c.TestResourcesReportPortsType("SD",0,"OCCUPIED")
c.TestResourcesReportPortsType("SD",15,"FREE")
print "============================================================"
print "Try different types of get_check enhanced configuration and see that we get the expected results" 
print "-----------------------------------------------------------"
c.GetCheckEnhancedConfiguration("CONFIG_SYSTEM_MAXIMUM", 0, 0, 0, 0, 0, 200, 40, 15, 10, 5)
c.GetCheckEnhancedConfiguration("CONFIG_CURRENT", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
c.GetCheckEnhancedConfiguration("CONFIG_CURRENT", 100, 0, 0, 0, 0, 100, 0, 0, 0, 0)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 0, 0, 0, 200, 40, 15, 10, 5)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 100, 0, 0, 0, 0, 200, 20, 7, 5, 2)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 40, 0, 0, 0, 0, 40, 0, 0, 0)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 10, 0, 0, 66, 13, 15, 3, 1)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 3, 2, 1, 80, 16, 9, 6, 3)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 80, 0, 3, 2, 1, 80, 0, 3, 2, 1)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 0, 0, 1, 160, 32, 12, 8, 5)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 1, 0, 0, 0, 0, 200, 39, 14, 9, 4)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 1, 0, 0, 186, 37, 15, 9, 4)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 40, 8, 0, 2, 1, 80, 16, 3, 4, 2)
print "============================================================"
print "Try different types of enhanced configuration that should fail" 
print "-----------------------------------------------------------"
c.SetEnhancedConfiguration(0,0,10,0,0,"This action is illegal while there are ongoing conferences")
c.DeleteConf(confid)
c.WaitConfEnd(confid)
c.SetEnhancedConfiguration(201,0,0,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(0,41,0,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(0,0,16,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(0,0,0,11,0, "Invalid port configuration")
c.SetEnhancedConfiguration(0,0,0,0,6, "Invalid port configuration")
c.SetEnhancedConfiguration(101,20,0,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(101,0,8,0,0, "Invalid port configuration")
c.SetEnhancedConfiguration(101,0,0,5,0, "Invalid port configuration")
c.SetEnhancedConfiguration(101,0,0,0,3, "Invalid port configuration")
c.SetEnhancedConfiguration(1,0,0,0,5, "Invalid port configuration")
c.SetEnhancedConfiguration(41,8,3,2,1, "Invalid port configuration")
c.SetEnhancedConfiguration(200,40,15,10,5, "Invalid port configuration")
print "============================================================"
print "Try to different types of enhanced configuration that should succeed + check audio video units" 
print "-----------------------------------------------------------"
c.SetEnhancedConfiguration(199,0,0,0,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(16, 0, 1)
c.SetEnhancedConfiguration(0,39,0,0,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(3, 13, 1)
c.SetEnhancedConfiguration(0,0,14,0,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(1, 15, 1)
c.SetEnhancedConfiguration(0,0,0,9,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(1, 15, 1)
c.SetEnhancedConfiguration(0,0,0,0,4, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(1, 15, 1)
c.SetEnhancedConfiguration(0,0,0,0,5)
sleep(5)
c.CheckAudioVideoUnits(1, 15, 1)
c.SetEnhancedConfiguration(0,0,0,0,0, "Port configuration not optimized. There are unallocated resources in the system.")
sleep(5)
c.CheckAudioVideoUnits(1, 15, 1)
print "============================================================"
print "Changing the configuration to all audio (200) - still in fixed mode, throughly check" 
print "-----------------------------------------------------------"
c.SetEnhancedConfiguration(200,0,0,0,0)
sleep(5)
c.CheckAudioVideoUnits(16, 0, 1)
print "There's NO active alarm of Port Configuration Change - as expected" 	  
c.CheckModes("fixed","fixed")
c.CheckEnhancedConfiguration(200, 0, 0, 0, 0)
c.TestResourcesReportPortsType("audio",200,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",200,"FREE")
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
print "Now adding a conference with 200 audio only parties"
c.CreateConfWithAudioOnlyParties("Conf200_1", 200)
c.TestResourcesReportPortsType("audio",200,"TOTAL")
c.TestResourcesReportPortsType("audio",200,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")

c.Disconnect()

