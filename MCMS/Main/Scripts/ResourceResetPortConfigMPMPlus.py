#!/mcms/python/bin/python

#############################################################################
#Script which tests reset port configuration feature, in MPM plus!!!!
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-LONG_SCRIPT_TYPE


#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_160Video.xml" 
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"

import ResourceResetPortConfig

ResourceResetPortConfig.TestResetPort(80,160,"VersionCfg/MPL_SIM_BARAK_ONE_CARD.XML","VersionCfg/MPL_SIM_BARAK.XML",9)



