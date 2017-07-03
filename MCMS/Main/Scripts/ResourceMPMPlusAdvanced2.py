#!/mcms/python/bin/python

#############################################################################
#Script which tests what happens when at startup there's only one card (in fixed mode and in auto mode). 
#It will also check moving from auto to fixed mode
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_5
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_160CIF.xml"
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK_ONE_CARD.XML"
#-LONG_SCRIPT_TYPE

import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()
c.WaitUntilStartupEnd()

print "============================================================"
print "Checking startup and the expected configuration - it should be auto mode with 160-video 0-audio"
print "Resource report should NOT be affected BUT with an active alarm (because there's only one board)" 
print "-----------------------------------------------------------"
c.CheckModes("fixed","fixed") 	
c.CheckEnhancedConfiguration(0, 160, 0, 0, 0, "The current slider settings require more system resources than are currently available")
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
c.TestResourcesReportPortsType("CIF",160,"TOTAL")
c.TestResourcesReportPortsType("CIF",0,"OCCUPIED")
c.TestResourcesReportPortsType("CIF",160,"FREE")
c.TestResourcesReportPortsType("SD",0,"TOTAL")
c.TestResourcesReportPortsType("SD",0,"OCCUPIED")
c.TestResourcesReportPortsType("SD",0,"FREE")
c.TestResourcesReportPortsType("HD720",0,"TOTAL")
c.TestResourcesReportPortsType("HD720",0,"OCCUPIED")
c.TestResourcesReportPortsType("HD720",0,"FREE")
c.TestResourcesReportPortsType("HD1080",0,"TOTAL")
c.TestResourcesReportPortsType("HD1080",0,"OCCUPIED")
c.TestResourcesReportPortsType("HD1080",0,"FREE")
if c.IsThereAnActiveAlarmOfInsufficientResources() == 0 :
   	c.Disconnect()                
   	sys.exit("There's NO active alarm of insufficient resources")  
print "There's indeed an active alarm of insufficient resources"
print "============================================================"
print "With only one board, get_check enhanced configuration. Also try setting invalid configuration" 
print "-----------------------------------------------------------"
c.GetCheckEnhancedConfiguration("CONFIG_SYSTEM_MAXIMUM", 0, 0, 0, 0, 0, 400, 80, 30, 20, 10)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 0, 0, 0, 400, 80, 30, 20, 10)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 400, 0, 0, 0, 0, 400, 0, 0, 0, 0)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 80, 0, 0, 0, 0, 80, 0, 0, 0)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 200, 0, 0, 0, 0, 400, 40, 15, 10, 5)
c.GetCheckEnhancedConfiguration("CONFIG_OPTIONAL_MAXIMUM", 0, 0, 0, 10, 5, 0, 0, 0, 10, 5)
c.SetEnhancedConfiguration(401,0,0,0,0, "Invalid port configuration")
print "============================================================"
print "With one board only, check that only 80 CIF parties will be able to connect" 
print "-----------------------------------------------------------"  
profId=c.AddProfileWithRate("ProfRate128",128)
print "Starting one conf with 80 CIF parties"
confname = "Conf1"
num_of_parties = 80
confid = c.CreateConfFromProfileWithVideoParties(confname,profId,num_of_parties)
c.WaitAllOngoingConnected(confid)
print "Starting second conf with 80 CIF parties"
confname = "Conf2"
confid2 = c.CreateConfFromProfileWithVideoParties(confname,profId,num_of_parties)
sleep(3)
num_connected = c.GetNumConnectedParties(confid2)
if (num_connected == 0) :
	print "Connected 0 parties"
else : 
    print "Unexpected number of connected parties : " + str(num_connected) + ". It should have been 0" 
    c.Disconnect()                
    sys.exit("Unexpected number of connected parties")  
c.TestResourcesReportPortsType("CIF",160,"TOTAL")
c.TestResourcesReportPortsType("CIF",80,"OCCUPIED")
c.TestResourcesReportPortsType("CIF",80,"FREE")
print "Disconnecting all parties"
c.ConnectDisconnectAllParties(confid, "false")
c.WaitAllOngoingDisConnected(confid)
c.ConnectDisconnectAllParties(confid2, "false")
c.WaitAllOngoingDisConnected(confid2)     
c.TestResourcesReportPortsType("CIF",160,"TOTAL")
c.TestResourcesReportPortsType("CIF",0,"OCCUPIED")
c.TestResourcesReportPortsType("CIF",160,"FREE")  
print "============================================================"
print "Insert board, check that the active alarm disappears, that the resource report is updated, that the get_check is back up, and that all 160 parties can now be connected" 
print "-----------------------------------------------------------"
c.SimInsertCard(9,2)  
sleep(10)
if c.IsThereAnActiveAlarmOfInsufficientResources() == 1 :
   	c.Disconnect()                
   	sys.exit("There still is an active alarm of insufficient resources!!!")  
print "There's indeed NO active alarm of insufficient resources"
c.TestResourcesReportPortsType("CIF",160,"TOTAL")
c.TestResourcesReportPortsType("CIF",0,"OCCUPIED")
c.TestResourcesReportPortsType("CIF",160,"FREE")  
c.GetCheckEnhancedConfiguration("CONFIG_SYSTEM_MAXIMUM", 0, 0, 0, 0, 0, 800, 160, 60, 40, 20)
c.ConnectDisconnectAllParties(confid, "true")
c.ConnectDisconnectAllParties(confid2, "true")
c.WaitAllOngoingConnected(confid)
c.WaitAllOngoingConnected(confid2)
print "Indeed succeeded connecting 160 participants again"    
print "============================================================"
print "Changing the configuration to auto mode. Resetting, in order to reload the 'one board' configuration" 
print "-----------------------------------------------------------"  
c.SetMode("auto")
c.Disconnect()
os.environ["CLEAN_CFG"]="NO"
os.environ["RESOURCE_SETTING_FILE"]="VersionCfg/Resource_Setting_80Video.xml"
os.system("Scripts/Destroy.sh")
os.system("Scripts/Startup.sh")
c.Connect()
c.WaitUntilStartupEnd()
if c.IsThereAnActiveAlarmOfAllocationModeChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Allocation Mode Change after reset ")  
if c.IsThereAnActiveAlarmOfPortConfigurationChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Port Configuration Change after reset ")
c.CheckModes("auto","auto")
c.CheckAudioVideoAutoConfiguration(0, 160)
print "============================================================"
print "With one board only. Check that it DOES affects the resource report, and that an active alarm is added " 
print "-----------------------------------------------------------"  
c.TestResourcesReportPortsType("video",80,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",80,"FREE")
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
if c.IsThereAnActiveAlarmOfInsufficientResources() == 0 :
   	c.Disconnect()                
   	sys.exit("There's NO active alarm of insufficient resources")  
print "There's indeed an active alarm of insufficient resources"
print "============================================================"
print "With one board only, check that only 80 CIF parties will be able to connect" 
print "-----------------------------------------------------------"  
print "Starting one conf with 80 CIF parties"
confname = "Conf1"
num_of_parties = 80
confid = c.CreateConfFromProfileWithVideoParties(confname,profId,num_of_parties)
c.WaitAllOngoingConnected(confid)
print "Starting second conf with 80 CIF parties"
confname = "Conf2"
confid2 = c.CreateConfFromProfileWithVideoParties(confname,profId,num_of_parties)
sleep(3)
num_connected = c.GetNumConnectedParties(confid2)
if (num_connected == 0) :
	print "Connected 0 parties"
else : 
    print "Unexpected number of connected parties : " + str(num_connected) + ". It should have been 0" 
    c.Disconnect()                
    sys.exit("Unexpected number of connected parties")  
c.TestResourcesReportPortsType("video",80,"TOTAL")
c.TestResourcesReportPortsType("video",80,"OCCUPIED")
c.TestResourcesReportPortsType("video",0,"FREE")
print "Disconnecting all parties"
c.ConnectDisconnectAllParties(confid, "false")
c.WaitAllOngoingDisConnected(confid)
c.ConnectDisconnectAllParties(confid2, "false")
c.WaitAllOngoingDisConnected(confid2)
c.TestResourcesReportPortsType("video",80,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",80,"FREE")
print "============================================================"
print "Insert board, check that the active alarm disappears, that the resource report is updated, and that all 160 parties can now be connected" 
print "-----------------------------------------------------------"
c.SimInsertCard(9,2)  
sleep(10)
if c.IsThereAnActiveAlarmOfInsufficientResources() == 1 :
   	c.Disconnect()                
   	sys.exit("There still is an active alarm of insufficient resources!!!")  
print "There's indeed NO active alarm of insufficient resources"
c.CheckAudioVideoAutoConfiguration(0, 160)
c.TestResourcesReportPortsType("video",160,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",160,"FREE")
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
c.ConnectDisconnectAllParties(confid, "true")
c.ConnectDisconnectAllParties(confid2, "true")
c.WaitAllOngoingConnected(confid)
c.WaitAllOngoingConnected(confid2)
print "Indeed succeeded connecting 160 participants again"    

c.Disconnect()

