#!/bin/sh

export MCMS=$PWD
export LAST_BUILD=`ls -l /Carmel-Versions/StableBuild/last | sed s/".*last -> "//`

cd /Carmel-Versions/StableBuild/$LAST_BUILD
./build.sh --MCMS=$MCMS --OUTPUT_FOLDER=$MCMS






