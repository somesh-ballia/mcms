#!/bin/sh

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_breeze.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_60HD_v100.0.cfs" # for 60HD720 (180CIF) ports in license

if [[ -f ./Scripts/FixMIBFile.py ]]; then
	echo "Applying MIB fix"
	./Scripts/FixMIBFile.py
fi

TESTS="Scripts/CallsAndScpTestForSvcOnlyConf.py \
       Scripts/AutoRealVoip.py \
       Scripts/AutoRealVideo.py \
       Scripts/SipAutoRealVoip.py \
       Scripts/SipAutoRealVideo.py \
       Scripts/MixAutoRealVideo.py \
       Scripts/Add20ConferenceNew.py \
       Scripts/TestMIBFile.py \
       Scripts/AwakeMrByUndef.py \
	   Scripts/EncryDialIn.py"


#       Scripts/VideoConfWith40Participants.py \


# The following script should be checked on Event Mode and be entered back to the list above. 
#	   Scripts/UndefinedDialIn.py \


# non-cop version tests:
#TESTS="Scripts/PSTNConf.py \
#       Scripts/PSTN_Move.py \
#       Scripts/AutoRealVoip.py \
#       Scripts/AutoRealVideo.py \
#       Scripts/SipAutoRealVoip.py \
#       Scripts/SipAutoRealVideo.py \
#       Scripts/MixAutoRealVideo.py \
#       Scripts/UndefinedDialIn.py \
#       Scripts/Add20ConferenceNew.py \
#       Scripts/VideoConfWith40Participants.py \
#       Scripts/TestMIBFile.py"

echo "***********************************"       
echo "******** Run Tests ****************"
echo "***********************************"       

for test in $TESTS
do
  Scripts/Header.sh $test
  $test
if [ "$?" != "0" ]
then 
   exit 101
fi
done

