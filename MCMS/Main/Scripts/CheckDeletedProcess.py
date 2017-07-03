#!/usr/bin/python
#This script should not be run with RunTest.sh script
#that is becasue RunTest.sh runs all the processes
#which disables us from checking a missing EXE file.

#!!!!!!---IT SHOULD BE ONLY RUN AS A STANDALONE---!!!!!!
#Dan Porat


import shutil
from McmsConnection import *
from Faults_Tools import *

#Removing the CDR temporarily to tmp folder

shutil.copyfile('Bin/CDR','/tmp/CDR')
os.remove('Bin/CDR')


#Running the system

os.system("Scripts/Startup.sh")

c = McmsConnection()
c.Connect()

#This script Checks for 2 Indications
#1.That we have finished the Startup
#2.That we have the MODULE_DOES_NOT_EXIST fault


for x in range(20):
    c.SendXmlFile('Scripts/GetMcuState.xml')
    state = c.GetTextUnder("MCU_STATE","DESCRIPTION")
    print "Mcu status:" + state
    if state != "Startup":
        m_Faults = Faults()
        res=m_Faults.DoesFaultExist(c,"MODULE_DOES_NOT_EXIST")
    else:    
        print "MCU startup failed"

c.Disconnect()
shutil.copyfile('/tmp/CDR','Bin/CDR')
sys.exit(res)

