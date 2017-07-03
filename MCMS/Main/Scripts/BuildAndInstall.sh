#!/bin/sh
make -j100 || exit 1
my_mcms_folder=$PWD
latest=$(readlink /Carmel-Versions/StableBuild/last)
echo "building private version based on "$latest
cd /Carmel-Versions/StableBuild/$latest
./build.sh --MCMS=$my_mcms_folder --TARGET_IP=$1
