#!/usr/bin/python
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

r = UsersUtilities()
c = UsersUtilities()
j = UsersUtilities()
sleep(2)
#ProcessName = "CDR"

c.Connect()
r.Connect()
#r.AddNewUser("SUPPORT","SUPPORT","administrator")
j.Connect("SUPPORT","SUPPORT")
    
os.environ["CLEAN_FAULTS"]="NO"
os.system("Bin/McuCmd add_aa CDR 16 YES")
"""
os.system("Bin/McuCmd add_aa CDR 3 YES")
sleep (60)
os.system("Bin/McuCmd add_aa CDR 3 YES")
sleep (60)
os.system("Bin/McuCmd add_aa CDR 3 YES")
sleep (60)
os.system("Bin/McuCmd add_aa CDR 3 YES")
sleep (60)
os.system("Bin/McuCmd add_aa CDR 3 YES")
sleep (60)
os.system("Bin/McuCmd add_aa CDR 3 YES")
"""
sleep (4)
r.SendXmlFile("Scripts/GetFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        
sleep(2)

print "First"
os.system("Scripts/Startup.sh")
c.Connect()
r.Connect()
r.AddNewUser("SUPPORT","SUPPORT","administrator")
j.Connect("SUPPORT","SUPPORT")
#j.SendXmlFile("Scripts/GetFaults.xml")

r.SendXmlFile("Scripts/GetFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
"""
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
		#print general_msg
"""		
os.system("Bin/McuCmd add_aa CDR 3 YES")
r.SendXmlFile("Scripts/GetFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
"""
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
"""

print "End"
#sys.exit(0)






