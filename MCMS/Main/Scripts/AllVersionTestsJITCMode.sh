#!/bin/sh

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"


TESTS="Scripts/PSTNConf.py \
       Scripts/PSTN_Move.py \
       Scripts/AutoRealVoip.py \
       Scripts/AutoRealVideo.py \
       Scripts/SipAutoRealVoip.py \
       Scripts/SipAutoRealVideo.py \
       Scripts/MixAutoRealVideo.py \
       Scripts/UndefinedDialIn.py \
       Scripts/Add20ConferenceNew.py \
       Scripts/VideoConfWith40Participants.py \
       Scripts/TestMIBFile.py \
       Scripts/FirstLogin.py \
       Scripts/FedPassword.py \
       Scripts/FedPasswordAging.py "

for test in $TESTS
do
  Scripts/Header.sh $test
  $test || exit 101
done





