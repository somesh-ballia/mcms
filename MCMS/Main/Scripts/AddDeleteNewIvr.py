#!/mcms/python/bin/python

# Re-Write date = 10/8/13
# Re-Write name = Uri A.

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-LONG_SCRIPT_TYPE

from McmsConnection import *
from IvrUtils.IvrUtils import *

McmsUtilClass = McmsConnection() # MCMS connection and XML_API file changes class 
McmsUtilClass.Connect()
IvrActionsClass = IvrUtils() # Ivr util class
retries = 50


# 1. get number of IVR services
num_ivr_services = IvrActionsClass.GetNoOfIvrServices(McmsUtilClass)

# 2. add new IVR service
ServiceName = "IVR2"
IvrActionsClass.AddNewIvrService(McmsUtilClass, ServiceName)

# 3. Check new IVR service is added.
IvrActionsClass.CheckNewServiceAdded(McmsUtilClass, num_ivr_services+1, retries)

# 4. Update IVR service.
RetriesNumber_updateParam = 5
IvrActionsClass.UpdateIvrService(McmsUtilClass, RetriesNumber_updateParam, ServiceName)

# 5. Print and compare IVR service params.
IvrActionsClass.PrintAndCompareIvrServiceParam(McmsUtilClass, RetriesNumber_updateParam, ServiceName)

# 6. Delete IVR service by name.
IvrActionsClass.DeleteIvrServiceByName(McmsUtilClass, ServiceName)

# 7. Check IVR service has been deleted
IvrActionsClass.CheckIvrServiceDeleted(McmsUtilClass, num_ivr_services, retries)

McmsUtilClass.Disconnect()

