#!/mcms/python/bin/python
from McmsTargetConnection import *
from McmsConnection import *
#from test_recovery_policy_Utilities import *
#-- SKIP_ASSERTS
#-LONG_SCRIPT_TYPE
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

import os, string
import sys
import shutil
from RRecovery import *
from SysCfgUtils import *
from UsersUtils import *

m = SyscfgUtilities()
m.Connect()
#m.UpdateSyscfgFlag("MCMS_PARAMETERS_DEBUG","MAX_TIME_BETWEEN_PROCESS_CRUSH",90)
m.UpdateSyscfgFlag("MCMS_PARAMETERS_DEBUG","MAX_TIME_BETWEEN_TASK_CRUSH",90)
sleep (1)
os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")

a = test_recovery_policy_Utilities()
c = test_recovery_policy_Utilities()
j = test_recovery_policy_Utilities()
r = UsersUtilities()
b = UsersUtilities()
a.Connect()
b.Connect()
b.AddNewUser("SUPPORT","SUPPORT","administrator")

Process_failer_policy=[["McuMngr",1],
                      #["Fault",1],
                      #["ApacheModule",3],
                       #["MplApi",2],
                       #["CSApi",2],
                       ["Logger",3],
                       ["CDR",3],
                      #["EncryptionKeyServer",3],
                       ["Resource",1],
                       #["ConfParty",1],
                       ["CSMngr",2],
                       ["Cards",1],
                       ["DNSAgent",2],
                       ["SipProxy",2],
                       ["Gatekeeper",1],
                       ["Configurator",3],
                       ["Authentication",2],
                       ["QAAPI",2]]
for index in range(len(Process_failer_policy)):
    c.Connect()
    r.Connect()
#    r.AddNewUser("SUPPORT","SUPPORT","administrator")
    j.Connect("SUPPORT","SUPPORT")
    
    Process_name=Process_failer_policy[index][0]
    Process_policy=Process_failer_policy[index][1]
    print Process_name
    print Process_policy
    c.test_segment_fault(Process_name,"TASK_FAILED",1)
    sleep(91)
    c.test_segment_fault(Process_name,"TASK_FAILED",2)
    c.test_segment_fault(Process_name,"TASK_FAILED",3)
    c.test_segment_fault(Process_name,"TASK_FAILED",4)
    sleep(10)
    j.test_process__failfault(Process_name,"PROCESS_FAILED",0,1)
    c.test_segment_fault(Process_name,"TASK_FAILED",5)
    sleep(15)
    j.test_process__failfault(Process_name,"PROCESS_FAILED",1,1)
    sleep(10)
#    j.test_reset_mcu("McmsDaemonCards","PROCESS_TERMINATED",1)# ?????????
#  # if Process_policy ==1:
#   #    j.test_reset_mcu("Cards","RESET_MCU_ON_EXCEPTION",1)
    #m.Connect()
   # m.UpdateSyscfgFlag("MCMS_PARAMETERS_DEBUG","MAX_TIME_BETWEEN_TASK_CRUSH",90)
    sleep (1)
    os.environ["CLEAN_CFG"]="NO"
    os.system("Scripts/Startup.sh")
sleep (10)
str_to_rm_core1=str("rm *core*RCVR*")
os.system(str_to_rm_core1)

sys.exit(0)







