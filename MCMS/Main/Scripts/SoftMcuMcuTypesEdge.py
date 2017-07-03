#!/mcms/python/bin/python

from McmsConnection import *

import os, string
import sys
import shutil
from SysCfgUtils import *
from UsersUtils import *
import subprocess
import re

from SoftMcuMcuTypes import *

#*PRERUN_SCRIPTS=
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export SOFT_MCU_EDGE="YES"
#*export MCMS_DIR=`pwd`
#*export FLEXERA_LICENSE="YES"
#-- SKIP_ASSERTS
#*export TARGET_PORT=443

os.system("Scripts/TestFlexeraLicense.py")

testMachineType("5800","36","16","20")
testMachineType("5800","30","12","20")
testMachineType("5800","22","12","15")
testMachineType("5800","14","8","7")
testMachineType("5800","10","8","5")
testMachineType("5400","30","8","13")
testMachineType("5400","22","8","10")
testMachineType("5400","14","8","6")
testMachineType("5400","10","8","5")
testMachineType("5600","22","8","10")
testMachineType("5600","14","8","7")


#################################################################


