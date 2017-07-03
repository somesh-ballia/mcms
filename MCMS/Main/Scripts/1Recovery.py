#!/mcms/python/bin/python
from McmsTargetConnection import *
from McmsConnection import *
#from test_recovery_policy_Utilities import *


#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6

import os, string
import sys
import shutil
from RRecovery import *
from SysCfgUtils import *
from UsersUtils import *


sleep(2)
ProcessName = "CDR"

c = test_recovery_policy_Utilities()
j = test_recovery_policy_Utilities()
r = UsersUtilities()
#McmsTargetConnection()

c.Connect()
r.Connect()
r.AddNewUser("SUPPORT","SUPPORT","administrator")
j.Connect("SUPPORT","SUPPORT")
#################################################################
"""
c.test_segment_fault("Cards","TASK_FAILED",1)
c.test_segment_fault("Cards","TASK_FAILED",2)
c.test_segment_fault("Cards","TASK_FAILED",3)
c.test_segment_fault("Cards","TASK_FAILED",4)

c.test_kill_process("Cards","PROCESS_FAILED",2)
sleep(5)
j.test_process__failfault("Cards","PROCESS_FAILED",1)
sys.exit(0)
j.test_reset_mcu("Cards","RESET_MCU_ON_EXCEPTION",1)
"""
 
c.test_segment_fault("Logger","TASK_FAILED",1)
c.test_segment_fault("Logger","TASK_FAILED",2)
c.test_segment_fault("Logger","TASK_FAILED",3)
c.test_segment_fault("Logger","TASK_FAILED",4)
sleep(5)
print "shlolmit1"
j.test_process__failfault("Logger","PROCESS_FAILED",1)
print "shlolmit2"
#c.test_segment_fault("Logger","TASK_FAILED",5)
#c.test_segment_fault("Logger","TASK_FAILED",6)
#c.test_segment_fault("Logger","TASK_FAILED",7)
#c.test_segment_fault("Logger","TASK_FAILED",8)
c.test_kill_process("Logger","PROCESS_FAILED",2)
sleep(25)
print "shlolmit3"
j.test_process__failfault("Logger","PROCESS_FAILED",2)
sys.exit(0)
c.test_kill_process("Logger","PROCESS_FAILED",2)
sleep(5)
sys.exit(0)

c.test_segment_fault("CDR","TASK_FAILED",1)
c.test_segment_fault("CDR","TASK_FAILED",2)
c.test_segment_fault("CDR","TASK_FAILED",3)
c.test_segment_fault("CDR","TASK_FAILED",4)
sleep(5)
j.test_process__failfault("CDR","PROCESS_FAILED",1)

sleep(30)


c.test_segment_fault("Configurator","TASK_FAILED",1)
c.test_segment_fault("Configurator","TASK_FAILED",2)
c.test_segment_fault("Configurator","TASK_FAILED",3)
c.test_segment_fault("Configurator","TASK_FAILED",4)
sleep(5)
j.test_process__failfault("Configurator","PROCESS_FAILED",1)
sleep(5)
c.Connect()
sys.exit(0)


#c.test_segment_fault("ApacheModule","TASK_FAILED",1)
#c.test_segment_fault("ApacheModule","TASK_FAILED",2)
#c.test_segment_fault("ApacheModule","TASK_FAILED",3)
#c.test_segment_fault("ApacheModule","TASK_FAILED",4)

c.test_segment_fault("MplApi","TASK_FAILED",1)
c.test_segment_fault("MplApi","TASK_FAILED",2)
c.test_segment_fault("MplApi","TASK_FAILED",3)
c.test_segment_fault("MplApi","TASK_FAILED",4)

c.test_segment_fault("CSApi","TASK_FAILED",1)
c.test_segment_fault("CSApi","TASK_FAILED",2)
c.test_segment_fault("CSApi","TASK_FAILED",3)
c.test_segment_fault("CSApi","TASK_FAILED",4)

c.test_segment_fault("Authentication","TASK_FAILED",1)
c.test_segment_fault("Authentication","TASK_FAILED",2)
c.test_segment_fault("Authentication","TASK_FAILED",3)
c.test_segment_fault("Authentication","TASK_FAILED",4)


c.test_segment_fault("EncryptionKeyServer","TASK_FAILED",1)
c.test_segment_fault("EncryptionKeyServer","TASK_FAILED",2)
c.test_segment_fault("EncryptionKeyServer","TASK_FAILED",3)
c.test_segment_fault("EncryptionKeyServer","TASK_FAILED",4)
#sleep(30)
sys.exit(0)


c.test_segment_fault("QAAPI","TASK_FAILED",1)
c.test_segment_fault("QAAPI","TASK_FAILED",2)
c.test_segment_fault("QAAPI","TASK_FAILED",3)
c.test_segment_fault("QAAPI","TASK_FAILED",4)
sleep(30)

c.test_segment_fault("McuMngr","TASK_FAILED",1)
c.test_segment_fault("McuMngr","TASK_FAILED",2)
c.test_segment_fault("McuMngr","TASK_FAILED",3)
#c.test_segment_fault("McuMngr","TASK_FAILED",4)

#sleep(30)

#sleep(30)
c.test_segment_fault("ConfParty","TASK_FAILED",1)
c.test_segment_fault("ConfParty","TASK_FAILED",2)
c.test_segment_fault("ConfParty","TASK_FAILED",3)
#c.test_segment_fault("ConfParty","TASK_FAILED",4)

#sleep(30)
c.test_segment_fault("Resource","TASK_FAILED",1)
c.test_segment_fault("Resource","TASK_FAILED",2)
c.test_segment_fault("Resource","TASK_FAILED",3)
#c.test_segment_fault("Resource","TASK_FAILED",4)

#sleep(30)
c.test_segment_fault("Configurator","TASK_FAILED",1)
c.test_segment_fault("Configurator","TASK_FAILED",2)
c.test_segment_fault("Configurator","TASK_FAILED",3)
#c.test_segment_fault("Configurator","TASK_FAILED",4)

#sleep(30)
c.test_segment_fault("SipProxy","TASK_FAILED",1)
c.test_segment_fault("SipProxy","TASK_FAILED",2)
c.test_segment_fault("SipProxy","TASK_FAILED",3)
c.test_segment_fault("SipProxy","TASK_FAILED",4)
sleep(30)
c.test_segment_fault("SipProxy","TASK_FAILED",5)
c.test_segment_fault("SipProxy","TASK_FAILED",6)
c.test_segment_fault("SipProxy","TASK_FAILED",7)
c.test_segment_fault("SipProxy","TASK_FAILED",8)
c.test_segment_fault("SipProxy","TASK_FAILED",9)

c.test_segment_fault("DNSAgent","TASK_FAILED",1)
c.test_segment_fault("DNSAgent","TASK_FAILED",2)
c.test_segment_fault("DNSAgent","TASK_FAILED",3)
c.test_segment_fault("DNSAgent","TASK_FAILED",4)
sleep(30)
c.test_segment_fault("DNSAgent","TASK_FAILED",5)
c.test_segment_fault("DNSAgent","TASK_FAILED",6)
c.test_segment_fault("DNSAgent","TASK_FAILED",7)
c.test_segment_fault("DNSAgent","TASK_FAILED",8)
c.test_segment_fault("DNSAgent","TASK_FAILED",9)

#sleep(30)
c.test_segment_fault("Gatekeeper","TASK_FAILED",1)
c.test_segment_fault("Gatekeeper","TASK_FAILED",2)
c.test_segment_fault("Gatekeeper","TASK_FAILED",3)
c.test_segment_fault("Gatekeeper","TASK_FAILED",4)

#sleep(30)





#os.system("Bin/McuCmd segment_fault Logger")
#os.system("Bin/McuCmd segment_fault Logger")

sleep(2)
#os.system("killall " + ProcessName")
#print c.RemoteCommand("killall " + ProcessName)

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

#r = SyscfgUtilities()
#r.UpdateSyscfgFlag("MCMS_PARAMETERS_USER","DEBUG_MODE","NO","user")
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

