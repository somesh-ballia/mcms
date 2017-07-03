#!/mcms/python/bin/python

#############################################################################
#Script which tests reservation feature with minimum participants for Auto Mode!
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpm.txt"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_80Video.xml"

import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

t = datetime.utcnow( ) 
deltat = timedelta(0,0,0,0,30,1,0)
t = t + deltat

t2 = datetime.utcnow( ) 
deltat = timedelta(1,0,0,0,0,0,0)
t2 = t + deltat

t3 = datetime.utcnow( ) 
deltat = timedelta(2,0,0,0,0,0,0)
t3 = t + deltat

profId = c.AddProfileWithRate("ProfRate64",64)
print "-----------------------------------------------------------"
print "Check startup as expected - auto mode with 80 video"
c.CheckAudioVideoAutoConfiguration(0, 80)
print "-----------------------------------------------------------"
print "Trying to add a reservation with 1 audio. This should fail"
c.CreateRes("test1", profId, t, "", 1, 0, "Insufficient resources")
print "Trying to add a reservation with both 10 audio and 10 video. This should fail too"
c.CreateRes("test1", profId, t, "", 10, 10, "Insufficient resources")
print "-----------------------------------------------------------"
print "Adding a reservation with 30 video."
res_id1 = c.CreateRes("test1", profId, t, "", 0, 30)
print "Adding a second reservation with 30 video (at the same time)."
res_id2 = c.CreateRes("test2", profId, t, "", 0, 30)
print "Trying to add a third reservation with 30 video (at the same time). This should fail"
c.CreateRes("test3", profId, t, "", 0, 30, "Insufficient resources")
print "Adding again third reservation. This time with 20 video. This should succeed"
res_id3 = c.CreateRes("test3", profId, t, "", 0, 20)
print "-----------------------------------------------------------"
print "Adding a reservation with 80 video at a different time."
res_id=c.CreateRes("test2_1", profId, t2, "", 0, 80)
print "Adding a second reservation with 10 video (at the same time). This should fail"
c.CreateRes("test2_2", profId, t2, "", 0, 30, "Insufficient resources")
print "Delete the first reservation"
c.DelConfReservation(res_id)
print "Now try the second one again"
res_id_2_2=c.CreateRes("test2_2", profId, t2, "", 0, 30)
print "-----------------------------------------------------------"  
print "Adding another reservation with 80 video at another different time."
res_id_3_1=c.CreateRes("test3_1", profId, t3, "", 0, 80)
print "-----------------------------------------------------------"
print "Changing the configuration to all audio (400) - still in auto mode" 
c.SetAutoPortConfiguration(40,"System should be reset") 
c.CheckAudioVideoAutoConfiguration(400, 0)
c.Disconnect()
print "Resetting MCU in order for new configuration to take effect"
os.environ["CLEAN_CFG"]="NO"
os.environ["RESOURCE_SETTING_FILE"]=""
os.system("Scripts/Destroy.sh")
os.system("Scripts/Startup.sh")
c.Connect()
c.CheckAudioVideoAutoConfiguration(400, 0)
print "-----------------------------------------------------------"  
print "Check that now all reservation suffer from resource deficiency"
c.SendXmlFile('Scripts/AddRemoveReservation/TransResList.xml',"Status OK") 
res_status_list = c.xmlResponse.getElementsByTagName("RES_STATUS")
if(len(res_status_list) != 5):
	sys.exit("Incorrect number of reservations found in list " + str(len(res_status_list)))
print "res_status_list: " + str(len(res_status_list))
for index in range(len(res_status_list)):
    if(res_status_list[index].firstChild.data  != "suspended"):
    	sys.exit("Incorrect status was found for reservation: " + res_status_list[index].firstChild.data)
print "Indeed all reservation suffer now from resource deficiency"
print "-----------------------------------------------------------" 
print "Trying to add five reservations, each one with 80 audio. All at the same time as the first bunch of reservations (that are now faulty)"
print "The first reservation that we will add is with 200 audio participants. This will be automaticly changed by MCMS to 80"
res_Id_audio1 = c.CreateRes("audio_only_test1", profId, t, "", 200, 0, "Reserved resources for audio participants exceeds the maximum number of audio participants per conference. Additional connections of participant are denied.",1)
c.LoadXmlFile('Scripts/AddRemoveReservation/GetRes.xml')
c.ModifyXml("GET_RES","ID",res_Id_audio1)
c.Send()
received_min_audio = c.GetTextUnder("MEET_ME_PER_CONF","MIN_NUM_OF_AUDIO_PARTIES")
if(received_min_audio != "80"):
  	sys.exit("Incorrect number of min parties found in reservation. Actual number: " + received_min_audio)
print "Indeed the minimum audio parties was lowered to 80"    	
res_Id_audio2 = c.CreateRes("audio_only_test2", profId, t, "", 80, 0)
res_Id_audio3 = c.CreateRes("audio_only_test3", profId, t, "", 80, 0)
res_Id_audio4 = c.CreateRes("audio_only_test4", profId, t, "", 80, 0)
res_Id_audio5 = c.CreateRes("audio_only_test5", profId, t, "", 80, 0)
print "-----------------------------------------------------------"  
print "Trying to add an additional reservation with 1 audio only participant. And another reservation with the same name. Both should fail"
c.CreateRes("audio_only_test6", profId, t, "", 1, 0, "Insufficient resources")
c.CreateRes("audio_only_test1", profId, t3, "", 1, 0, "Conference name already exists")
print "-----------------------------------------------------------"  
print "Update reservation from the second bunch of reservation, change from 30 min video to 10 min audio. It should stop being suspended"
c.UpdateReservation("test2_2",res_id_2_2,"",profId,t2, 10, 0)
print "Succeeded updating"
c.CheckResStatus(res_id_2_2,"ok")
print "-----------------------------------------------------------"  
print "Changing the configuration to 200 audio - 40 video - still in auto mode" 
c.SetAutoPortConfiguration(20,"System should be reset") 
c.CheckAudioVideoAutoConfiguration(200, 40)
c.Disconnect()
print "Resetting MCU in order for new configuration to take effect"
os.system("Scripts/Destroy.sh")
os.system("Scripts/Startup.sh")
c.Connect()
c.CheckAudioVideoAutoConfiguration(200, 40)
print "-----------------------------------------------------------"  
print "Checking the status of each reservation according to this configuration"
c.CheckResStatus(res_id1,"ok")
c.CheckResStatus(res_id2,"suspended")
c.CheckResStatus(res_id3,"suspended")
c.CheckResStatus(res_Id_audio1,"ok")
c.CheckResStatus(res_Id_audio2,"ok")
c.CheckResStatus(res_Id_audio3,"suspended")
c.CheckResStatus(res_Id_audio4,"suspended")
c.CheckResStatus(res_Id_audio5,"suspended")
c.CheckResStatus(res_id_2_2,"ok")
c.CheckResStatus(res_id_3_1,"suspended")
print "-----------------------------------------------------------"  
print "Updating test1 to not take resources at all. as a result test2 should be changed to status OK"
c.UpdateReservation("test1",res_id1,"",profId,t, 0, 0)
c.CheckResStatus(res_id1,"ok")
c.CheckResStatus(res_id2,"ok")
c.CheckResStatus(res_id3,"suspended")
print "-----------------------------------------------------------"  
print "Updating test2 to take only 20 video parties (instead of 30). as a result test3 should be changed to status OK"
c.UpdateReservation("test2",res_id2,"",profId,t, 0, 20)
c.CheckResStatus(res_id1,"ok")
c.CheckResStatus(res_id2,"ok")
c.CheckResStatus(res_id3,"ok")
print "-----------------------------------------------------------"  
print "Removing audio_only_test1, as a result audio_only_test3 should be changed to status OK"
c.DelConfReservation(res_Id_audio1)
c.CheckResStatus(res_Id_audio2,"ok")
c.CheckResStatus(res_Id_audio3,"ok")
c.CheckResStatus(res_Id_audio4,"suspended")
c.CheckResStatus(res_Id_audio5,"suspended")
print "-----------------------------------------------------------"  
print "Updating test3_1 to take only 40 video parties (instead of 80). as a result test3_1 should be changed to status OK (first try 50, this should fail)"
c.UpdateReservation("test3_1",res_id_3_1,"",profId,t3, 0, 50, "Insufficient resources")
c.UpdateReservation("test3_1",res_id_3_1,"",profId,t3, 0, 40)
c.CheckResStatus(res_id_3_1,"ok")
print "-----------------------------------------------------------"  
c.Disconnect()

