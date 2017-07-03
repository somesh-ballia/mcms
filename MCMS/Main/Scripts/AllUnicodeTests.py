#!/bin/sh

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_breeze.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_60HD_v100.0.cfs" # for 60HD720 (180CIF) ports in license

TESTS="Scripts/UnicodeTests.py \
       Scripts/TestCDRDisplayName.py \
       Scripts/TestUnicodeCutUtf8Strings.py \
       Scripts/TestUnicodeUnknownCharsets.py \
       Scripts/CheckUnicodeDisplayName.py"


# Scripts/TestUnicodeExAsciiFile.py , should be last, it does reset"  , kobig remooved from test , irrelevant test hence unicode can be part of the name of the service


for test in $TESTS
do
  Scripts/Header.sh $test
  $test || exit 101
done


# LookupError: unknown encoding: ISO_8859, should work
#---------------------------
# Scripts/UnicodeTests.py \

# No such file or directory: 'Scripts/RemoveDirectory.xml'
#---------------------------
#Scripts/TestUnicodeApiAsciiValidation.py \


#*** SendXml - Expected: Status OK, got: Invalid Service name ***
#---------------------------
#Scripts/CheckUnicodeFields.py \
