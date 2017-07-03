#!/bin/sh

#-LONG_SCRIPT_TYPE
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs"
#export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_180_YES_MultipleServices_YES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_144Video_80Audio.xml"


TESTS="Scripts/PSTNConf.py \
       Scripts/AutoRealVoip.py \
       Scripts/AutoRealVideo.py \
       Scripts/SipAutoRealVoip.py \
       Scripts/SipAutoRealVideo.py \
       Scripts/MixAutoRealVideo.py \
       Scripts/UndefinedDialIn.py \
       Scripts/Add20ConferenceNew.py \
       Scripts/TestMIBFile.py"

# PSTN_Move.py and Scripts/VideoConfWith40Participants.py fail
#       Scripts/VideoConfWith40Participants.py \
#       Scripts/PSTN_Move.py

for test in $TESTS
do
  Scripts/Header.sh $test
  $test
if [ "$?" != "0" ]
then 
   exit 101
fi
done
