#!/mcms/python/bin/python

#-LONG_SCRIPT_TYPE
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

import os



def RunPrintCommand(command):
#    print "---> run " + command
    os.system(command)


def OnMIBFileHasToBeUpdate():
    print ''
    print ''
    print ''
    print '********************************'
    print 'The MIB file has to be updated'
    print '********************************'
    print ''
    print ''
    print ''
    
    message = '1) You have changed the active alarms during development. \n'
    message = message + '2) Now you have to update the MIB file. \n'
    message = message + '3) Do not worry you do not need to understand what the MIB file is.\n'
    message = message + '4) Just do the following actions.\n'
    message = message + '5) If your problem was not solved ask Yuri Ratner ;-). He will help you.\n'
    message = message + '\n'
    message = message + '                                                     Yuri Ratner'

    actions = 'Actions:\n'
    actions = actions + '1) ct co -nc MIBS/POLYCOM-RMX-MIB.MIB \n'
    actions = actions + '2) Scripts/Startup.sh \n'
    actions = actions + '3) Scripts/FixMIBFile.py \n'

    print message
    print ''
    print ''
    print ''
    print actions
    print ''
    print ''
    print ''



# 1) Generate MIB file
tmpMibFile = 'tmpMibFile'
command = 'Scripts/McuCmd.sh mkMib SnmpProcess MIBS/POLYCOM-RMX-MIB.MIB_static ' + tmpMibFile
RunPrintCommand(command)


# 2) Compare the generated file to the exist file
tmpOutputFile = 'tmpTerminalCommandOutput.yura'
command = 'diff MIBS/POLYCOM-RMX-MIB.MIB ' + tmpMibFile + ' | grep -v \"LAST-UPDATED\" > ' + tmpOutputFile
RunPrintCommand(command)


# 3) Get the result
outputSize = os.path.getsize(tmpOutputFile)


# 4) Remove all tmp files
command = 'rm ' + tmpOutputFile
RunPrintCommand(command)
command = 'rm ' + tmpMibFile
RunPrintCommand(command)


# 5) Handle the comparing result
# if date is different the diff prints   12c12
#                                        ---
# which is 10
if(outputSize > 0):
    OnMIBFileHasToBeUpdate()
    exit(1)


print ''
print 'The MIB file is updated, nothing need to be done'
print ''

#exit(0)


