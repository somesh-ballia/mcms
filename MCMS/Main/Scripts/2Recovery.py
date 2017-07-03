#!/mcms/python/bin/python
from McmsTargetConnection import *


#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6

import os, string
import sys
import shutil
from SysCfgUtils import *
from UsersUtils import *

sleep(2)
ProcessName = "CDR"

c = McmsTargetConnection()

c.Connect()

#################################################################

from McmsConnection import *

class test_recovery_policy_Utilities ( McmsConnection ):
    
#------------------------------------------------------------------------------
    def test_segment_fault(self,proc_name,expected_fault,expected_fault_retries_num):
        print "Test +proc_name+ Indications...."
        SM_fail = 0
        fault_elm_list=0
 
       # os.system("Bin/McuCmd segment_fault Logger")
       # fault_elm_list = self.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
	#	num_of_faults = len(fault_elm_list)
#		for index in range(len(fault_elm_list)):
	#	        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
#		        print fault_desc
		#        if fault_desc == "TASK_FAILED":
#		           SM_fail = SM_fail+1
                   
		#if SM_fail <> 1:
		#    print "Test failed,  no SM fail Indications found in Faults"
		  #  sys.exit(1)
		      #  return(0)
#------------------------------------------------------------------------------


test_segment_fault(Logger,2,3)


####################################################################
#####################
print "Test Logger Indications...."
SM_fail = 0
fault_elm_list=0


#####################
os.system("Bin/McuCmd segment_fault Logger")
#####################

fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "TASK_FAILED":
           SM_fail = SM_fail+1
                   
if SM_fail <> 1:
    print "Test failed,  no SM fail Indications found in Faults"
    sys.exit(1)
######################


os.system("Bin/McuCmd segment_fault Logger")
os.system("Bin/McuCmd segment_fault Logger")

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

