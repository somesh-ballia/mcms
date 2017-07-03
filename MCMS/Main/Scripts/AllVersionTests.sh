#!/bin/sh

#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpm.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_80_YES_PstnYES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Settings.xml" 

TESTS="Scripts/PSTNConf.py \
       Scripts/ISDN_DialInDialOut.py \
       Scripts/PSTN_Move.py \
       Scripts/AutoRealVoip.py \
       Scripts/AutoRealVideo.py \
       Scripts/SipAutoRealVoip.py \
       Scripts/SipAutoRealVideo.py \
       Scripts/MixAutoRealVideo.py \
       Scripts/UndefinedDialIn.py \
       Scripts/Add20ConferenceNew.py \
       Scripts/AutoRealVideoV6.py \
       Scripts/AutoRealVideoDialInMixedV4V6.py \
       Scripts/VideoConfWith40Participants.py \
       Scripts/TestMIBFile.py"

for test in $TESTS
do
  Scripts/Header.sh $test
  $test || exit 101
done



