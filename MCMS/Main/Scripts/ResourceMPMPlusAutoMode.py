#!/mcms/python/bin/python

#############################################################################
#Script which tests auto allocation mode and configuration:
#-reading the configuration file
#-testing full video capacity (including reconfiguration of units while connecting participants) 
#-changing the slider + reset
#-and much more...
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_5
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs"
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_160Video.xml"
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
c.CheckModes("auto","auto")
c.CheckAudioVideoUnits(6, 26, 1)
c.CheckAudioVideoUnits(6, 26, 2)
c.CheckAudioVideoAutoConfiguration(0, 160)
c.TestResourcesReportPortsType("video",160,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",160,"FREE")
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")
c.SetEnhancedConfiguration(200,40,15,10,1, "Invalid operation in Flexible Resource Capacity Mode in MPM+ Mode")
print "============================================================"
print "Checking that the configuration indeed works with 60 SD30" 
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
print "Trying to add 60 parties (SD30). They should all connect, but will require some reconfiguration of units"
num_of_parties = 60
c.AddVideoParties(confid,num_of_parties)
sleep(10)
#Some of the parties will require reconnecting, because some of the units had to be reconfigured. Retry several times
c.ReconnectDisconnectedParties(confid)
sleep(5)
c.ReconnectDisconnectedParties(confid)
sleep(5)
c.WaitAllOngoingConnected(confid)
print "Indeed succeeded to connect all parties"
print "The parties that we added are all SD30, so 60 parties should take all the resources"
c.TestResourcesReportPortsType("video",160,"TOTAL")
c.TestResourcesReportPortsType("video",160,"OCCUPIED")
c.TestResourcesReportPortsType("video",0,"FREE")
c.CheckAudioVideoUnits(2, 30, 1)
c.CheckAudioVideoUnits(2, 30, 2)
print "============================================================"
print "Disconnect parties (3 every time), and see that the resources are freed (8 resources per 3 parties). At the all resources are freed" 
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
  if (x+1)%3 == 0 :
    sleep(1) 
    c.TestResourcesReportPortsType("video",160-8*(x+1)/3,"OCCUPIED")
    c.TestResourcesReportPortsType("video",8*(x+1)/3,"FREE")
print "============================================================"
print "Check configuration with 160 CIFs (this will require reconfiguration)" 
print "-----------------------------------------------------------"
profIdCIF = c.AddProfileWithRate("ProfileCIF",128,"motion")
num_of_parties = 80
confid1 = c.CreateConfFromProfileWithVideoParties("ConfCIF1",profIdCIF,num_of_parties)
confid2 = c.CreateConfFromProfileWithVideoParties("ConfCIF2",profIdCIF,num_of_parties)
c.WaitAllOngoingConnected(confid1)
c.WaitAllOngoingConnected(confid2)
c.TestResourcesReportPortsType("video",160,"TOTAL")
c.TestResourcesReportPortsType("video",160,"OCCUPIED")
c.TestResourcesReportPortsType("video",0,"FREE")
c.CheckAudioVideoUnits(6, 26, 1)
#c.CheckAudioVideoUnits(5, 27, 2) sometimes its 5 sometimes its 6....  
print "============================================================"
print "Changing the configuration to all audio (800) - still in auto mode" 
print "-----------------------------------------------------------"
c.SetAutoPortConfiguration(80) 
if c.IsThereAnActiveAlarmOfAllocationModeChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Allocation Mode Change")  
if c.IsThereAnActiveAlarmOfPortConfigurationChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Port Configuration Change")  
c.CheckModes("auto","auto")
c.CheckAudioVideoAutoConfiguration(800, 0)
c.TestResourcesReportPortsType("video",0,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",0,"FREE")
c.TestResourcesReportPortsType("audio",800,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",800,"FREE")
c.CheckAudioVideoUnits(32, 0, 1)
c.CheckAudioVideoUnits(32, 0, 2)
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
print "Changing the configuration to fixed mode" 
print "-----------------------------------------------------------"  
c.SetMode("fixed")
c.CheckModes("fixed","fixed")
if c.IsThereAnActiveAlarmOfAllocationModeChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Allocation Mode Change")
print "There's NO active alarm of Allocation Mode Change - as expected" 	  
print "-----------------------------------------------------------"  

c.Disconnect()

