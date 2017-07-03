#!/mcms/python/bin/python

# m_LogicalHD720WeightOfPartyPerType: (AVC)
# +------------+----------+--------+
# | Type       | NonMixed | Mixed  |
# +------------+----------+--------+
# | eAudio     |   0.0833 | 0.0833 |
# | eCif       |   0.3333 |   0.75 |
# | eSD30      |      0.5 |   0.75 |
# | eHD720     |        1 |    1.5 |
# | eHD1080p30 |        2 |      3 |
# | eHD1080p60 |        3 |      5 |
# +------------+----------+--------+
# m_LogicalHD720WeightSVCpartyPerType: (SVC)
# +------------+----------+--------+
# | Type       | NonMixed | Mixed  |
# +------------+----------+--------+
# | eAudio     |   0.3333 | 0.3333 |
# | eCif       |   0.3333 | 0.3333 |
# | eSD30      |   0.3333 | 0.3333 |
# | eHD720     |   0.3333 | 0.3333 |
# | eHD1080p30 |   0.3333 | 0.3333 |
# | eHD1080p60 |   0.3333 | 0.3333 |
# +------------+----------+--------+

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE_4_CARDS.XML"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_120_YES_PstnYES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_360Video.xml"
   
import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

print "============================================================"
print "Checking startup and the expected configuration - it should be auto mode with 120-video 0-audio" 
print "============================================================"

if c.IsThereAnActiveAlarmOfAllocationModeChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Allocation Mode Change at the beginning of the test ")  
if c.IsThereAnActiveAlarmOfPortConfigurationChange() == 1 :
   	c.Disconnect()                
   	sys.exit("There's an active alarm of Port Configuration Change at the beginning of the test ")  
c.CheckModes("auto","auto")
c.CheckAudioVideoUnits(10, 30, 1)
c.CheckAudioVideoUnits(10, 30, 2)
c.CheckAudioVideoAutoConfiguration(0, 120)


c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",0,"OCCUPIED")
c.TestResourcesReportPortsType("video",360,"FREE")
c.TestResourcesReportPortsType("audio",0,"TOTAL")
c.TestResourcesReportPortsType("audio",0,"OCCUPIED")
c.TestResourcesReportPortsType("audio",0,"FREE")

profId_256 = c.AddProfileWithRate("Profile256",256)
profId_768 = c.AddProfileWithRate("Profile768",768)
profId_1024 = c.AddProfileWithRate("Profile1024",1024)
profId_1536 = c.AddProfileWithRate("Profile1536",1536)

profId_256_motion = c.AddProfileWithRate("Profile256_motion",256,"motion")
profId_768_motion = c.AddProfileWithRate("Profile768_motion",768, "motion")
profId_1024_motion = c.AddProfileWithRate("Profile1024_motion",1024, "motion")
profId_1536_motion = c.AddProfileWithRate("Profile1536_motion",1536, "motion")

print "============================================================"
print "Check the \"balanced\" slider settings"
print "============================================================"

num_of_parties = 2
sleep_seconds = 1

if (c.IsProcessUnderValgrind("ConfParty")):
    sleep_seconds = 5

confname = "Conf_256"
partyNamePrefix = "VideoParty1_"
capName = "H264(hd1080)+ALL"
confid = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_256, partyNamePrefix, num_of_parties, capName)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",4,"OCCUPIED")
c.TestResourcesReportPortsType("video",357,"FREE")

confname = "Conf_1024"
partyNamePrefix = "VideoParty2_"
capName = "H264(hd1080)+ALL"
confid_1024 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_1024, partyNamePrefix, num_of_parties, capName)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",10,"OCCUPIED")
c.TestResourcesReportPortsType("video",351,"FREE")

c.DeleteConf(confid)
c.DeleteConf(confid_1024)
c.WaitAllConfEnd()

print "Test balance for conf of motion type "

confname = "Conf_256_motion"
partyNamePrefix = "MVideoParty1_"
capName = "H264(hd1080)+ALL"
confid = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_256_motion, partyNamePrefix, num_of_parties, capName)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",2,"OCCUPIED")
c.TestResourcesReportPortsType("video",358,"FREE")

confname = "Conf_1024_motion"
partyNamePrefix = "MVideoParty2_"
capName = "H264(hd1080)+ALL"
confid_1024 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_1024_motion, partyNamePrefix, num_of_parties, capName)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",9,"OCCUPIED")
c.TestResourcesReportPortsType("video",352,"FREE")

c.DeleteConf(confid)
c.DeleteConf(confid_1024)
c.WaitAllConfEnd()

print "============================================================"
print "Change slider to \"resource_optimized\" settings"
print "============================================================"

c.SetResolutionSlider("hd1080p60", "resource_optimized")

confname = "Conf_768"
partyNamePrefix = "VideoParty3_"
capName = "H264(hd1080)+ALL"
confid_768 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_768, partyNamePrefix, num_of_parties, capName)

confname = "Conf_1024"
partyNamePrefix = "VideoParty4_"
confid_1024 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_1024, partyNamePrefix, num_of_parties, capName)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",7,"OCCUPIED")
c.TestResourcesReportPortsType("video",354,"FREE")
c.DeleteConf(confid_768)
c.DeleteConf(confid_1024)
c.WaitAllConfEnd()

print "Test resource_optimized for conf type of motion"

c.SetResolutionSlider("hd1080p60", "resource_optimized")

confname = "Conf_768_motion"
partyNamePrefix = "MVideoParty3_"
capName = "H264(hd1080)+ALL"
confid_768 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_768_motion, partyNamePrefix, num_of_parties, capName)

confname = "Conf_1024_motion"
partyNamePrefix = "MVideoParty4_"
confid_1024 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_1024_motion, partyNamePrefix, num_of_parties, capName)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",10,"OCCUPIED")
c.TestResourcesReportPortsType("video",351,"FREE")
c.DeleteConf(confid_768)
c.DeleteConf(confid_1024)
c.WaitAllConfEnd()

print "Test resource_optimized for pary type with high profile"
c.SetResolutionSlider("hd1080p60", "resource_optimized")

confname = "Conf_768"
partyNamePrefix = "HVideoParty3_"
capName = "FULL CAPSET"
confid_768 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_768, partyNamePrefix, num_of_parties, capName)

confname = "Conf_1024"
partyNamePrefix = "HVideoParty4_"
confid_1024 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_1024, partyNamePrefix, num_of_parties, capName)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",7,"OCCUPIED")
c.TestResourcesReportPortsType("video",354,"FREE")
c.DeleteConf(confid_768)
c.DeleteConf(confid_1024)
c.WaitAllConfEnd()


print "============================================================"
print "Change slider to \"user_exp_optimized\" settings"
print "============================================================"

c.SetResolutionSlider("hd1080p60", "user_exp_optimized")

confname = "Conf_768"
partyNamePrefix = "VideoParty5_"
capName = "H264(hd1080)+ALL"
confid_768 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_768, partyNamePrefix, num_of_parties, capName)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",4,"OCCUPIED")
c.TestResourcesReportPortsType("video",357,"FREE")

confname = "Conf_1536"
partyNamePrefix = "VideoParty6_"
confid_1536 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_1536, partyNamePrefix, num_of_parties, capName)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",10,"OCCUPIED")
c.TestResourcesReportPortsType("video",351,"FREE")
c.DeleteConf(confid_768)
c.DeleteConf(confid_1536)
c.WaitAllConfEnd()

print "Test user_exp_optimized for conf type of motion"

c.SetResolutionSlider("hd1080p60", "user_exp_optimized")

confname = "Conf_768_motion"
partyNamePrefix = "MVideoParty5_"
capName = "H264(hd1080)+ALL"
confid_768 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_768_motion, partyNamePrefix, num_of_parties, capName)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",7,"OCCUPIED")
c.TestResourcesReportPortsType("video",354,"FREE")

confname = "Conf_1536_motion"
partyNamePrefix = "MVideoParty6_"
confid_1536 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_1536_motion, partyNamePrefix, num_of_parties, capName)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",19,"OCCUPIED")
c.TestResourcesReportPortsType("video",342,"FREE")
c.DeleteConf(confid_768)
c.DeleteConf(confid_1536)
c.WaitAllConfEnd()

print "============================================================"
print "Change slider to \"manual\" settings where HD1080 set from 768k"
print "============================================================"

dic = { \
       "SET_SHARPNESS_RESOLUTIONS" : [256, 2048, 2048, 2048], \
       "SET_MOTION_RESOLUTIONS" : [384, 1024, 1920, 3584], \
       "SET_SHARPNESS_HIGH_PROFILE_RESOLUTIONS" : [256, 832, 1536, 1536], \
       "SET_MOTION_HIGH_PROFILE_RESOLUTIONS" : [256, 768, 1280, 2560]
       }
c.SetResolutionSlider("hd1080p60", "manual", dic)

# set with same setting again
c.SetResolutionSlider("hd1080p60", "manual", dic) 

confname = "Conf_768"
partyNamePrefix = "VideoParty7_"
capName = "H264(hd1080)+ALL"
confid_768 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_768, partyNamePrefix, num_of_parties, capName)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",4,"OCCUPIED")
c.TestResourcesReportPortsType("video",357,"FREE")

print "\n Adding participants with restricted max resolution: one=SD and other=HD720 "
num_of_parties_restricted = 3
for x in range(num_of_parties_restricted):
    partyip =  "1.2.3." + str(x+10)
    c.LoadXmlFile("Scripts/CheckMultiTypesOfCalls/AddDialOutH323Party.xml")
    c.ModifyXml("PARTY","IP", partyip)
    c.ModifyXml("ADD_PARTY","ID",confid_768)
    
    if(x == 1):
        partyname = "Party_HD720"
        c.ModifyXml("PARTY","MAX_RESOLUTION", "hd_720")
    else:
        partyname = "Party_SD"+str(x+1)
        c.ModifyXml("PARTY","MAX_RESOLUTION", "sd")   
    c.ModifyXml("PARTY","NAME", partyname)
    c.ModifyXml("ALIAS","NAME", partyname)
    c.Send()
    
c.WaitAllPartiesWereAdded(confid_768, num_of_parties+num_of_parties_restricted, 10)
c.WaitAllOngoingConnected(confid_768)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",10,"OCCUPIED")
c.TestResourcesReportPortsType("video",351,"FREE")

confName="Conf_768_sd"
print "\nAdding " + confName + " which conf max resolution field restricted to SD..."
c.LoadXmlFile('Scripts/AddCpConf.xml')
c.ModifyXml("RESERVATION","NAME",confName)
c.ModifyXml("RESERVATION","MAX_RESOLUTION", "sd")
c.Send()
confid_768sd = c.WaitConfCreated(confName)  
c.AddVideoParties(confid_768sd, num_of_parties)
c.WaitAllOngoingConnected(confid_768sd)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",13,"OCCUPIED")
c.TestResourcesReportPortsType("video",348,"FREE")

c.DeleteConf(confid_768)
c.DeleteConf(confid_768sd)
c.WaitAllConfEnd()


print "\n Test resolution slide with max resolution set to hd720p30..."
dic = { \
       "SET_SHARPNESS_RESOLUTIONS" : [256, 1024, 2048, 2048], \
       "SET_MOTION_RESOLUTIONS" : [384, 1024, 1920, 3584], \
       "SET_SHARPNESS_HIGH_PROFILE_RESOLUTIONS" : [256, 832, 1536, 1536], \
       "SET_MOTION_HIGH_PROFILE_RESOLUTIONS" : [256, 768, 1280, 2560]
       }
c.SetResolutionSlider("hd720p30", "manual", dic)

confname = "Conf_1536"
partyNamePrefix = "VideoParty8_"
capName = "FULL CAPSET"
confid_1536 = c.CreateConfFromProfileAndDialInVideoParties(confname, profId_1536, partyNamePrefix, num_of_parties, capName)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",7,"OCCUPIED")
c.TestResourcesReportPortsType("video",354,"FREE")

c.DeleteConf(confid_1536)
c.WaitAllConfEnd()

c.Disconnect()

