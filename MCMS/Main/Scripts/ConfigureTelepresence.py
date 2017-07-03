#!/mcms/python/bin/python

#############################################################################
# Creating Conf with a 4 defined Dial In SIP participants
#
# Date: 23/01/05
# By  : Ron S.

#############################################################################
import os
import sys
import shutil

from McmsConnection import *
from SysCfgUtils import *
from UsersUtils import *

sysCfgUtils = SyscfgUtilities()
sysCfgUtils.ConnectSpecificUser("SUPPORT", "SUPPORT")

sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER","ITP_CERTIFICATION","YES","user")
## sysCfgUtils.UpdateSyscfgFlag("MCMS_PARAMETERS_USER","ITP_CERTIFICATION","YES","user")

sysCfgUtils.SendSetCfg()
print "updating ITP_CERTIFICATION=YES"
sysCfgUtils.Sleep(3)

os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Destroy.sh")
os.system("Scripts/Startup.sh")

sleep(20)
##--------------------------------------- TEST ---------------------------------

