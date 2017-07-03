#!/mcms/python/bin/python

#*Script_Info_Name="AmosCapacity_MaEqMrTest.py"
#*Script_Info_Group="ConfParty"
#*Script_Info_Programmer="Ron"
#*Script_Info_Version="V5_00"
#*Script_Info_Description="Max EQ, Sip Factories, Profiles and MR in Amos 4 cards system"


# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_Amos_1600_voip.cfs"
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK_4.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_80HD720_Amos.xml"


from AmosCapacity_functions import *

## tests to run


max_EQs_test = True
max_sip_factories_test = True
max_profiles_test = True
max_MRs_test = True

if max_EQs_test==True:
    max_EQs = 80
    c = McmsConnection()
    c.Connect()
    MaxEntryQueuesTest(c,max_EQs,60,"FALSE")
    c.Disconnect()
    
if max_sip_factories_test==True:
    max_sip_factories = 80
    c = McmsConnection()
    c.Connect()
    MaxSipFactoriesTest(c,max_sip_factories,60,"FALSE")
    c.Disconnect()

if max_profiles_test==True:
    max_profiles = 80
    c = McmsConnection()
    c.Connect()
    MaxProfilesTest(c,max_profiles,60,"FALSE")
    c.Disconnect()
 
if max_MRs_test==True:
    max_MRs = 2000
    c = McmsConnection()
    c.Connect()
    MaxMeetingRoomsTest(c,max_MRs,60,"FALSE")
    c.Disconnect()

