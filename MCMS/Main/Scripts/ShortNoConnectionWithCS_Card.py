#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_7

#-- EXPECTED_ASSERT(1)=conId does not exist

#*CLEAN_CFG = NO
import os
import sys
import shutil

from SysCfgUtils import *

############################################
# 
##############################################


r = SyscfgUtilities()
r.Connect()

r.UpdateSyscfgFlag("MCMS_PARAMETERS_DEBUG","KEEP_ALIVE_RECEIVE_PERIOD",1)
sleep (1)
#r.UpdateSyscfgFlag("Other","CS_KEEP_ALIVE_SEND_PERIOD",1)

os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")

os.system("Scripts/NoConnectionWithCS_Card.py")

#r.Disconnect()

