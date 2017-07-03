#!/mcms/python/bin/python

#############################################################################
#Script which tests switching between modes with no reset
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_5
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_1600AUDIO_Amos_Voice.xml"
#-- SKIP_ASSERTS
#-LONG_SCRIPT_TYPE
   
import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

profId = c.AddProfile("profile")
profIdCIF = c.AddProfileWithRate("ProfileCIF",128,"motion")
t = datetime.utcnow( )
deltat = timedelta(0,0,0,0,0,2,0)
t = t + deltat
   
print "============================================================"
print "Checking startup and the expected configuration - it should be fixed mode with 1600-audio" 
print "-----------------------------------------------------------"
if c.IsThereAnActiveAlarmOfAllocationModeChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Allocation Mode Change at the beginning of the test ")  
if c.IsThereAnActiveAlarmOfPortConfigurationChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Port Configuration Change at the beginning of the test ")  
c.CheckModes("fixed","fixed")
c.CheckAudioVideoUnits(32, 0, 1)
c.CheckAudioVideoUnits(32, 0, 2)
c.CheckAudioVideoUnits(32, 0, 3)
c.CheckAudioVideoUnits(32, 0, 4)
c.CheckEnhancedConfiguration(1600, 0, 0, 0, 0)
c.TestResourcesReportPortsType("audio",1600,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",1600,"FREE")
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
print "============================================================"
print "Checking that the configuration indeed works" 
print "-----------------------------------------------------------"
print "adding 2 reservations at the same time, each one with 800 audio participants"
res_idAudio1 = c.CreateRes("Audio1", profId, t, "", 800, 0)
res_idAudio2 = c.CreateRes("Audio2", profId, t, "", 800, 0)
print "Trying to add a reservation with one more audio participant at the same time, this should fail"
res_idAudio3 = c.CreateRes("Audio3", profId, t, "", 1, 0, "Insufficient resources")
print "============================================================"
print "Changing the configuration to auto mode and check expected configuration - 320 video" 
print "-----------------------------------------------------------"  
c.SetMode("auto")
c.CheckModes("auto","auto")
if c.IsThereAnActiveAlarmOfAllocationModeChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Allocation Mode Change when changing to fixed mode")  
sleep(5)   	
c.CheckAudioVideoUnits(6, 26, 1)
c.CheckAudioVideoUnits(6, 26, 2)
c.CheckAudioVideoUnits(6, 26, 3)
c.CheckAudioVideoUnits(6, 26, 4)
c.CheckAudioVideoAutoConfiguration(0, 320)
c.TestResourcesReportPortsType("video",320,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",320,"FREE")
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
print "============================================================"
print "Checking that the configuration indeed works" 
print "-----------------------------------------------------------"
c.CheckResStatus(res_idAudio1,"suspended")
c.CheckResStatus(res_idAudio2,"suspended")
print "adding 2 reservations at the same time, each one with 160 CIF participants"
res_idVideo1 = c.CreateRes("Video1", profIdCIF, t, "", 0, 160)
res_idVideo2 = c.CreateRes("Video2", profIdCIF, t, "", 0, 160)
print "Trying to add a reservation with one more video participant at the same time, this should fail"
res_idVideo3 = c.CreateRes("Video3", profIdCIF, t, "", 0, 1, "Insufficient resources")
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
print "Trying to add 120 parties (SD30). They should all connect, but will require some reconfiguration of units"
num_of_parties = 120
c.AddVideoParties(confid,num_of_parties)
sleep(10)
#Some of the parties will require reconnecting, because some of the units had to be reconfigured. Retry several times
c.ReconnectDisconnectedParties(confid)
sleep(5)
c.ReconnectDisconnectedParties(confid)
sleep(5)
c.WaitAllOngoingConnected(confid)
print "Indeed succeeded to connect all parties"
print "The parties that we added are all SD30, so 120 parties should take all the resources"
c.TestResourcesReportPortsType("video",320,"TOTAL")
c.TestResourcesReportPortsType("video",320,"OCCUPIED")
c.TestResourcesReportPortsType("video",0,"FREE")
c.CheckAudioVideoUnits(2, 30, 1)
c.CheckAudioVideoUnits(2, 30, 2)
c.CheckAudioVideoUnits(2, 30, 3)
c.CheckAudioVideoUnits(2, 30, 4)
print "============================================================"
print "Disconnect parties and see that the resources are freed" 
print "-----------------------------------------------------------"  
c.DeleteAllConf()
sleep(10)
c.WaitAllConfEnd() 
c.TestResourcesReportPortsType("video",320,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",320,"FREE")   
print "============================================================"
print "Changing the configuration to all audio (1600) - still in auto mode" 
print "-----------------------------------------------------------"
c.SetAutoPortConfiguration(160) 
c.SetAutoPortConfiguration(0,"System is applying last configuration. Please try again") 
if c.IsThereAnActiveAlarmOfPortConfigurationChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Port Configuration Change")
print "There's NO active alarm of Port Configuration Change - as expected" 	
sleep(5)
c.CheckAudioVideoAutoConfiguration(1600, 0)
c.TestResourcesReportPortsType("video",0,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",0,"FREE")
c.TestResourcesReportPortsType("audio",1600,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",1600,"FREE")
c.CheckAudioVideoUnits(32, 0, 1)
c.CheckAudioVideoUnits(32, 0, 2)
c.CheckAudioVideoUnits(32, 0, 3)
c.CheckAudioVideoUnits(32, 0, 4)
c.CheckResStatus(res_idAudio1,"ok")
c.CheckResStatus(res_idAudio2,"ok")
c.CheckResStatus(res_idVideo1,"suspended")
c.CheckResStatus(res_idVideo2,"suspended")
print "============================================================"
print "Changing the configuration back to all video 320 - still in auto mode" 
print "-----------------------------------------------------------"
c.SetAutoPortConfiguration(0)
sleep(5)
if c.IsThereAnActiveAlarmOfPortConfigurationChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Port Configuration Change")
print "There's NO active alarm of Port Configuration Change - as expected" 	  
c.CheckAudioVideoAutoConfiguration(0, 320)
c.TestResourcesReportPortsType("video",320,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",320,"FREE")
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
c.CheckAudioVideoUnits(6, 26, 1)
c.CheckAudioVideoUnits(6, 26, 2)
c.CheckAudioVideoUnits(6, 26, 3)
c.CheckAudioVideoUnits(6, 26, 4)
c.CheckResStatus(res_idAudio1,"suspended")
c.CheckResStatus(res_idAudio2,"suspended")
c.CheckResStatus(res_idVideo1,"ok")
c.CheckResStatus(res_idVideo2,"ok")
print "============================================================"
print "Resetting to check that configuration is as expected"
print "-----------------------------------------------------------"  
c.Disconnect()
print "Resetting MCU to check that it didn't affect the current configuration"
os.environ["CLEAN_CFG"]="NO"
os.environ["RESOURCE_SETTING_FILE"]=""
os.system("Scripts/Destroy.sh")
os.system("Scripts/Startup.sh")
c.Connect()
c.CheckAudioVideoAutoConfiguration(0, 320)
c.TestResourcesReportPortsType("video",320,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",320,"FREE")
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
c.CheckAudioVideoUnits(6, 26, 1)
c.CheckAudioVideoUnits(6, 26, 2)
c.CheckAudioVideoUnits(6, 26, 3)
c.CheckAudioVideoUnits(6, 26, 4)
c.CheckResStatus(res_idAudio1,"suspended")
c.CheckResStatus(res_idAudio2,"suspended")
c.CheckResStatus(res_idVideo1,"ok")
c.CheckResStatus(res_idVideo2,"ok")
print "============================================================"
print "Changing the configuration to fixed mode and check expected configuration - 1600 audio" 
print "-----------------------------------------------------------"  
c.SetMode("fixed")
c.CheckModes("fixed","fixed")
if c.IsThereAnActiveAlarmOfAllocationModeChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Allocation Mode Change when changing to fixed mode")  
sleep(5)   	
c.CheckAudioVideoUnits(32, 0, 1)
c.CheckAudioVideoUnits(32, 0, 2)
c.CheckAudioVideoUnits(32, 0, 3)
c.CheckAudioVideoUnits(32, 0, 4)
c.CheckEnhancedConfiguration(1600, 0, 0, 0, 0)
c.TestResourcesReportPortsType("audio",1600,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",1600,"FREE")
c.TestResourcesReportPortsType("CIF",0,"TOTAL")
c.TestResourcesReportPortsType("CIF",0,"OCCUPIED")
c.TestResourcesReportPortsType("CGelernter, HaggaiIF",0,"FREE")
c.TestResourcesReportPortsType("SD",0,"TOTAL")
c.TestResourcesReportPortsType("SD",0,"OCCUPIED")
c.TestResourcesReportPortsType("SD",0,"FREE")
c.TestResourcesReportPortsType("HD720",0,"TOTAL")
c.TestResourcesReportPortsType("HD720",0,"OCCUPIED")
c.TestResourcesReportPortsType("HD720",0,"FREE")
c.TestResourcesReportPortsType("HD1080",0,"TOTAL")
c.TestResourcesReportPortsType("HD1080",0,"OCCUPIED")
c.TestResourcesReportPortsType("HD1080",0,"FREE")
c.CheckResStatus(res_idAudio1,"ok")
c.CheckResStatus(res_idAudio2,"ok")
c.CheckResStatus(res_idVideo1,"suspended")
c.CheckResStatus(res_idVideo2,"suspended")

c.Disconnect()

