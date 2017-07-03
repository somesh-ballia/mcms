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

Process_failer_policy=[["McuMngr",1],
                      #["Fault",1],
                      #["ApacheModule",3],
                      ["MplApi",2],
                      ["CSApi",2],
                       ["Logger",3],
                       ["CDR",3],
                       ["EncryptionKeyServer",3],
                       ["Resource",1],
                       ["ConfParty",1],
                      # ["CSMngr",2],
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
    r.AddNewUser("SUPPORT","SUPPORT","administrator")
    j.Connect("SUPPORT","SUPPORT")
    
    Process_name=Process_failer_policy[index][0]
    Process_policy=Process_failer_policy[index][1]
    print Process_name
    print Process_policy

  #  c.test_segment_fault(Process_name,"TASK_FAILED",1)
  #  c.test_segment_fault(Process_name,"TASK_FAILED",2)
  #  c.test_segment_fault(Process_name,"TASK_FAILED",3)
  #  c.test_segment_fault(Process_name,"TASK_FAILED",4)
    c.test_kill_process(Process_name,"PROCESS_FAILED",1)
    sleep(5)
    j.test_process__failfault(Process_name,"PROCESS_FAILED",1)
    
    if Process_policy> 1:
       c.test_kill_process(Process_name,"PROCESS_FAILED",2)
       sleep(10)
       print "shlolmit2"
       j.test_process__failfault(Process_name,"PROCESS_FAILED",2)
       c.test_kill_process(Process_name,"PROCESS_FAILED",3)
       sleep(10)
       print "shlolmit3"
       j.test_process__failfault(Process_name,"PROCESS_FAILED",3)
       c.test_kill_process(Process_name,"PROCESS_FAILED",4)
       sleep(10)
       print "shlolmit4"
       j.test_process__failfault(Process_name,"PROCESS_FAILED",4)
       
    j.test_reset_mcu("McmsDaemonCards","PROCESS_TERMINATED",1)
    if Process_policy<> 3:
       j.test_reset_mcu("Cards","RESET_MCU_ON_EXCEPTION",1)
    os.system("Scripts/Startup.sh")

sys.exit(0)




"""
********************************** EXCEPTION **********************************
ASSERT FAILED:
Code: 1, Name: McuMngr, Task ID: 3002997680, Name: Manager
ErrorHandlerTask.cpp:150: error(1):
RetAddr1:0x00171fe0
RetAddr2:0x00463a48
RetAddr3:0x02daf5a1
RetAddr4:0x0017ba9e
RetAddr5:0x0017abfb
*******************************************************************************

********************************** EXCEPTION **********************************
ASSERT FAILED:
Code: 11, Name: McuMngr, Task ID: 3076426672, Name: ErrorHandlerTask
ErrorHandlerTask.cpp:163: error(11): ********** EXECPTION SIGNAL ***********
RetAddr1:0x0017204f
RetAddr2:0x0019b499
RetAddr3:0x0019b664
RetAddr4:0x001bf5d6
RetAddr5:0x001bf28d
*******************************************************************************
"""


