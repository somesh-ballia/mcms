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

def findThisProcess( process_name ):
  ps     = subprocess.Popen("ps -eaf | grep "+process_name + "| grep -v grep", shell=True, stdout=subprocess.PIPE)
  output = ps.stdout.read()
  ps.stdout.close()
  ps.wait()

  return output

# This is the function you can use  
def isThisRunning( process_name ):
  output = findThisProcess( process_name )

  if re.search(process_name, output) is None:
    return False
  else:
    return True

def lsResult( path ):
  ps     = subprocess.Popen("echo -n `ls "+path +"`" , shell=True, stdout=subprocess.PIPE)
  output = ps.stdout.read()
  ps.stdout.close()
  ps.wait()

  return output

sleep(2)
ProcessName = "GideonSim"

connection = McmsConnection()

#################################################################
connection.Connect()

command = "Scripts/McuCmd.sh set_build_burn_rate GideonSim 0 ipmc 5"
os.system(command)
command = "Scripts/McuCmd.sh set_build_burn_rate GideonSim 1 ipmc 7"
os.system(command)
command = "Scripts/McuCmd.sh set_build_burn_rate GideonSim 2 ipmc 10"
os.system(command)

connection.LoadXmlFile('Scripts/UpgradeVersion/StartUpgradeVersion.xml')

connection.Send("") 
print connection.xmlResponse.toprettyxml()
sleep(6)

command = "ln -sf " + lsResult("/Carmel-Versions/NonStableBuild/RMX_7.8/last/RMX_*") + " Install/new_version.bin"
print command
os.system(command)

connection.LoadXmlFile('Scripts/UpgradeVersion/FinishUpgradeVersion.xml')
connection.Send() 
sleep(6)

connection.LoadXmlFile('Scripts/UpgradeVersion/KeycodeForV7_8.xml')
connection.Send("In progress") 

print connection.xmlResponse.toprettyxml()
sleep(6)
         
##installationStatus = connection.GetTextUnder("ACTION","GET_STATE","MCU_STATE","INSTALL_PHASES_LIST","INSTALL_PHASE","INSTALL_PHASE_STATUS")
InstallationInPorgress=True;
instTimeout=0
while InstallationInPorgress:

	install_ended = False
	connection.LoadXmlFile('Scripts/UpgradeVersion/GetInstallationStatus.xml')
	connection.Send()
	installationStatus_list = connection.xmlResponse.getElementsByTagName("INSTALL_PHASE")
	if (len(installationStatus_list) == 0):
    		print "no installation status"
	else:  
     		for index in range(len(installationStatus_list)):         		 
			installationType = installationStatus_list[index].getElementsByTagName("INSTALL_PHASE_TYPE")[0].firstChild.data
			installationProgress = installationStatus_list[index].getElementsByTagName("INSTALL_PHASE_PROGRESS")[0].firstChild.data
                  	installationStatus = installationStatus_list[index].getElementsByTagName("INSTALL_PHASE_STATUS")[0].firstChild.data
         		print installationType + ":progress:" + installationProgress + ",status:" + installationStatus			
			if (installationType == "InstallPhaseType_completed" and installationStatus == "success"): 
				sys.stdout.flush()
				install_ended = True
				InstallationInPorgress=False
		print ""

	sleep(3)
	instTimeout=instTimeout+1
	if (install_ended==True or instTimeout>=60):
		break

connection.Disconnect()			
if (instTimeout>=120):
	exit(-1)

instTimeout=0

while isThisRunning("Bin/McmsDaemon")==False or instTimeout==0:
	instTimeout=instTimeout+1
	if (instTimeout>20):
		print "Installation Failed McmsDaemon did not shutdown properly!"
		exit(-1)
	else:
		print "Process McmsDaemon is Still Up , Sleep for 3 Sec...."	
		sleep(3)

