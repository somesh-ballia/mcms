#!/mcms/python/bin/python

#*Script_Info_Status="Non Active"
#*Script_Info_Name="111.py"
#*Script_Info_Group="???"
#*Script_Info_Programmer="???"
#*Script_Info_Version="???"
#*Script_Info_Description="???"


#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6
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

#r.UpdateSyscfgFlag("MCMS_PARAMETERS_DEBUG","KEEP_ALIVE_RECEIVE_PERIOD",1)
#sleep (10)
#r.UpdateSyscfgFlag("Other","CM_KEEP_ALIVE_SEND_PERIOD",1)

os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")

#os.system("Scripts/NoConnectionWithMFA_Card.py")

#r.Disconnect()

