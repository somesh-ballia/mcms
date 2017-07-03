#!/mcms/python/bin/python
from McmsConnection import *

import os, string
import sys
import shutil
from SysCfgUtils import *
from UsersUtils import *
import subprocess
import re

#*PRERUN_SCRIPTS=
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export MCMS_DIR=`pwd`
#-- SKIP_ASSERTS

ProcessName = "McuMngr"
connection = McmsConnection()
connection.Connect()
sleep(2)



def getPortAllocation():  
	#ps     = subprocess.Popen("Bin/McuCmd @GetCPUsize Resource", shell=True, stdout=subprocess.PIPE)
	#output = ps.stdout.read()
	#ps.stdout.close()
	#ps.wait()
	connection.LoadXmlFile("Scripts/Resource2Undef3RealParties/ResourceMonitorCarmel.xml")
	connection.Send()
	print connection.xmlResponse.toprettyxml()
	totalPorts = connection.GetTextUnder("RSRC_REPORT_RMX_LIST_HD","TOTAL")
	print "result is:" + totalPorts
	return totalPorts

def testMachineType(bogoval,spusize,memory,expected):
	command = "Bin/McuCmd @set_mcutype McuMngr " + bogoval + " " +  spusize + " " + memory
	print command	
	os.system(command)
	sleep(2)
	output = getPortAllocation()
	

	if re.search(expected, output) is None:		               
	   	sys.exit("Unexpected port allocation in mcu types " + str(output) + " Expected: " + str(expected))
	
