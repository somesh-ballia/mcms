#!/mcms/python/bin/python

#############################################################################
#Script which tests reservation feature with different types of participants (on conference level)
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_160CIF.xml"

import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

t = datetime.utcnow( )
deltat = timedelta(0,0,0,0,0,2,0)
print "-----------------------------------------------------------"
print "Check startup as expected - fixed mode with 160 video"
c.SetMode("fixed")
c.CheckModes("fixed","fixed")
c.CheckEnhancedConfiguration(0, 160, 0, 0, 0)
print "-----------------------------------------------------------"
print "Initiating different kinds of profiles, that we will use later"
print "Two profiles that should be CIF"
profId128_Motion = c.AddProfileWithRate("Profile128_motion",128,"motion")
profId128_Sharpness = c.AddProfileWithRate("Profile128_sharpness",128,"sharpness")
print "Two profiles that should be SD"
profId512_Motion = c.AddProfileWithRate("Profile512_motion",512,"motion")
profId768_Sharpness = c.AddProfileWithRate("Profile768_sharpness",768,"sharpness")
print "Two profiles that should be HD720"
profId1024_Motion = c.AddProfileWithRate("Profile1024_motion",1024,"motion")
profId1536_Sharpness = c.AddProfileWithRate("Profile1536_sharpness",1536,"sharpness")
print "Two profiles that should be HD1080"
profId1920_Motion = c.AddProfileWithRate("Profile1920_motion",1920,"motion")
profId4M_Sharpness = c.AddProfileWithRate("Profile4M_sharpness",4096,"sharpness")
print "-----------------------------------------------------------"
print "Initiating two reservation, one with each of the CIF profiles"
t = t + deltat
res_id128_Motion = c.CreateRes("128_Motion", profId128_Motion, t, "", 0, 80)
res_id128_Sharpness = c.CreateRes("128_Sharpness", profId128_Sharpness, t, "", 0, 80)
print "Trying to add an additional one, this should fail"
res_idDummy = c.CreateRes("Dummy", profId128_Sharpness, t, "", 0, 1, "Insufficient resources")
print "-----------------------------------------------------------"
print "Changing configuration to all audio"
c.SetEnhancedConfiguration(800,0,0,0,0) 
c.CheckEnhancedConfiguration(800,0,0,0,0)
c.CheckResStatus(res_id128_Motion,"suspended")
c.CheckResStatus(res_id128_Sharpness,"suspended")
print "-----------------------------------------------------------"
print "Initiate 4 reservations of 200 audio only participants"
t = t + deltat
res_id_audio1 = c.CreateRes("audio1", profId128_Motion, t, "", 200, 0)
res_id_audio2 = c.CreateRes("audio2", profId128_Motion, t, "", 200, 0)
res_id_audio3 = c.CreateRes("audio3", profId128_Motion, t, "", 200, 0)
res_id_audio4 = c.CreateRes("audio4", profId128_Motion, t, "", 200, 0)
print "Trying to add an additional one, this should fail"
res_idDummy = c.CreateRes("Dummy", profId128_Motion, t, "", 1, 0, "Insufficient resources")
print "-----------------------------------------------------------"
#sleep a while because else we will get the status of busy setting the last configuration 
sleep(5)
print "Changing configuration to all SD"
c.SetEnhancedConfiguration(0,0,60,0,0) 
c.CheckEnhancedConfiguration(0,0,60,0,0)
c.CheckResStatus(res_id_audio1,"suspended")
c.CheckResStatus(res_id_audio2,"suspended")
c.CheckResStatus(res_id_audio3,"suspended")
c.CheckResStatus(res_id_audio4,"suspended")
c.CheckResStatus(res_id128_Motion,"suspended")
c.CheckResStatus(res_id128_Sharpness,"suspended")
print "-----------------------------------------------------------"
print "Initiating two reservation, one with each of the SD profiles"
t = t + deltat
res_id512_Motion = c.CreateRes("512_Motion", profId512_Motion, t, "", 0, 30)
res_id768_Sharpness = c.CreateRes("768_Sharpness", profId768_Sharpness, t, "", 0, 30)
print "Trying to add an additional one, this should fail"
res_idDummy = c.CreateRes("Dummy", profId768_Sharpness, t, "", 0, 1, "Insufficient resources")
print "-----------------------------------------------------------"
#sleep a while because else we will get the status of busy setting the last configuration 
sleep(5)
print "Changing configuration to all HD720"
c.SetEnhancedConfiguration(0,0,0,40,0) 
c.CheckEnhancedConfiguration(0,0,0,40,0)
c.CheckResStatus(res_id_audio1,"suspended")
c.CheckResStatus(res_id_audio2,"suspended")
c.CheckResStatus(res_id_audio3,"suspended")
c.CheckResStatus(res_id_audio4,"suspended")
c.CheckResStatus(res_id128_Motion,"suspended")
c.CheckResStatus(res_id128_Sharpness,"suspended")
c.CheckResStatus(res_id512_Motion,"ok")
c.CheckResStatus(res_id768_Sharpness,"suspended")
print "-----------------------------------------------------------"
print "Initiating two reservation, one with each of the HD720 profiles"
t = t + deltat
res_id1024_Motion = c.CreateRes("1024_Motion", profId1024_Motion, t, "", 0, 20)
res_id1536_Sharpness = c.CreateRes("1536_Sharpness", profId1536_Sharpness, t, "", 0, 20)
print "Trying to add an additional one, this should fail"
res_idDummy = c.CreateRes("Dummy", profId1536_Sharpness, t, "", 0, 1, "Insufficient resources")
print "-----------------------------------------------------------"
#sleep a while because else we will get the status of busy setting the last configuration 
sleep(5)
print "Changing configuration to all HD1080"
c.SetEnhancedConfiguration(0,0,0,0,20) 
c.CheckEnhancedConfiguration(0,0,0,0,20)
c.CheckResStatus(res_id_audio1,"suspended")
c.CheckResStatus(res_id_audio2,"suspended")
c.CheckResStatus(res_id_audio3,"suspended")
c.CheckResStatus(res_id_audio4,"suspended")
c.CheckResStatus(res_id128_Motion,"suspended")
c.CheckResStatus(res_id128_Sharpness,"suspended")
c.CheckResStatus(res_id512_Motion,"suspended")
c.CheckResStatus(res_id768_Sharpness,"suspended")
c.CheckResStatus(res_id1024_Motion,"ok")
c.CheckResStatus(res_id1536_Sharpness,"suspended")
print "-----------------------------------------------------------"
print "Initiating two reservation, one with each of the HD1080 profiles"
t = t + deltat
res_id1920_Motion = c.CreateRes("1920_Motion", profId1920_Motion, t, "", 0, 10)
res_id4M_Sharpness = c.CreateRes("4M_Sharpness", profId4M_Sharpness, t, "", 0, 10)
print "Trying to add an additional one, this should fail"
res_idDummy = c.CreateRes("Dummy", profId4M_Sharpness, t, "", 0, 1, "Insufficient resources")
print "-----------------------------------------------------------"
#sleep a while because else we will get the status of busy setting the last configuration 
sleep(5)
print "Changing configuration back to all CIFs"
c.SetEnhancedConfiguration(0, 160, 0, 0, 0) 
c.CheckEnhancedConfiguration(0, 160, 0, 0, 0)
c.CheckResStatus(res_id_audio1,"suspended")
c.CheckResStatus(res_id_audio2,"suspended")
c.CheckResStatus(res_id_audio3,"suspended")
c.CheckResStatus(res_id_audio4,"suspended")
c.CheckResStatus(res_id128_Motion,"ok")
c.CheckResStatus(res_id128_Sharpness,"ok")
c.CheckResStatus(res_id512_Motion,"suspended")
c.CheckResStatus(res_id768_Sharpness,"suspended")
c.CheckResStatus(res_id1024_Motion,"suspended")
c.CheckResStatus(res_id1536_Sharpness,"suspended")
c.CheckResStatus(res_id1920_Motion,"suspended")
c.CheckResStatus(res_id4M_Sharpness,"suspended")
print "-----------------------------------------------------------"
print "Resetting MCU, and see that during startup we receive the appropriate profile table."
print "This will be done by changing the configuration and seeing that we indeed got the expected statuses for the different reservations"
sleep(10)
c.Disconnect()
os.environ["CLEAN_CFG"]="NO"
os.environ["RESOURCE_SETTING_FILE"]=""
os.system("Scripts/Destroy.sh")
os.system("Scripts/Startup.sh")
c.Connect()
print "-----------------------------------------------------------"
print "Checking res statuses"
c.SetMode("fixed")
#c.SetEnhancedConfiguration(0, 160, 0, 0, 0) 
#sleep(50)
#c.CheckEnhancedConfiguration(0, 160, 0, 0, 0)
c.CheckResStatus(res_id_audio1,"suspended")
c.CheckResStatus(res_id_audio2,"suspended")
c.CheckResStatus(res_id_audio3,"suspended")
c.CheckResStatus(res_id_audio4,"suspended")
c.CheckResStatus(res_id128_Motion,"ok")
c.CheckResStatus(res_id128_Sharpness,"ok")
c.CheckResStatus(res_id512_Motion,"suspended")
c.CheckResStatus(res_id768_Sharpness,"suspended")
c.CheckResStatus(res_id1024_Motion,"suspended")
c.CheckResStatus(res_id1536_Sharpness,"suspended")
c.CheckResStatus(res_id1920_Motion,"suspended")
c.CheckResStatus(res_id4M_Sharpness,"suspended")
print "-----------------------------------------------------------"
#sleep a while because else we will get the status of busy setting the last configuration 
sleep(5)
print "Changing configuration to all audio"
c.SetEnhancedConfiguration(800,0,0,0,0) 
c.CheckEnhancedConfiguration(800,0,0,0,0)
c.CheckResStatus(res_id_audio1,"ok")
c.CheckResStatus(res_id_audio2,"ok")
c.CheckResStatus(res_id_audio3,"ok")
c.CheckResStatus(res_id_audio4,"ok")
c.CheckResStatus(res_id128_Motion,"suspended")
c.CheckResStatus(res_id128_Sharpness,"suspended")
c.CheckResStatus(res_id512_Motion,"suspended")
c.CheckResStatus(res_id768_Sharpness,"suspended")
c.CheckResStatus(res_id1024_Motion,"suspended")
c.CheckResStatus(res_id1536_Sharpness,"suspended")
c.CheckResStatus(res_id1920_Motion,"suspended")
c.CheckResStatus(res_id4M_Sharpness,"suspended")
print "-----------------------------------------------------------"
#sleep a while because else we will get the status of busy setting the last configuration 
sleep(5)
print "Changing configuration to all HD1080"
c.SetEnhancedConfiguration(0,0,0,0,20) 
c.CheckEnhancedConfiguration(0,0,0,0,20)
c.CheckResStatus(res_id_audio1,"suspended")
c.CheckResStatus(res_id_audio2,"suspended")
c.CheckResStatus(res_id_audio3,"suspended")
c.CheckResStatus(res_id_audio4,"suspended")
c.CheckResStatus(res_id128_Motion,"suspended")
c.CheckResStatus(res_id128_Sharpness,"suspended")
c.CheckResStatus(res_id512_Motion,"suspended")
c.CheckResStatus(res_id768_Sharpness,"suspended")
c.CheckResStatus(res_id1024_Motion,"ok")
c.CheckResStatus(res_id1536_Sharpness,"suspended")
c.CheckResStatus(res_id1920_Motion,"ok")
c.CheckResStatus(res_id4M_Sharpness,"ok")
print "-----------------------------------------------------------"
#sleep a while because else we will get the status of busy setting the last configuration 
sleep(5)
print "Changing configuration to all HD720"
c.SetEnhancedConfiguration(0,0,0,40,0) 
c.CheckEnhancedConfiguration(0,0,0,40,0)
c.CheckResStatus(res_id_audio1,"suspended")
c.CheckResStatus(res_id_audio2,"suspended")
c.CheckResStatus(res_id_audio3,"suspended")
c.CheckResStatus(res_id_audio4,"suspended")
c.CheckResStatus(res_id128_Motion,"suspended")
c.CheckResStatus(res_id128_Sharpness,"suspended")
c.CheckResStatus(res_id512_Motion,"ok")
c.CheckResStatus(res_id768_Sharpness,"suspended")
c.CheckResStatus(res_id1024_Motion,"ok")
c.CheckResStatus(res_id1536_Sharpness,"ok")
c.CheckResStatus(res_id1920_Motion,"suspended")
c.CheckResStatus(res_id4M_Sharpness,"suspended")
print "-----------------------------------------------------------"
#sleep a while because else we will get the status of busy setting the last configuration 
sleep(5)
print "Changing configuration to all SD"
c.SetEnhancedConfiguration(0,0,60,0,0) 
c.CheckEnhancedConfiguration(0,0,60,0,0)
c.CheckResStatus(res_id_audio1,"suspended")
c.CheckResStatus(res_id_audio2,"suspended")
c.CheckResStatus(res_id_audio3,"suspended")
c.CheckResStatus(res_id_audio4,"suspended")
c.CheckResStatus(res_id128_Motion,"suspended")
c.CheckResStatus(res_id128_Sharpness,"suspended")
c.CheckResStatus(res_id512_Motion,"ok")
c.CheckResStatus(res_id768_Sharpness,"ok")
c.CheckResStatus(res_id1024_Motion,"suspended")
c.CheckResStatus(res_id1536_Sharpness,"suspended")
c.CheckResStatus(res_id1920_Motion,"suspended")
c.CheckResStatus(res_id4M_Sharpness,"suspended")
print "-----------------------------------------------------------"
print "Checking delete profile - that the RP is updated - Trying to delete one of the profiles, this will fail, because it's in use!"
c.LoadXmlFile("Scripts/RemoveNewProfile.xml")
c.ModifyXml("TERMINATE_PROFILE","ID",profId128_Sharpness)
c.Send("Profile in use and cannot be deleted")
print "-----------------------------------------------------------"
print "Checking update profile - that the RP is updated"
print "Updating the two hd720 profiles, so they will be SD profiles. As a result the two hd720 reservations, will have enough resources"  
c.UpdateProfile("Profile1024_motion",profId1024_Motion,512,"motion")
c.UpdateProfile("Profile1536_sharpness", profId1536_Sharpness,512,"motion")
sleep(3)
c.CheckResStatus(res_id1024_Motion,"ok")
c.CheckResStatus(res_id1536_Sharpness,"ok")
print "Updating the profiles back to what they were, and check that the two reservations are again suspended"
c.UpdateProfile("Profile1024_motion",profId1024_Motion,1024,"motion")
c.UpdateProfile("Profile1536_sharpness", profId1536_Sharpness,1536,"sharpness")
sleep(3)
c.CheckResStatus(res_id1024_Motion,"suspended")
c.CheckResStatus(res_id1536_Sharpness,"suspended")
print "-----------------------------------------------------------"
print "Change to auto mode"
c.SetMode("auto")
c.CheckModes("auto","auto")
print "-----------------------------------------------------------"
print "All reservations should now be OK, except for the reservations with audio participants"
c.CheckResStatus(res_id_audio1,"suspended")
c.CheckResStatus(res_id_audio2,"suspended")
c.CheckResStatus(res_id_audio3,"suspended")
c.CheckResStatus(res_id_audio4,"suspended")
c.CheckResStatus(res_id128_Motion,"ok")
c.CheckResStatus(res_id128_Sharpness,"suspended")
c.CheckResStatus(res_id512_Motion,"ok")
c.CheckResStatus(res_id768_Sharpness,"suspended")
c.CheckResStatus(res_id1024_Motion,"ok")
c.CheckResStatus(res_id1536_Sharpness,"suspended")
c.CheckResStatus(res_id1920_Motion,"ok")
c.CheckResStatus(res_id4M_Sharpness,"suspended")
print "-----------------------------------------------------------"


c.Disconnect()

