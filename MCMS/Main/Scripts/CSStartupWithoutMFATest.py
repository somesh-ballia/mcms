#!/mcms/python/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_17
#*export GIDEONSIM=NO
#*export USE_DEFAULT_IP_SERVICE=NO

#-- EXPECTED_ASSERT(2)=HTTP_DIGEST_LIST
#-- EXPECTED_ASSERT(1)= Failed to shadow private data, audit event will be discarded

from IpServiceUtils import *


ipServiceUtils = IpServiceUtils()
ipServiceUtils.Connect()
result = ipServiceUtils.WaitUntilStartupEnd()
if(result <> 0):
    sys.exit(result)


ipServiceUtils.testActiveAlarm("CSMngr", "Manager", "NO_LICENSING", 1 )
ipServiceUtils.testActiveAlarm("CSMngr", "Manager", "NO_SERVICE_FOUND_IN_DB", 1 )
ipServiceUtils.testActiveAlarm("Resource", "Manager", "NO_LICENSING", 1 )


ipServiceUtils.AddService("Scripts/AddIpService.xml", "Status OK")
ipServiceUtils.Wait(2)
ipServiceUtils.testActiveAlarm("CSMngr", "Manager", "NO_SERVICE_FOUND_IN_DB", 0 )


