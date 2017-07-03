#!/mcms/python/bin/python


import string
import shutil
from SysCfgUtils import *
from UsersUtils import *
import subprocess
import re

from SoftMcuMcuTypes import *

#*PRERUN_SCRIPTS=
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export SOFT_MCU_MFW="YES"
#*export MCMS_DIR=`pwd`
#-- SKIP_ASSERTS



testMachineType("5800","33","12","67")
testMachineType("5800","32","12","67")
testMachineType("5800","24","12","14")
testMachineType("5800","16","8","14")
testMachineType("5800","12","8","14")
testMachineType("5400","33","8","14")
testMachineType("5400","24","8","14")
testMachineType("5400","16","8","14")
testMachineType("5400","12","8","14")
testMachineType("5600","24","8","14")
testMachineType("5600","16","8","4")


#################################################################
