#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_18
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK_ONE_CARD.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_160Video.xml"
#-- SKIP_ASSERTS

import os
import ResourceHotSwapAA

ResourceHotSwapAA.CheckHotswapAA()