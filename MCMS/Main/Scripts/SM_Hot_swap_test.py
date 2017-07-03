#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6

import os
import sys
import shutil

from SysCfgUtils import *

############################################
# 
##############################################
 
c = McmsConnection()
sleep(5)
c.Connect()
SM_fail = 0

if SM_fail <> 0:
    print "Test failed, VOLTAGE_PROBLEM found in Faults BEFORE the test started"
    sys.exit(1)

if SM_fail == 0:
   print "Good Start, Faults found no errors BEFORE the test started"

debug_mode_str= str("Bin/McuCmd set McmsDaemon DEBUG_MODE YES")
os.system(debug_mode_str)
#////////////////////////////////*************MFA********///////////////////////////////////////////////
SM_fail = 0
fault_elm_list=0
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 1 from 2 to 2 32 ")

sleep(3)
SM_fail=0
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "POWE_OFF_PROBLEM":
           SM_fail = SM_fail+1
 
if SM_fail == 0:
    print "Test failed,  no SM MFA fail Indications found in Faults"
    sys.exit(1)

if SM_fail > 1:
    print "Test failed, More than 5 SM MFA fail Indications"
    sys.exit(1) 
    
if SM_fail == 1:
    print "Good !!! POWE_OFF_PROBLEM Indications found in Faults "

SM_fail = 0
fault_elm_list=0
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 1 from 2 to 2 0 ")

sleep(2)
#SM_fail=0
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "VOLTAGE_PROBLEM":
           SM_fail = SM_fail+1
        if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
           SM_fail = SM_fail+1    
        if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
           SM_fail = SM_fail+1  
        if fault_desc == "FAILURE_PROBLEM":
           SM_fail = SM_fail+1   
        if fault_desc == "OTHER_PROBLEM":
           SM_fail = SM_fail+1
if SM_fail == 5:
    print "Test failed,  the 5 SM MFA fail Indications found in Active alarm, while MFA status is OK"
    sys.exit(1)

if SM_fail > 5:
    print "Test failed, more than 5 SM MFA fail Indications found in Active alarm, while MFA status is OK"
    sys.exit(1) 
    
if SM_fail == 0:
    print "Good !!! The 5 SM MFA fail  active alarm Indications has delete"
