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


sleep(2)
ProcessName = "CDR"

c = test_recovery_policy_Utilities()
j = test_recovery_policy_Utilities()
r = UsersUtilities()

Process_failer_policy=[#["McuMngr",1],
                      #["Fault",1],
                      #["ApacheModule",3],
                      # ["MplApi",2],
                      # ["CSApi",2],
                       ["SNMPProcess",3],
                       ["Logger",3],
                       ["CDR",3],
                       ["EncryptionKeyServer",3],
                       ["Resource",1],
                       ["ConfParty",1],
                      # ["CSMngr",2],
                       ["Cards",1],
                       ["DNSAgent",2],
                      ["SipProxy",2],
                       #["Gatekeeper",1],
                       ["Configurator",3]]
                      #["Authentication",2]]
                      # ["QAAPI",2]]
for index in range(len(Process_failer_policy)):
    print "testing " + Process_failer_policy[index][0]
    print "Waiting 40 seconds before mcms daemon software watch dog starts"
    sleep(40)
    c.Connect()
    r.Connect()
    j.Connect("SUPPORT","SUPPORT")
    
    Process_name=Process_failer_policy[index][0]
    Process_policy=Process_failer_policy[index][1]
    print Process_name
    print Process_policy
    os.environ["CLEAN_FAULTS"]="NO"
    c.test_kill_process_dbg_mode_no(Process_name,"PROCESS_FAILED",1)
    if Process_policy==1:
       os.system("Scripts/Startup.sh")
       sleep (20)
       c.Connect()
       r.Connect()
       j.Connect("SUPPORT","SUPPORT")
       j.test_process__failfault(Process_name,"PROCESS_FAILED",1)
    
    if Process_policy> 1:
       c.test_kill_process(Process_name,"PROCESS_FAILED",2)
       c.test_kill_process(Process_name,"PROCESS_FAILED",3)
       j.Connect("SUPPORT","SUPPORT")
       j.test_process__failfault(Process_name,"PROCESS_FAILED",3)
       c.test_kill_process_dbg_mode_no(Process_name,"PROCESS_FAILED",4)
   
    if Process_policy==2:
       os.environ["CLEAN_FAULTS"]="NO"
       sleep(1)
       os.system("Scripts/Startup.sh")
       c.Connect()
       r.Connect()
       j.Connect("SUPPORT","SUPPORT")
       j.test_process__failfault_dbg_mode_no(Process_name,"PROCESS_FAILED",4,1)
    
    if Process_policy<> 3:
        j.Connect("SUPPORT","SUPPORT")
        j.test_reset_mcu("McmsDaemonCards","PROCESS_TERMINATED",1)
        j.test_reset_mcu("Cards","RESET_MCU_INTERNAL",1)
       
    os.environ["CLEAN_FAULTS"]="YES"
    sleep(1)
    os.system("Scripts/Startup.sh")

sys.exit(0)






