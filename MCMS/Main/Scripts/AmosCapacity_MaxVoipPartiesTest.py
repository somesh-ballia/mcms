#!/mcms/python/bin/python

#*Script_Info_Name="AmosCapacity_MaxVoipPartiesTest.py"
#*Script_Info_Group="ConfParty"
#*Script_Info_Programmer="Ron"
#*Script_Info_Version="V4.2 (Amos)"
#*Script_Info_Description="Max voip participants and confrences in Amos 4 cards system (1600 parties in 800 confs)"


# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_Amos_1600_voip.cfs"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_1600AUDIO_Amos_Voice.xml"

#-LONG_SCRIPT_TYPE


from AmosCapacity_functions import *

## tests to run


max_EQs_test = False
max_sip_factories_test = False
max_profiles_test = False
max_MRs_test = False
max_confrences_test = False
max_dial_out_voip_parties = True
max_dial_out_voip_parties_per_conf = False
max_dial_out_video_parties_per_conf = False

if max_EQs_test==True:
    max_EQs = 80
    c = McmsConnection()
    c.Connect()
    MaxEntryQueuesTest(c,max_EQs)
    c.Disconnect()
    
if max_sip_factories_test==True:
    max_sip_factories = 80
    c = McmsConnection()
    c.Connect()
    MaxSipFactoriesTest(c,max_sip_factories)
    c.Disconnect()

if max_profiles_test==True:
    max_profiles = 80
    c = McmsConnection()
    c.Connect()
    MaxProfilesTest(c,max_profiles)
    c.Disconnect()
 
if max_MRs_test==True:
    max_MRs = 2000
    c = McmsConnection()
    c.Connect()
    MaxMeetingRoomsTest(c,max_MRs)
    c.Disconnect()

if max_confrences_test == True:
    max_confrences = 800
    c = McmsConnection()
    c.Connect()
    MaxConfrencesTest(c,max_confrences)
    c.Disconnect()

if max_dial_out_voip_parties == True:
    c = McmsConnection()
    c.Connect()
    ## configuration
    confrences = 800
    ## participants per conf by type
    h323_voip_out = 2
    h323_voip_in = 0
    h323_video_out = 0
    h323_video_in = 0

    MaxMixedPartiesTest(c,confrences,h323_voip_out,h323_voip_in,h323_video_out,h323_video_in)

    c.Disconnect()
  
if max_dial_out_voip_parties_per_conf == True:
    c = McmsConnection()
    c.Connect()
    ## configuration
    confrences = 1
    ## participants per conf by type
    h323_voip_out = 200
    h323_voip_in = 0
    h323_video_out = 0
    h323_video_in = 0

    MaxPartiesInConfTest(c,'Scripts/AddVoipConf.xml','Scripts/AddVoipParty1.xml',800,60,"FALSE")
    ##MaxMixedPartiesTest(c,confrences,h323_voip_out,h323_voip_in,h323_video_out,h323_video_in)
    
    c.Disconnect()

if max_dial_out_video_parties_per_conf == True:
    c = McmsConnection()
    c.Connect()
    ## configuration
    confrences = 8
    ## participants per conf by type
    h323_voip_out = 100
    h323_voip_in = 0
    h323_video_out = 0
    h323_video_in = 0

    MaxMixedPartiesTest(c,confrences,h323_voip_out,h323_voip_in,h323_video_out,h323_video_in)
    
    c.Disconnect()


