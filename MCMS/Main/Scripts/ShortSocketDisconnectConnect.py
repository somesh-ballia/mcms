#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6
#-- EXPECTED_ASSERT(4)=conId does not exist
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

r.UpdateSyscfgFlag("MCMS_PARAMETERS_DEBUG","KEEP_ALIVE_RECEIVE_PERIOD",3)
sleep (5)

#r.UpdateSyscfgFlag("Other","CM_KEEP_ALIVE_SEND_PERIOD",3)

os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")

#os.system("Scripts/SocketDisconnectConnect.py")
os.system("Scripts/ActiveA_SocketDisconnectConnect.py")

#r.Disconnect()

