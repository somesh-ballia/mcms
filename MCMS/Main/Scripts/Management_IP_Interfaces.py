#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6
#-- EXPECTED_ASSERT(1)=conId does not exist
import os
import sys
import shutil

from SysCfgUtils import *

############################################
# 
##############################################



r = SyscfgUtilities()
r.Connect()

os.system("export CLEAN_CFG=NO")
os.system("Scripts/Startup.sh")



#r.UpdateSyscfgFlag("COMMON","SYSTEM_TYPE","PIZZA")
#sleep (5)

#os.system("export CLEAN_CFG=NO")
#os.system("Scripts/Startup.sh")

r.Disconnect()

