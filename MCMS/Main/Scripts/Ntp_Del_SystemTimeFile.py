#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_7



#*CLEAN_CFG = NO
import os
import sys
import shutil

from SysCfgUtils import *

############################################
# 
##############################################
from McmsConnection import *
 
c = McmsConnection()
c.Connect()


os.system("rm Cfg/SystemTime.xml")
os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")

c.Connect()
print "Monitor active alarms..."
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
noConnetion = 0
for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "NO_TIME_CONFIGURATION":
        noConnetion = noConnetion+1
print noConnetion 
if noConnetion <> 1:
    print "Test failed, NO_TIME_CONFIGURATIONfound in active alararms after delet the systemTime file and restarted"
    sys.exit(1)

print "set system time" 
c.SendXmlFile("Scripts/SetSystemTime.xml")
sleep(1)
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
noConnetion = 0
for index in range(len(fault_elm_list)):
    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
    print fault_desc
    if fault_desc == "NO_TIME_CONFIGURATION":
        noConnetion = noConnetion+1
 
if noConnetion <> 0:
    print "Test failed, NO_TIME_CONFIGURATION found in active alararms After set system time"
    sys.exit(1)
