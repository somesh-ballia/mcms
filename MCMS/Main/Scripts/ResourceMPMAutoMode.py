#!/mcms/python/bin/python

#############################################################################
#Script which tests auto allocation mode and configuration for MPM:
#-reading the configuration file
#-testing full video capacity 
#-changing the slider + reset
#-and much more...
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_5
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpm.txt"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_80Video.xml"
#-- SKIP_ASSERTS
#-LONG_SCRIPT_TYPE
   
import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

   
print "============================================================"
print "Checking startup and the expected configuration - it should be auto mode with 160-video 0-audio" 
print "-----------------------------------------------------------"
if c.IsThereAnActiveAlarmOfAllocationModeChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Allocation Mode Change at the beginning of the test ")  
if c.IsThereAnActiveAlarmOfPortConfigurationChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Port Configuration Change at the beginning of the test ")  
c.CheckModes("auto","auto","Illegal in mpm mode")
c.CheckAudioVideoUnits(5, 20, 1)
c.CheckAudioVideoUnits(5, 20, 2)
c.CheckAudioVideoAutoConfiguration(0, 80)
c.TestResourcesReportPortsType("video",80,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",80,"FREE")
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
c.SetEnhancedConfiguration(200,40,15,10,1, "Illegal in not fixed mpm plus mode")
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
print "Trying to add 40 parties (x2). They should all connect"
num_of_parties = 40
c.AddVideoParties(confid,num_of_parties)
c.WaitAllOngoingConnected(confid)
print "Indeed succeeded to connect all parties"
print "The parties that we added are all x2, so 40 parties should take all the resources"
c.TestResourcesReportPortsType("video",80,"TOTAL")
c.TestResourcesReportPortsType("video",80,"OCCUPIED")
c.TestResourcesReportPortsType("video",0,"FREE")
print "============================================================"
print "Disconnect parties (4 every time), and see that the resources are freed (8 resources per 4 parties). At the all resources are freed" 
print "-----------------------------------------------------------"  
c.LoadXmlFile('Scripts/TransConf2.xml')
c.ModifyXml("GET","ID",confid)
c.Send()
ongoing_parties = c.xmlResponse.getElementsByTagName("PARTY")
num_ongoing_parties = len(ongoing_parties)
for x in range(num_ongoing_parties):
  partyId = ongoing_parties[x].getElementsByTagName("ID")[0].firstChild.data
  print "Disconnecting party " + str(partyId)
  c.LoadXmlFile('Scripts/ReconnectParty/TransSetConnect.xml')
  c.ModifyXml("SET_CONNECT","ID",confid)
  c.ModifyXml("SET_CONNECT","CONNECT","false")
  c.ModifyXml("SET_CONNECT","PARTY_ID",partyId)
  c.Send()
  if (x+1)%4 == 0 :
    sleep(1) 
    c.TestResourcesReportPortsType("video",80-2*(x+1),"OCCUPIED")
    c.TestResourcesReportPortsType("video",2*(x+1),"FREE")
print "============================================================"
print "Changing the configuration to all audio (400) - still in auto mode" 
print "-----------------------------------------------------------"
c.SetAutoPortConfiguration(40,"System should be reset") 
c.CheckAudioVideoAutoConfiguration(400, 0)
if c.IsThereAnActiveAlarmOfPortConfigurationChange() != 1 :
   	c.Disconnect()                
   	sys.exit("There's NO active alarm of Port Configuration Change")
print "There's an active alarm of Port Configuration Change - as expected" 	  
c.CheckAudioVideoAutoConfiguration(400, 0)
c.Disconnect()
print "Resetting MCU in order for new configuration to take effect"
os.environ["CLEAN_CFG"]="NO"
os.environ["RESOURCE_SETTING_FILE"]=""
os.system("Scripts/Destroy.sh")
os.system("Scripts/Startup.sh")
c.Connect()
if c.IsThereAnActiveAlarmOfAllocationModeChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Allocation Mode Change after reset ")  
if c.IsThereAnActiveAlarmOfPortConfigurationChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Port Configuration Change after reset ")  
c.CheckAudioVideoAutoConfiguration(400, 0)
c.TestResourcesReportPortsType("video",0,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",0,"FREE")
c.TestResourcesReportPortsType("audio",400,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",400,"FREE")
c.CheckAudioVideoUnits(25, 0, 1)
c.CheckAudioVideoUnits(25, 0, 2)
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
print "Now adding 5 conferences, each one with 80 audio only parties"
c.CreateConfWithAudioOnlyParties("Conf80_1", 80)
c.CreateConfWithAudioOnlyParties("Conf80_2", 80)
c.CreateConfWithAudioOnlyParties("Conf80_3", 80)
c.CreateConfWithAudioOnlyParties("Conf80_4", 80)
c.CreateConfWithAudioOnlyParties("Conf80_5", 80)
c.TestResourcesReportPortsType("audio",400,"TOTAL")
c.TestResourcesReportPortsType("audio",400,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")

c.Disconnect()

