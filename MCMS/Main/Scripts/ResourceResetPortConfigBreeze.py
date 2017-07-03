#!/mcms/python/bin/python

#############################################################################
#Script which tests reset port configuration feature, in BREEZE!!!!
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-LONG_SCRIPT_TYPE

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_breeze.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_180_YES_PstnYES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_Default_Breeze.xml" 

import ResourceResetPortConfig

ResourceResetPortConfig.TestResetPort(90,180,"VersionCfg/MPL_SIM_BREEZE_ONE_CARD.XML","VersionCfg/MPL_SIM_BREEZE.XML",12)



