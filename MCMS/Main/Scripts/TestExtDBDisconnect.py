#!/mcms/python/bin/python
# -*- coding: utf-8 -*-

import os
import sys

from AlertUtils import *
from SysCfgUtils import *


def RunSystemCommand(command):
    print ""
    print "Command: " + command
    os.system(command)

def RunSleep(secondsToSleep):
    print ""
    index = 0
    sys.stdout.write("Sleep " + str(secondsToSleep) + " seconds : ")
    sys.stdout.flush()
    while index < secondsToSleep:
        sys.stdout.write(str(index))
        sys.stdout.flush()
        sleep(1)
        index = index + 1
    print ""






#--------------------------------------------------------------------------------------
# start test
#--------------------------------------------------------------------------------------

# 1. set the CFG params
sysCfgUtils = SyscfgUtilities()
sysCfgUtils.ConnectSpecificUser("SUPPORT", "SUPPORT")

sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_IP", "127.0.0.1", "user")
#sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PORT","8080","user")
sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PORT","80","user")
sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_LOGIN","POLYCOM","user")
sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_PASSWORD","POLYCOM","user")
sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "EXTERNAL_DB_DIRECTORY","A","user")
sysCfgUtils.AddCfgParam("MCMS_PARAMETERS_USER", "ENABLE_EXTERNAL_DB_ACCESS","YES","user")

sysCfgUtils.PrintCfgTable()
sysCfgUtils.SendSetCfg()

sysCfgUtilsDebug = SyscfgUtilities()
sysCfgUtilsDebug.ConnectSpecificUser("SUPPORT", "SUPPORT")
sysCfgUtilsDebug.AddCfgParam("MCMS_PARAMETERS_DEBUG", "DEBUG_MODE", "YES", "debug")
sysCfgUtilsDebug.AddCfgParam("MCMS_PARAMETERS_DEBUG", "QAAPI_KEEP_ALIVE_PERIOD", "2", "debug")

sysCfgUtilsDebug.PrintCfgTable()
sysCfgUtilsDebug.SendSetCfg()

RunSleep(2)



# 2. Reset the system
os.environ["CLEAN_CFG"]="NO"
RunSystemCommand("Scripts/Destroy.sh")
RunSystemCommand("Scripts/Startup.sh")



# 3. kill External DB simulation,
RunSystemCommand("Bin/McuCmd kill EndpointsSim")


RunSleep(5)

alertUtils = AlertUtils()
alertUtils.IsAlertExist("QAAPI", "Manager", "CAN_NOT_ESTABLISH_CONNECTION_WITH_APPLICATION_SERVER", 1)


