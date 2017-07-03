#!/mcms/python/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_17
#*export ENDPOINTSSIM=NO


from IpServiceUtils import *

ipServiceUtils = IpServiceUtils()
ipServiceUtils.Connect()
result = ipServiceUtils.WaitUntilStartupEnd()
if(result <> 0):
    sys.exit(result)
    
ipServiceUtils.testActiveAlarm("CSMngr", "Manager", "CS_STARTUP_FAILED", 1 )


sys.exit(0)
