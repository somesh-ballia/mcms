#!/mcms/python/bin/python
from McmsConnection import *

import os, string
import sys
import shutil
import subprocess
import re
import commands
from SysCfgUtils import *
from UsersUtils import *

#*PRERUN_SCRIPTS=
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export SOFT_MCU="YES"
#*export MCMS_DIR=`pwd`
#*export USE_ALT_MNGMNT_SERVICE=Scripts/CsSimulationConfig/NetworkCfg_Management_ipv4_only.xml
#-- SKIP_ASSERTS


def RunCommand(command):
	ps     = subprocess.Popen(command , shell=True, stdout=subprocess.PIPE)
	output = ps.stdout.read()
	ps.stdout.close()
	ps.wait()
	return output
		


#Create McmsConnection , and Check Via API the Active Alarm List to see if it is Empty
#and if the System State is in Normal.

connection = McmsConnection()
connection.Connect()
connection.LoadXmlFile('Scripts/GetMcuState.xml')
connection.Send()
print connection.xmlResponse.toprettyxml()
text = connection.xmlResponse.getElementsByTagName("MCU_STATE")[0].getElementsByTagName("DESCRIPTION")[0].firstChild.data
print "========== mcu state -> DESCRIPTION : " + text
if -1 == text.find("Normal"):
    sys.exit(1)
text = connection.xmlResponse.getElementsByTagName("MCU_STATE")[0].getElementsByTagName("LICENSING_VALIDATION_STATE")[0].firstChild.data
print "========== mcu state -> LICENSING_VALIDATION_STATE : " + text
if -1 == text.find("success"):
    sys.exit(1)
text = connection.xmlResponse.getElementsByTagName("MCU_STATE")[0].getElementsByTagName("NUMBER_OF_ACTIVE_ALARMS")[0].firstChild.data
print "========== mcu state -> NUMBER_OF_ACTIVE_ALARMS : " + text
if -1 == text.find("0"):
    sys.exit(1)
text = connection.xmlResponse.getElementsByTagName("MCU_STATE")[0].getElementsByTagName("NUMBER_OF_CORE_DUMPS")[0].firstChild.data
print "========== mcu state -> NUMBER_OF_CORE_DUMPS : " + text
if -1 == text.find("0"):
    sys.exit(1)

