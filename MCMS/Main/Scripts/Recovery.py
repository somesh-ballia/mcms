#!/mcms/python/bin/python
from McmsTargetConnection import *


#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

#import os, string
#import sys
#import shutil
#
from SysCfgUtils import *
from UsersUtils import *


ProcessName = "CDR"

c = McmsTargetConnection()

c.Connect()

# put here code for test party connection.
# find endpoint that can be tested during night
#c.SimpleXmlConfPartyTest('Scripts/AddVideoCpConf.xml',
#                         'Scripts/AddVideoParty1.xml',
#                         2,
#                         5)


print c.RemoteCommand("killall " + ProcessName)

#print c.RemoteCommand("killall Cards")
#sleep(100)
#print c.RemoteCommand("killall Resource")
#sleep(100)
#print c.RemoteCommand("killall Gatekeeper")
#sleep(100)
#print c.RemoteCommand("Reset.sh")

                         
############################################
# 
##############################################
#c = McmsTargetConnection()
#c.Connect()
#print c.RemoteCommand("pidof McuMngr")

r = SyscfgUtilities()
r.UpdateSyscfgFlag("MCMS_PARAMETERS_USER","DEBUG_MODE","NO","user")
#r.Connect()
#r.SendSyscfgFlags("Debug")
#r.Disconnect()

#c.RemoteCommand("/mcms/Scripts/Reset.sh")

#os.environ["CLEAN_CFG"]="NO"
#os.system("Scripts/Startup.sh")
#print c.RemoteCommand("killall -9  McuMngr")
#for line in c.RemoteCommand("ps"):
#	sys.stdout.write(line)

#c.Disconnect()

###########################################

