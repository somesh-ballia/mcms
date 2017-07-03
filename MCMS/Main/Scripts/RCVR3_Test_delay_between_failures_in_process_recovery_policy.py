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
m = SyscfgUtilities()
#m.Connect()
#m.UpdateSyscfgFlag("MCMS_PARAMETERS_DEBUG","MAX_TIME_BETWEEN_PROCESS_CRUSH",90)
##m.UpdateSyscfgFlag("MCMS_PARAMETERS_DEBUG","MAX_TIME_BETWEEN_TASK_CRUSH",90)
#sleep (1)
#os.environ["CLEAN_CFG"]="NO"
#os.system("Scripts/Startup.sh")

c = test_recovery_policy_Utilities()
j = test_recovery_policy_Utilities()
r = UsersUtilities()

Process_failer_policy= [#["MplApi",2],
                        #["CSApi",2],
                         ["Logger",3],
                         ["CDR",3],
                         #["EncryptionKeyServer",3],
                         #["CSMngr",1],
                         #["DNSAgent",2],
                         #["SipProxy",2],
                         ["Configurator",3],
                         #["Authentication",2],
                         ["QAAPI",2]
                       ]
c.Connect()
r.Connect()
r.AddNewUser("SUPPORT","SUPPORT","administrator")

                       
for index in range(len(Process_failer_policy)):
    m.Connect()
    m.UpdateSyscfgFlag("MCMS_PARAMETERS_DEBUG","MAX_TIME_BETWEEN_PROCESS_CRUSH",90)
#m.UpdateSyscfgFlag("MCMS_PARAMETERS_DEBUG","MAX_TIME_BETWEEN_TASK_CRUSH",90)
    sleep (1)
    os.environ["CLEAN_CFG"]="NO"
    
    os.system("Scripts/Startup.sh")
    c.Connect()
    r.Connect()
    ##c.tstat_McmsDaemon("CDR")
    #r.AddNewUser("SUPPORT","SUPPORT","administrator")
    j.Connect("SUPPORT","SUPPORT")
    sleep(15)
    Process_name=Process_failer_policy[index][0]
    Process_policy=Process_failer_policy[index][1]
    print Process_name
    print Process_policy
    print "*******************************cycle1*************************"
    c.test_kill_process(Process_name,"PROCESS_FAILED",1)
    sleep(15)
    j.test_process__failfault(Process_name,"PROCESS_FAILED",1)

    sleep(91)
    #c.tstat_McmsDaemon("CDR")

    c.test_kill_process(Process_name,"PROCESS_FAILED",2)
    sleep(10)
    print "*******************************cycle2*************************"
    #c.tstat_McmsDaemon("CDR")
    j.test_process__failfault(Process_name,"PROCESS_FAILED",2,1)

    c.test_kill_process(Process_name,"PROCESS_FAILED",3)
    sleep(10)
    print "*******************************cycle3*************************"
    #c.tstat_McmsDaemon("CDR")
    j.test_process__failfault(Process_name,"PROCESS_FAILED",3)

    c.test_kill_process(Process_name,"PROCESS_FAILED",4)
    sleep(10)
    print "*******************************cycle4*************************"
    #c.tstat_McmsDaemon("CDR")
    j.test_process__failfault(Process_name,"PROCESS_FAILED",4)
       
    j.test_reset_mcu("McmsDaemonCards","PROCESS_TERMINATED",0,1)
    if Process_policy<> 3:
       j.test_reset_mcu("Cards","RESET_MCU_ON_EXCEPTION",0)
    
    j.test_process__failfault(Process_name,"PROCESS_FAILED",4)
    c.test_kill_process(Process_name,"PROCESS_FAILED",5)
    sleep(10)
    print "*******************************cycle5*************************"
    #c.tstat_McmsDaemon("CDR")
    j.test_process__failfault(Process_name,"PROCESS_FAILED",5,1)
       
    j.test_reset_mcu("McmsDaemonCards","PROCESS_TERMINATED",1)
    if Process_policy<> 3:
       j.test_reset_mcu("Cards","RESET_MCU_ON_EXCEPTION",1)
      
    #os.system("Scripts/Startup.sh")
print "Test_Passed!!!!"
sys.exit(0)
