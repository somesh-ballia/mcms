#!/mcms/python/bin/python
from McmsConnection import *


import os, string
import sys
import shutil
from SysCfgUtils import *
from UsersUtils import *

sleep(2)
ProcessName = "GideonSim"

##c = McmsTargetConnection()

connection = McmsConnection()

#################################################################
connection.Connect()

command = "Bin/McuCmd set_build_burn_rate GideonSim 0 ipmc 5"
os.system(command)
command = "Bin/McuCmd set_build_burn_rate GideonSim 1 ipmc 7"
os.system(command)
command = "Bin/McuCmd set_build_burn_rate GideonSim 2 ipmc 10"
os.system(command)

connection.LoadXmlFile('Scripts/UpgradeVersion/StartUpgradeVersion.xml')

connection.Send("") 
print connection.xmlResponse.toprettyxml()
sleep(6)

connection.LoadXmlFile('Scripts/UpgradeVersion/FinishUpgradeVersion.xml')
connection.Send() 

         
##installationStatus = connection.GetTextUnder("ACTION","GET_STATE","MCU_STATE","INSTALL_PHASES_LIST","INSTALL_PHASE","INSTALL_PHASE_STATUS")
while True:
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
				connection.Disconnect()			
				sys.stdout.flush()
				install_ended = True				
		print ""

	sleep(3)
	if (install_ended):
		break






